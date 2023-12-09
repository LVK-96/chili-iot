#pragma once

#include "BlinkyLED.h"
#include "Network.h"
#include "Temperature.h"

void led_task(void* a);
void temperature_task(void* a);
void log_task(void* a);