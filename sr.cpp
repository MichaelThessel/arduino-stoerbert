#include "sr.h"

#include <Arduino.h>
#include <stdint.h>
#include "pins.h"
#include "commands.h"

const uint16_t DEBOUNCE_DELAY = 1000;

sr sr1, sr2 = {0, -1, 0};

srAssignments sra = {
    // Shift register 1
    0b10000000,
    0b01000000,
    0b00100000,
    0b00010000,
    0b00001000,
    0b00000100,
    0b00000010,
    0b00000001,

    // Shift register 2
    0b10000000,
    0b01000000,
    0b00100000,
    0b00010000,
};

// Set up shift register
void setupSr() {
    // Set up shift registers for buttons
    pinMode(PIN_SR_LATCH, OUTPUT);
    pinMode(PIN_SR_CLOCK, OUTPUT);
    pinMode(PIN_SR_DATA, INPUT);
}

// Returns current shift register state as byte
// Bit 7 = Pin 7 / Bit 0= Pin 0
uint8_t srShiftIn(bool doLatch) {
    uint8_t data = 0;

    if (doLatch) {
        digitalWrite(PIN_SR_LATCH, 1);
        delayMicroseconds(20);
        digitalWrite(PIN_SR_LATCH, 0);
    }

    for (int i = 7; i >= 0; i--) {
        digitalWrite(PIN_SR_CLOCK, 0);
        delayMicroseconds(2);
        if (digitalRead(PIN_SR_DATA)) {
            data = data | (1 << i);
        }
        digitalWrite(PIN_SR_CLOCK, 1);

    }

    //DPRINTBINLN(data);

    return data;
}

// Debounce button presses
bool debounce(sr *srd) {
    // First run
    if (srd->previous == -1) {
        srd->previous = srd->state;
        return false;
    }

    if (srd->state != srd->previous) {
        srd->previous = srd->state;

        if ((millis() - srd->debounceTime) > DEBOUNCE_DELAY) {
            return true;
        }

        srd->debounceTime = millis();
    }

    return false;
}
