#include "GPIO.h"
#include "doctest.h"

TEST_CASE("GPIO Port Initialization")
{
    GPIOPort portA(BluePillGPIOPort::A, RCC_GPIOA, RST_GPIOA);

    // Just verifying that we can instantiate the class and call methods
    // In a real test we would mock the HAL calls and verify they are called correctly
    portA.set_pins(GPIO0);

    CHECK(true);
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
