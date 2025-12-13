#include "AtCommandProcessor.h"
#include "doctest.h"
#include "interfaces/IDmaSerial.h"
#include "mocks/MockRTOS.h"
#include <string>
#include <vector>

// Simple Stub for IDmaSerial
class StubDmaSerial : public IDmaSerial {
public:
    mutable std::vector<std::string> log;

    // ISerial methods not used by AtCommandProcessor
    void enable_rx_dma(uint32_t, unsigned int, bool, bool, bool, bool) const override
    {
        log.push_back("enable_rx_dma");
    }
    void enable_tx_dma(uint32_t, unsigned int, bool, bool, bool) const override { log.push_back("enable_tx_dma"); }
    void disable_rx_dma() const override { }
    void disable_tx_dma() const override { log.push_back("disable_tx_dma"); }
    void tx_complete_interrupt(bool) const override { }
    void error_interrupt(bool) const override { }
    unsigned int get_dma_count() const override { return 0; }

    // Flags control
    mutable bool tx_complete = true;
    mutable bool tx_error = false;
    mutable bool rx_error = false;
    mutable bool overrun_error = false;

    bool get_tx_transfer_complete_flag() const override { return tx_complete; }
    void clear_tx_transfer_complete_flag() const override { }
    void clear_sr_tc_bit() const override { }
    bool get_tx_dma_error_flag() const override { return tx_error; }
    bool get_rx_dma_error_flag() const override { return rx_error; }
    bool get_overrun_error_flag() const override { return overrun_error; }

    void clear_rx_dma_error_flag() const override { }
    void clear_tx_dma_error_flag() const override { }
    void clear_rx_dma_complete_flag() const override { }
    void clear_tx_dma_complete_flag() const override { }
};

TEST_CASE("AtCommandProcessor interactions")
{
    StubDmaSerial serial;
    MockRTOS rtos;
    AtCommandProcessor processor(&serial, &rtos);

    SUBCASE("init enables rx dma")
    {
        processor.start_rx_dma();
        CHECK(serial.log.size() == 1);
        CHECK(serial.log[0] == "enable_rx_dma");
    }

    SUBCASE("send_raw enables tx dma and waits for notification")
    {
        std::string cmd = "AT\r\n";
        processor.send_raw(cmd);

        CHECK(serial.log.size() == 2);
        CHECK(serial.log[0] == "enable_tx_dma");
        CHECK(serial.log[1] == "disable_tx_dma"); // disabled after wait

        CHECK(rtos.notify_waits.size() == 1);
        CHECK(rtos.notify_waits[0] == 10); // default timeout
    }
}
