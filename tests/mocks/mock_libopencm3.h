#pragma once

#include <cstdint>
#include <cstddef>

// Exposed for tests to assert that mocked libopencm3 functions are called.
#include <cstddef>
#include <vector>

struct MockI2CCall {
	uint8_t addr;
	size_t wlen;
	size_t rlen;
	std::vector<uint8_t> wdata; // copy of written bytes (if any)
};

extern std::vector<MockI2CCall> mock_i2c_calls;

extern std::vector<uint16_t> mock_usart_send_bytes;
extern int mock_usart_recv_blocking_count;

struct MockDMANumberCall {
	uint8_t channel;
	uint16_t number;
};

struct MockDMAAddressCall {
	uint8_t channel;
	uint32_t address;
};

extern std::vector<MockDMANumberCall> mock_dma_number_calls;
extern std::vector<MockDMAAddressCall> mock_dma_periph_addr_calls;
extern std::vector<MockDMAAddressCall> mock_dma_mem_addr_calls;
extern int mock_dma_enable_channel_count;
extern int mock_dma_disable_channel_count;
extern uint32_t mock_dma_cndtr;

void mock_libopencm3_reset();
