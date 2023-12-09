#include <cstdio>
#include <span>

#include "USART.h"
#include "interrupts.h"

void USART::set_baudrate(unsigned int baudrate) const { usart_set_baudrate(usart, baudrate); }

void USART::set_databits(unsigned int databits) const { usart_set_databits(usart, databits); }

void USART::set_stopbits(USARTStopBits stopbits) const { usart_set_stopbits(usart, static_cast<uint32_t>(stopbits)); }

void USART::set_mode(USARTMode mode) const { usart_set_mode(usart, static_cast<uint32_t>(mode)); }

void USART::set_parity(USARTParity parity) const { usart_set_parity(usart, static_cast<uint32_t>(parity)); }

void USART::set_flow_control(USARTFlowControl flowcontrol) const
{
    usart_set_flow_control(usart, static_cast<uint32_t>(flowcontrol));
}

void USART::enable() const { usart_enable(usart); }

void USART::disable() const { usart_disable(usart); }

void USART::setup(unsigned int baudrate, unsigned int databits, USARTStopBits stopbits, USARTMode mode,
    USARTParity parity, USARTFlowControl flowcontrol)
{
    set_baudrate(baudrate);
    set_databits(databits);
    set_stopbits(stopbits);
    set_mode(mode);
    set_parity(parity);
    set_flow_control(flowcontrol);
    is_setup = true;
}

void USART::send_blocking(char c) const { usart_send_blocking(usart, c); }
void USART::send_blocking(std::string_view str) const
{
    for (const auto& ch : str) {
        send_blocking(ch);
    }
}
uint16_t USART::recieve_blocking() const { return usart_recv_blocking(usart); }
uint16_t USART::recieve() const { return usart_recv(usart); }

void USART::rx_interrupt(bool set) const
{
    if (set) {
        usart_enable_rx_interrupt(usart);
    } else {
        usart_disable_rx_interrupt(usart);
    }
}

void USART::tx_interrupt(bool set) const
{
    if (set) {
        usart_enable_tx_interrupt(usart);
    } else {
        usart_disable_tx_interrupt(usart);
    }
}

void USART::error_interrupt(bool set) const
{
    if (set) {
        usart_enable_error_interrupt(usart);
    } else {
        usart_disable_error_interrupt(usart);
    }
}

bool USART::get_is_setup() const { return is_setup; }

void USARTWithDMA::enable_rx_dma(uint32_t dest_addr, unsigned int number_of_data) const
{
    reset_rx_dma();
    dma_channels.dma->setup_channel(DMADirection::PER2MEM,
        dma_channels.rx_channel.channel, // channel
        (uint32_t)&USART_DR(usart), // source address
        BluePillDMAPeripheralWordSize::BYTE, // source word size
        dest_addr,
        BluePillDMAMemWordSize::BYTE, // destination word size
        BluePillDMAPriority::VERY_HIGH, // priority
        number_of_data, false, true, true, false, true);

    usart_enable_rx_dma(usart);
    dma_channels.dma->enable(dma_channels.rx_channel.channel);
}

void USARTWithDMA::enable_tx_dma(uint32_t source_addr, unsigned int number_of_data) const
{
    reset_tx_dma();
    dma_channels.dma->setup_channel(DMADirection::MEM2PER,
        dma_channels.tx_channel.channel, // channel
        (uint32_t)&USART_DR(usart), // destination address
        BluePillDMAPeripheralWordSize::BYTE, // destination word size
        source_addr,
        BluePillDMAMemWordSize::BYTE, // source word size
        BluePillDMAPriority::VERY_HIGH, // priority
        number_of_data, false, true, true, false, true);

    usart_enable_tx_dma(usart);
    dma_channels.dma->enable(dma_channels.tx_channel.channel);
}

void USARTWithDMA::disable_rx_dma() const
{
    usart_disable_rx_dma(usart);
    dma_channels.dma->disable(dma_channels.rx_channel.channel);
}

void USARTWithDMA::disable_tx_dma() const
{
    usart_disable_tx_dma(usart);
    dma_channels.dma->disable(dma_channels.tx_channel.channel);
}

void USARTWithDMA::reset_rx_dma() const
{
    // Clear the RXNE bit to make sure the dma starts next time
    USART_SR(usart) &= ~USART_SR_RXNE;

    // Clear overrun error bit
    (void)USART_SR(usart);
    (void)USART_DR(usart);

    dma_channels.dma->reset(dma_channels.rx_channel.channel);

    // Clear old interrupt flags
    *(dma_channels.rx_channel.error_flag) = false;
    *(dma_channels.rx_channel.half_flag) = false;
    *(dma_channels.rx_channel.complete_flag) = false;
    usart2_overrun_error = false;
}

void USARTWithDMA::reset_tx_dma() const { dma_channels.dma->reset(dma_channels.tx_channel.channel); }

unsigned int USARTWithDMA::get_dma_count() const
{
    return dma_channels.dma->get_count(dma_channels.rx_channel.channel);
}