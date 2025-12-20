#pragma once

#include <memory>

#include "BlinkyLED.h"
#include "FreeRTOSAdapter.h"
#include "interfaces/II2C.h"
#include "interfaces/ILED.h"
#include "interfaces/INetwork.h"
#include "interfaces/IRTOS.h"
#include "interfaces/ITemperatureSensor.h"

#ifdef QEMU_ENV
#include "MockNetwork.h"
#include "MockTemperatureSensor.h"
#else
#include "BME280TemperatureSensor.h"
#include "ESP8266Network.h"
#endif

#include "System.h"
#include "USART.h"

inline std::unique_ptr<ITemperatureSensor> createTemperatureSensor()
{
// Use higher level mocks when running firmware in QEMU
// I can't be bothered to simulate the full behaviour of the BME280
// -> Just make a mock that returns a fixed value
#ifdef QEMU_ENV
    return std::make_unique<MockTemperatureSensor>();
#else // QEMU_ENV -> !QEMU_ENV
    return std::make_unique<BME280TemperatureSensor>(&bluepill::peripherals::i2c1, BME280I2CBusAddr::SECONDARY);
#endif // QEMU_ENV
}

inline std::unique_ptr<IRTOS> createRTOS() { return std::make_unique<FreeRTOSAdapter>(); }

inline std::unique_ptr<INetwork> createNetwork([[maybe_unused]] const IRTOS* rtos)
{
#ifdef QEMU_ENV
    return std::make_unique<MockNetwork>();
#else
    return std::make_unique<ESP8266Network>(
        &bluepill::peripherals::usart2, &bluepill::peripherals::esp_reset_pin, rtos);
#endif
}

inline std::unique_ptr<ILED> createLED() { return std::make_unique<BlinkyLED>(&bluepill::peripherals::led_pin); }