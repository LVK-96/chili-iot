#pragma once

#include <cstdint>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

#include "Peripheral.h"

enum class BluePillUSART : uint32_t {
    _1 = USART1,
    _2 = USART2,
    _3 = USART3
};

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

enum class USARTParity : uint32_t {
    NONE = USART_PARITY_NONE,
    EVEN = USART_PARITY_EVEN,
    ODD = USART_PARITY_ODD
};

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

    void set_baudrate(unsigned int baudrate);
    void set_databits(unsigned int databits);
    void set_stopbits(USARTStopBits);
    void set_mode(USARTMode);
    void set_parity(USARTParity);
    void set_flow_control(USARTFlowControl);
    void setup(unsigned int baudrate, unsigned int databits, USARTStopBits stopbits, USARTMode mode, USARTParity parity, USARTFlowControl flowcontrol);
    void disable() override;
    void enable() override;
    void clken_reset_disable_setup_enable(unsigned int baudrate, unsigned int databits, USARTStopBits stopbits, USARTMode mode, USARTParity parity, USARTFlowControl flowcontrol);
    void send_blocking(char c) const;
    bool get_is_setup() const;

private:
    bool is_setup = false;
    const uint32_t usart;
};