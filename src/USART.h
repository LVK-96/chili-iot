#pragma once

#include <atomic>
#include <ranges>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

#include "DMA.h"
#include "Peripheral.h"
#include "interrupts.h"

enum class BluePillUSART : uint32_t { _1 = USART1, _2 = USART2, _3 = USART3 };

enum class USARTStopBits : uint32_t {
    _1 = USART_STOPBITS_1,
    _0_5 = USART_STOPBITS_0_5,
    _2 = USART_STOPBITS_2,
    _1_5 = USART_STOPBITS_1_5
};

enum class USARTMode : uint32_t {
    RX = USART_MODE_RX,
    TX = USART_MODE_TX,
    TX_RX = USART_MODE_TX_RX,
};

enum class USARTParity : uint32_t { NONE = USART_PARITY_NONE, EVEN = USART_PARITY_EVEN, ODD = USART_PARITY_ODD };

enum class USARTFlowControl : uint32_t {
    NONE = USART_FLOWCONTROL_NONE,
    RTS = USART_FLOWCONTROL_RTS,
    CTS = USART_FLOWCONTROL_CTS,
    RTS_CTS = USART_FLOWCONTROL_RTS_CTS
};

class USART : public Peripheral {
public:
    constexpr USART(BluePillUSART usart, rcc_periph_clken clken, rcc_periph_rst rst,
        volatile std::atomic_bool* overrun_error, volatile std::atomic_bool* tx_transfer_complete) noexcept
        : Peripheral(clken, rst)
        , usart(static_cast<uint32_t>(usart))
        , overrun_error(overrun_error)
        , tx_transfer_complete(tx_transfer_complete)
    {
    }

    void set_baudrate(unsigned int baudrate) const;
    void set_databits(unsigned int databits) const;
    void set_stopbits(USARTStopBits stopbits) const;
    void set_mode(USARTMode mode) const;
    void set_parity(USARTParity parity) const;
    void set_flow_control(USARTFlowControl flowcontrol) const;
    void setup(unsigned int baudrate, unsigned int databits, USARTStopBits stopbits, USARTMode mode, USARTParity parity,
        USARTFlowControl flowcontrol);
    void disable() const override;
    void enable() const override;
    template <std::ranges::range R> void send_blocking(R&& range) const
    {
        for (const auto& byte : std::forward<R>(range)) {
            usart_send_blocking(usart, byte);
        }
    }
    [[nodiscard]] uint16_t recieve_blocking() const;
    [[nodiscard]] uint16_t recieve() const;
    void rx_interrupt(bool set) const;
    void tx_interrupt(bool set) const;
    void tx_complete_interrupt(bool set) const;
    void error_interrupt(bool set) const;
    void idle_line_received_interrupt(bool set) const;
    [[nodiscard]] bool get_is_setup() const;
    [[nodiscard]] bool get_overrun_error_flag() const { return *overrun_error; }
    [[nodiscard]] bool get_tx_transfer_complete_flag() const { return *tx_transfer_complete; }
    void clear_overrun_error_flag() const { *overrun_error = false; }
    void clear_tx_transfer_complete_flag() const { *tx_transfer_complete = false; }

protected:
    uint32_t usart;

private:
    bool is_setup = false;
    volatile std::atomic_bool* overrun_error;
    volatile std::atomic_bool* tx_transfer_complete;
};

struct DMAChannelAndFlags {
    BluePillDMAChannel channel;
    volatile std::atomic_bool* error_flag;
    volatile std::atomic_bool* half_flag;
    volatile std::atomic_bool* complete_flag;
};

struct USARTDMA {
    const DMA* dma;
    DMAChannelAndFlags rx_channel;
    DMAChannelAndFlags tx_channel;
};

class USARTWithDMA final : public USART {
public:
    constexpr USARTWithDMA(BluePillUSART usart, rcc_periph_clken clken, rcc_periph_rst rst,
        volatile std::atomic_bool* overrun_error_flag, volatile std::atomic_bool* tx_transfer_complete_flag,
        const USARTDMA& dma_channels) noexcept
        : USART(usart, clken, rst, overrun_error_flag, tx_transfer_complete_flag)
        , dma_channels(dma_channels)
    {
    }

    void enable_rx_dma(uint32_t dest_addr, unsigned int number_of_data, bool error_interrupt, bool half_interrupt,
        bool complete_interrupt, bool circular) const;
    void enable_tx_dma(uint32_t source_addr, unsigned int number_of_data, bool error_interrupt, bool half_interrupt,
        bool complete_interrupt) const;
    void disable_rx_dma() const;
    void disable_tx_dma() const;
    void reset_rx_dma() const;
    void reset_tx_dma() const;
    [[nodiscard]] unsigned int get_dma_count() const;
    [[nodiscard]] bool get_rx_dma_complete_flag() const { return *(dma_channels.rx_channel.complete_flag); }
    [[nodiscard]] bool get_tx_dma_complete_flag() const { return *(dma_channels.tx_channel.complete_flag); }
    [[nodiscard]] bool get_rx_dma_error_flag() const { return *(dma_channels.rx_channel.error_flag); }
    [[nodiscard]] bool get_tx_dma_error_flag() const { return *(dma_channels.tx_channel.error_flag); }
    void clear_rx_dma_complete_flag() const { *(dma_channels.rx_channel.complete_flag) = false; }
    void clear_tx_dma_complete_flag() const { *(dma_channels.tx_channel.complete_flag) = false; }
    void clear_rx_dma_error_flag() const { *(dma_channels.rx_channel.error_flag) = false; }
    void clear_tx_dma_error_flag() const { *(dma_channels.tx_channel.error_flag) = false; }
    void clear_sr_tc_bit() const { USART_SR(usart) &= ~USART_SR_TC; }

private:
    USARTDMA dma_channels;
};