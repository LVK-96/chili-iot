#include "BlinkyLED.h"

void GPIOLED::on() const { pin->port->set_pins(pin->pin_nro); }

void GPIOLED::off() const { pin->port->clear_pins(pin->pin_nro); }

void GPIOLED::toggle() const { pin->port->toggle_pins(pin->pin_nro); }