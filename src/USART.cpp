#include "USART.h"

USART::USART(BluePillUSART usart, rcc_periph_clken clken, rcc_periph_rst rst)
    : Peripheral(clken, rst)
    , usart(static_cast<uint32_t>(usart))
{
}

void USART::set_baudrate(unsigned int baudrate)
{
    usart_set_baudrate(usart, baudrate);
}

void USART::set_databits(unsigned int databits)
{
    usart_set_databits(usart, databits);
}

void USART::set_stopbits(USARTStopBits stopbits)
{
    usart_set_stopbits(usart, static_cast<uint32_t>(stopbits));
}

void USART::set_mode(USARTMode mode)
{
    usart_set_mode(usart, static_cast<uint32_t>(mode));
}

void USART::set_parity(USARTParity parity)
{
    usart_set_parity(usart, static_cast<uint32_t>(parity));
}

void USART::set_flow_control(USARTFlowControl flowcontrol)
{
    usart_set_flow_control(usart, static_cast<uint32_t>(flowcontrol));
}

void USART::enable()
{
    usart_enable(usart);
}

void USART::disable()
{
    usart_disable(usart);
}

void USART::setup(unsigned int baudrate, unsigned int databits, USARTStopBits stopbits, USARTMode mode, USARTParity parity, USARTFlowControl flowcontrol)
{
    set_baudrate(baudrate);
    set_databits(databits);
    set_stopbits(stopbits);
    set_mode(mode);
    set_parity(parity);
    set_flow_control(flowcontrol);
    is_setup = true;
}

void USART::send_blocking(char c) const
{
    usart_send_blocking(usart, c);
}

bool USART::get_is_setup() const
{
    return is_setup;
}