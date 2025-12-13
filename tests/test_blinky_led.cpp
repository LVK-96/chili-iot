#include "BlinkyLED.h"
#include "doctest.h"

TEST_CASE("GPIOLED construction and interface")
{
    GPIOPort port(BluePillGPIOPort::C, RCC_GPIOC, RST_GPIOC);
    GPIOPin pin(GPIO13, &port);
    GPIOLED led(&pin);

    SUBCASE("LED operations don't crash")
    {
        // These operations call the GPIO functions
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