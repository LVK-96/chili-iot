#pragma once

#include <cstdint>

class IDmaSerial {
public:
    virtual ~IDmaSerial() = default;

    virtual void enable_rx_dma(uint32_t dest_addr, unsigned int number_of_data, bool error_interrupt,
        bool half_interrupt, bool complete_interrupt, bool circular) const
        = 0;
    virtual void enable_tx_dma(uint32_t source_addr, unsigned int number_of_data, bool error_interrupt,
        bool half_interrupt, bool complete_interrupt) const
        = 0;

    virtual void disable_rx_dma() const = 0;
    virtual void disable_tx_dma() const = 0;

    virtual void tx_complete_interrupt(bool set) const = 0;
    virtual void error_interrupt(bool set) const = 0;

    [[nodiscard]] virtual bool get_tx_dma_error_flag() const = 0;
    [[nodiscard]] virtual bool get_rx_dma_error_flag() const = 0;
    [[nodiscard]] virtual bool get_tx_transfer_complete_flag() const = 0;
    [[nodiscard]] virtual bool get_overrun_error_flag() const = 0;

    virtual void clear_tx_transfer_complete_flag() const = 0;
    virtual void clear_sr_tc_bit() const = 0;
    virtual void clear_rx_dma_error_flag() const = 0;
    virtual void clear_tx_dma_error_flag() const = 0;
    virtual void clear_rx_dma_complete_flag() const = 0;
    virtual void clear_tx_dma_complete_flag() const = 0;

    [[nodiscard]] virtual unsigned int get_dma_count() const = 0;
};
