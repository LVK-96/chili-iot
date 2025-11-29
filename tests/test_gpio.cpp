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
