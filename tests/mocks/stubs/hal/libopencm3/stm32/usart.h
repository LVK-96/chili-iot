#pragma once

#include <cstdint>

#define USART1 0x40013800
#define USART2 0x40004400
#define USART3 0x40004800

#define USART_CR1_UE (1 << 13)
#define USART_CR1_TE (1 << 3)
#define USART_CR1_RE (1 << 2)
#define USART_CR1_RXNEIE (1 << 5)
#define USART_CR1_TXEIE (1 << 7)
#define USART_CR1_IDLEIE (1 << 4)

#define USART_SR_TXE (1 << 7)
#define USART_SR_RXNE (1 << 5)
#define USART_SR_TC (1 << 6)
#define USART_SR_IDLE (1 << 4)

#define USART_MODE_TX 0x1
#define USART_MODE_RX 0x2
#define USART_MODE_TX_RX 0x3

#define USART_FLOWCONTROL_NONE 0x0

#define USART_STOPBITS_1 0x0
#define USART_STOPBITS_0_5 0x1
#define USART_STOPBITS_2 0x2
#define USART_STOPBITS_1_5 0x3

#define USART_PARITY_NONE 0x0
#define USART_PARITY_EVEN 0x1
#define USART_PARITY_ODD 0x2

#define USART_FLOWCONTROL_NONE 0x0
#define USART_FLOWCONTROL_RTS 0x1
#define USART_FLOWCONTROL_CTS 0x2
#define USART_FLOWCONTROL_RTS_CTS 0x3

#ifdef __cplusplus
extern "C" {
#endif
void usart_set_baudrate(uint32_t usart, uint32_t baud);
void usart_set_databits(uint32_t usart, uint32_t bits);
void usart_set_stopbits(uint32_t usart, uint32_t stopbits);
void usart_set_mode(uint32_t usart, uint32_t mode);
void usart_set_parity(uint32_t usart, uint32_t parity);
void usart_set_flow_control(uint32_t usart, uint32_t flowcontrol);
void usart_enable(uint32_t usart);
void usart_disable(uint32_t usart);
void usart_send_blocking(uint32_t usart, uint16_t data);
uint16_t usart_recv_blocking(uint32_t usart);
void usart_enable_rx_interrupt(uint32_t usart);
void usart_disable_rx_interrupt(uint32_t usart);
void usart_enable_tx_interrupt(uint32_t usart);
void usart_disable_tx_interrupt(uint32_t usart);
void usart_enable_tx_complete_interrupt(uint32_t usart);
void usart_disable_tx_complete_interrupt(uint32_t usart);
void usart_enable_error_interrupt(uint32_t usart);
void usart_disable_error_interrupt(uint32_t usart);
void usart_disable_idle_interrupt(uint32_t usart);
void usart_enable_idle_interrupt(uint32_t usart);
bool usart_get_flag(uint32_t usart, uint32_t flag);
void usart_send(uint32_t usart, uint16_t data);
uint16_t usart_recv(uint32_t usart);
void usart_enable_rx_dma(uint32_t usart);
void usart_disable_rx_dma(uint32_t usart);
void usart_enable_tx_dma(uint32_t usart);
void usart_disable_tx_dma(uint32_t usart);
#ifdef __cplusplus
}
#endif

extern volatile uint32_t mock_usart_dr;
extern volatile uint32_t mock_usart_sr;
extern volatile uint32_t mock_usart_cr3;
#define USART_DR(usart) (mock_usart_dr)
#define USART_SR(usart) (mock_usart_sr)
#define USART_CR3(usart) (mock_usart_cr3)

#define USART_CR3_EIE (1 << 0)
#define USART_SR_ORE (1 << 3)
#define USART_SR_RXNE (1 << 5)

#define USART2_DR USART_DR(USART2)
