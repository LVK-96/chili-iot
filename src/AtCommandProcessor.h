#include <algorithm>
#include <array>
#include <cstdio>
#include <functional>
#include <ranges>
#include <span>
#include <string_view>

#include "ATCommand.h"
#include "Logger.h"
#include "System.h"
#include "interfaces/IDmaSerial.h"
#include "interfaces/IRTOS.h"
#include "interrupts.h"
#include "utils.h"

class AtCommandProcessor {
public:
    explicit constexpr AtCommandProcessor(const IDmaSerial* usart, const IRTOS* rtos)
        : usart(usart)
        , rtos(rtos)
    {
    }

    void start_rx_dma()
    {
        utils::logger.info("Enabling RX DMA\n");
        old_rx_dma_count = rx_buf.size();
        // TODO: This cast assumes TaskHandle_t is compatible with IRTOS::TaskHandle (void*)
        // This dependency on interrupts.h preventing full decoupling is known (Phase 2 refactor)
        set_network_task_handle_for_rx_dma_interrupts(static_cast<TaskHandle_t>(rtos->get_current_task_handle()));
        usart->error_interrupt(true);
        usart->enable_rx_dma(reinterpret_cast<uintptr_t>(rx_buf.data()), rx_buf.size(), true, false, false, true);
    }

    utils::ErrorCode send_raw(std::span<const std::byte> cmd, unsigned int timeout_ms = 10) const
    {
        if (std::ranges::size(cmd) == 0) {
            return utils::ErrorCode::OK;
        }

        // Enable TX DMA
        // TODO: not thread safe, lock USART/TX DMA mutex
        usart->enable_tx_dma(
            reinterpret_cast<uintptr_t>(cmd.data()), cmd.size() * sizeof(std::byte), true, false, true);

        // Yield task until 1. USART transfer is complete or 2. timeout is reached
        // The USART interrupt will xTaskNotify() the task when the transfer is complete
        bool const timeout = !rtos->task_notify_wait(timeout_ms);

        // TODO: Handle errors
        // DMA transfer error
        if (usart->get_tx_dma_error_flag()) {
            utils::logger.error("USART TX DMA transfer error!\n");
        }
        // Timeout, TX timeout is an error!
        // It is possible that we timeout just before the transfer is complete
        // -> if we just check the timeout flag we get a false positive error
        // -> check if the transfer is complete flag is set to be sure
        if (!usart->get_tx_transfer_complete_flag()) {
            // The other way around should not happen:
            // We don't timeout, but neither is the transfer complete...
            if (!timeout) {
                utils::logger.error("Timeout flag NOT set, but transfer is NOT complete. Is there a bug in the "
                                    "interrupt handler?\n");
            } else {
                utils::logger.error("USART TX transfer timeout!\n");
            }
        }

        // Cleanup, disable the TX DMA
        usart->disable_tx_dma();
        usart->clear_tx_transfer_complete_flag();
        // TODO: not thread safe, release the USART/DMA mutex

        return utils::ErrorCode::OK;
    }

    [[nodiscard]] utils::ErrorCode send_command(
        esp8266::ATCommand cmd, std::string_view ok_response, unsigned int response_time_ms = 1000) const
    {
        using namespace std::literals; // std::string_view literals

        // Don't stack allocate, the stack is too small to fit this
        // Use a static buffer instead
        static std::array<char, 2048> wrap_case_concat_buffer;

        // Send the command
        send_raw(cmd);

        // Now we wait for a response
        // Yield task until timeout is reached, or we have an error
        (void)rtos->task_notify_wait(response_time_ms);

        auto res = utils::ErrorCode::OK;
        // DMA transfer error
        if (usart->get_rx_dma_error_flag()) {
            // "Handle error" :D
            utils::logger.error("USART RX DMA transfer error!\n");
            res = utils::ErrorCode::NETWORK_RESPONSE_DMA_ERROR;
        }
        // UART overrun error
        if (usart->get_overrun_error_flag()) {
            utils::logger.error("USART RX DMA transfer overrun error!\n");
            res = utils::ErrorCode::NETWORK_RESPONSE_OVERRUN_ERROR;
        }
        // Everything OK!
        // // RX timeout is NOT an error condition

        // Check the response
        auto count = usart->get_dma_count();
        // DMCNT counts down
        bool wrapped = count > old_rx_dma_count;
        // First new byte received, based on the previous last byte received
        const size_t start_idx = rx_buf.size() - old_rx_dma_count;
        // One past last new byte received or one past the last byte in the buffer if we wrapped
        const size_t segment_end = rx_buf.size() - (wrapped ? 0 : count);

        // These are only used in the wrapped case
        const size_t wrapped_start_idx = 0;
        // The last byte received in the wrapped case
        const size_t wrapped_end_idx = wrapped ? rx_buf.size() - count : 0;

        // How many bytes were received before the wrap
        const unsigned int received_before_wrap = segment_end - start_idx;
        // How many bytes were received after the wrap
        const unsigned int received_after_wrap = wrapped_end_idx - wrapped_start_idx;
        // How many bytes in total
        const unsigned received = received_before_wrap + received_after_wrap; // Check how many bytes were received

        std::string_view sv;
        // TODO: Some fancy chaining of DMAs so that we don't need the CPU to copy the data?
        if (wrapped) {
            // Copy the bytes received before the wrap
            std::copy(rx_buf.begin() + start_idx, rx_buf.end(), wrap_case_concat_buffer.begin());
            // Copy the bytes received after the wrap
            std::copy(rx_buf.begin(), rx_buf.begin() + wrapped_end_idx,
                wrap_case_concat_buffer.begin() + received_before_wrap);
            sv = std::string_view(const_cast<char*>(wrap_case_concat_buffer.data()), received);
        } else {
            // No wrap, just directly access the bytes received
            sv = std::string_view(const_cast<char*>(rx_buf.data() + start_idx), received);
        }

        // Update the old rx dma count, so we know where we left off when the next DMA transfer starts
        old_rx_dma_count = count;

        // Split respone by line by line, check if any of the lines contains the OK response
        if (!(std::ranges::any_of(sv | std::views::lazy_split("\r\n"sv),
                [ok_response](auto const& line) { return !std::ranges::search(line, ok_response).empty(); }))) {
            res = utils::ErrorCode::NETWORK_RESPONSE_NOT_OK_ERROR;
        }

        if constexpr (debug) {
            bool error = res != utils::ErrorCode::OK;
            if (error) {
                utils::logger.error("Printing the received data (if any)...\n");
            } else {
                utils::logger.info("Printing the received data (if any)...\n");
            }
            if (received > 0) {
                utils::logger.log(sv);
            }
            utils::logger.log("\n");
        }

        return res;
    }

private:
    const IDmaSerial* usart;
    const IRTOS* rtos;
    static constexpr bool debug = true;
    mutable size_t old_rx_dma_count = 0;
    mutable std::array<volatile char, 2048> rx_buf {};
};
