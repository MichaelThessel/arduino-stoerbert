#include <Adafruit_VS1053.h>

#include "pins.h"
#include "debug.h"
#include "commands.h"
#include "sr.h"
#include "player.h"
#include "power.h"

extern sr sr1;
extern sr sr2;
extern srAssignments sra;

// ##################################
// Setup
// ##################################
void setup() {
    #ifdef DEBUG
    Serial.begin(9600);
    #endif

    setupPower();
    setupPlayer();
    setupSr();
}

// ##################################
// Main loop
// ##################################

void loop() {
    watchdogReset();

    #ifdef DEBUG
    if (Serial.available()) {
        char c = Serial.read();
        handleCommand(c);
    }
    #endif

    handlePlayer();
    handleButtons();
}

// ##################################
// Button handling
// ##################################
void handleButtons() {
    // Handle button shift registers
    sr1.state = srShiftIn(true);
    sr2.state = srShiftIn(false);

    if (debounce(&sr1)) {
        if (sr1.state & sra.button2) { handleCommand(COMMAND2); }
        if (sr1.state & sra.button3) { handleCommand(COMMAND3); }
        if (sr1.state & sra.button5) { handleCommand(COMMAND5); }
        if (sr1.state & sra.button6) { handleCommand(COMMAND6); }
        if (sr1.state & sra.button9) { handleCommand(COMMAND9); }
        if (sr1.state & sra.buttonNext) { handleCommand(COMMAND_NEXT); }
    }

    if (debounce(&sr2)) {
        if (sr2.state & sra.button1) { handleCommand(COMMAND1); }
        if (sr2.state & sra.button4) { handleCommand(COMMAND4); }
        if (sr2.state & sra.button7) { handleCommand(COMMAND7); }
        if (sr2.state & sra.button8) { handleCommand(COMMAND8); }
        if (sr2.state & sra.buttonPlayPause) { handleCommand(COMMAND_PLAY_PAUSE); }
        if (sr2.state & sra.buttonPrevious) { handleCommand(COMMAND_PREVIOUS); }
    }
}

// ##################################
// Commands
// ##################################
void handleCommand(char c) {
    switch (c) {
        // Start playing
        case COMMAND1:
        case COMMAND2:
        case COMMAND3:
        case COMMAND4:
        case COMMAND5:
        case COMMAND6:
        case COMMAND7:
        case COMMAND8:
        case COMMAND9:
            DPRINTF("Received Command: PLAY ");
            DPRINTLN(c);
            if (setAlbum(c)) {
                playAlbum();
            }
            break;

        // GOD mode !!!
        case COMMAND_GOD_MODE:
            DPRINTLNF("Received Command: God Mode");
            toggleGodMode();
            break;

        // Toggle play/pause
        case COMMAND_PLAY_PAUSE:
            detectGodMode(c);

            DPRINTLNF("Received Command: TOGGLE PLAY/PAUSE");
            togglePlayPause();
            break;

        // Next track
        case COMMAND_NEXT:
            if (detectGodMode(c)) {
                break;
            }

            DPRINTLNF("Received Command: NEXT TRACK");
            playNextTrack();
            break;

        // Previous track / COMMAND0 in god mode
        case COMMAND_PREVIOUS:
            detectGodMode(c);

            if (isGodMode()) {
                setAlbum(COMMAND0);
                break;
            }

            DPRINTLNF("Received Command: PREVIOUS TRACK");
            playPreviousTrack();
            break;

        // Increase volume
        case COMMAND_INCREASE_VOLUME:
            DPRINTLNF("Received Command: VOLUME +");
            increaseVolume();
            break;

        // Decrease volume
        case COMMAND_DECREASE_VOLUME:
            DPRINTLNF("Received Command: VOLUME -");
            decreaseVolume();
            break;

        // Ignore newlines
        case '\n':
            break;

        // Log invalid commands
        default:
            DPRINTF("Invalid command: ");
            DPRINTLN(c);
            break;
    }
}
