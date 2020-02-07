#include "power.h"

#include <Arduino.h>
#include <avr/power.h>
#include <avr/wdt.h>

#include "debug.h"

void setupPower() {
    wdt_enable(WDTO_2S);

    power_twi_disable();
    #ifndef DEBUG
    power_usart0_disable();
    #endif
}

void reset() {
    wdt_enable(WDTO_15MS);
    while(1);
}

void watchdogReset() {
    wdt_reset();
}
