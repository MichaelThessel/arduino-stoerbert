#ifndef SR_H
#define SR_H

#include <stdint.h>

const uint16_t DEBOUNCE_DELAY = 500;

// Shift register states
struct sr {
    uint8_t state; // Shift register current state
    int8_t previous; // Shift register previous state
    uint32_t debounceTime; // Shift register time since last debounce
};

// Shift register button - pin mappings
struct srAssignments {
    // Shift register 1
    const uint8_t button1;
    const uint8_t button2;
    const uint8_t button3;
    const uint8_t button4;
    const uint8_t button5;
    const uint8_t button6;
    const uint8_t button7;
    const uint8_t button8;

    // Shift register 2
    const uint8_t button9;
    const uint8_t buttonPlayPause;
    const uint8_t buttonPrevious;
    const uint8_t buttonNext;
};

void setupSr();
uint8_t srShiftIn(bool doLatch);
bool debounce(sr *srd);

#endif
