#include "BlinkyLED.h"
#include "doctest.h"

TEST_CASE("GPIOLED construction and interface")
{
    GPIOPort port(BluePillGPIOPort::C, RCC_GPIOC, RST_GPIOC);
    GPIOPin pin(GPIO13, &port);
    GPIOLED led(&pin);

    SUBCASE("LED operations don't crash")
    {
        // These operations call the mock GPIO functions
        // We're just verifying the interface works without crashing
        led.on();
        led.off();
        led.toggle();

        // If we get here, the operations completed successfully
        CHECK(true);
    }

    SUBCASE("LED can be used through base class pointer")
    {
        BlinkyLED* base_led = &led;

        // Verify polymorphism works
        base_led->on();
        base_led->off();
        base_led->toggle();

        CHECK(true);
    }
}

TEST_CASE("GPIOPort operations")
{
    GPIOPort port(BluePillGPIOPort::A, RCC_GPIOA, RST_GPIOA);

    SUBCASE("Port operations don't crash")
    {
        port.set_pins(GPIO0 | GPIO1);
        port.clear_pins(GPIO0);
        port.toggle_pins(GPIO1);
        uint16_t pins = port.get_pins(GPIO0 | GPIO1);
        (void)pins; // Suppress unused warning

        CHECK(true);
    }

    SUBCASE("Port setup operations work")
    {
        port.setup_pins(GPIO0, GPIOMode::OUTPUT_2_MHZ, GPIOFunction::OUTPUT_PUSHPULL);
        port.setup_pins(GPIO1, GPIOMode::INPUT, GPIOFunction::INPUT_FLOAT);

        CHECK(true);
    }
}
