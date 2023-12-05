#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

#include "DMA.h"
#include "Peripheral.h"

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
    constexpr USART(BluePillUSART usart, rcc_periph_clken clken, rcc_periph_rst rst) noexcept
        : Peripheral(clken, rst)
        , usart(static_cast<uint32_t>(usart))
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
    void send_blocking(char c) const;
    void send_blocking(std::string_view str) const;
    [[nodiscard]] uint16_t recieve_blocking() const;
    [[nodiscard]] uint16_t recieve() const;
    void rx_interrupt(bool set) const;
    void tx_interrupt(bool set) const;
    void error_interrupt(bool set) const;
    [[nodiscard]] bool get_is_setup() const;

protected:
    const uint32_t usart;

private:
    bool is_setup = false;
};

struct USARTDMA {
    const DMA& dma;
    const BluePillDMAChannel rx_channel;
    const BluePillDMAChannel tx_channel;
};

class USARTWithDMA : public USART {
public:
    constexpr USARTWithDMA(
        BluePillUSART usart, rcc_periph_clken clken, rcc_periph_rst rst, const USARTDMA& dma) noexcept
        : USART(usart, clken, rst)
        , dma(dma)
    {
    }

    void enable_rx_dma(uint32_t dest_addr, unsigned int number_of_data) const;
    void enable_tx_dma() const;
    void disable_rx_dma() const;
    void disable_tx_dma() const;
    void reset_rx_dma() const;
    void reset_tx_dma() const;
    [[nodiscard]] unsigned int get_dma_count() const;

private:
    const USARTDMA dma;
};