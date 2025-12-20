#include "BlinkyLED.h"

void BlinkyLED::on() const { pin->port->set_pins(pin->pin_nro); }

void BlinkyLED::off() const { pin->port->clear_pins(pin->pin_nro); }

void BlinkyLED::toggle() const { pin->port->toggle_pins(pin->pin_nro); }