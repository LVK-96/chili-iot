#pragma once

#pragma once

#include <cstddef>
#include <cstdint>

#define I2C1 0x40005400
#define I2C2 0x40005800

#define I2C_CR1_PE (1 << 0)
#define I2C_CR1_ACK (1 << 10)
#define I2C_CR1_STOP (1 << 9)
#define I2C_CR1_START (1 << 8)

#define I2C_SR1_SB (1 << 0)
#define I2C_SR1_ADDR (1 << 1)
#define I2C_SR1_BTF (1 << 2)
#define I2C_SR1_RxNE (1 << 6)
#define I2C_SR1_TxE (1 << 7)

#define I2C_SR2_MSL (1 << 0)
#define I2C_SR2_BUSY (1 << 1)

#ifdef __cplusplus
extern "C" {
#endif

enum i2c_speeds { i2c_speed_sm_100k, i2c_speed_fm_400k, i2c_speed_fmp_1m };

void i2c_peripheral_disable(uint32_t i2c);
void i2c_peripheral_enable(uint32_t i2c);
void i2c_set_speed(uint32_t i2c, enum i2c_speeds speed, uint32_t clock);
void i2c_set_own_7bit_slave_address(uint32_t i2c, uint8_t slave);
void i2c_reset(uint32_t i2c);
void i2c_send_start(uint32_t i2c);
void i2c_send_stop(uint32_t i2c);
void i2c_send_data(uint32_t i2c, uint8_t data);
uint8_t i2c_get_data(uint32_t i2c);
void i2c_enable_ack(uint32_t i2c);
void i2c_disable_ack(uint32_t i2c);
#ifdef __cplusplus
}
#endif

extern volatile uint32_t mock_i2c_cr1;
#define I2C_CR1(x) (mock_i2c_cr1)

#ifdef __cplusplus
extern "C" {
#endif
void i2c_transfer7(uint32_t i2c, uint8_t addr, const uint8_t* w, size_t wlen, uint8_t* r, size_t rlen);
#ifdef __cplusplus
}
#endif
