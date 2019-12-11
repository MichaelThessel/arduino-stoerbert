#include "power.h"

#include <Arduino.h>
#include <avr/power.h>

#include "debug.h"

void setupPower() {
    power_twi_disable();
    #ifndef DEBUG
    power_usart0_disable();
    #endif
}
