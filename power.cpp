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

void setLowPower() {
    DPRINTLNF("Entering low power state");

    delay(100);

    power_timer0_disable();
    power_timer1_disable();
    power_timer2_disable();

    power_adc_disable();

    power_usart0_disable();
}

void resetPower() {
    power_all_enable();
    setupPower();

    delay(100);

    DPRINTLNF("Restoring regular power state");
}
