#include "player.h"

#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <Arduino.h>
#include <SD.h>
#include <EEPROM.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "pins.h"
#include "debug.h"

player p = {{}, "k01", 0, 0, false, 0, false, 50, false, 0};

Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(
    PIN_VS1053_SHIELD_RESET,
    PIN_VS1053_SHIELD_CS,
    PIN_VS1053_SHIELD_DCS,
    PIN_VS1053_DREQ,
    PIN_VS1053_CARDCS
);

// ##################################
// File handling
// ##################################

// Detect sequence in album based on filename
int detectFileSequence(char *fileName) {
    char s[3];
    s[0] = fileName[0];
    s[1] = fileName[1];
    return atoi(s) - 1;
}

// Load album files
// Directory and file structure needs to be:
// /[1-9]/[01-99].mp3
void loadAlbum() {
    char folder[6];
    sprintf(folder, "/%s/", p.currentAlbum);

    DPRINTF("Loading files from directory: ");
    DPRINTLN(folder);

    File dir = SD.open(folder);

    // Process diretories in /
    uint8_t i = 0;
    while (true) {
        File entry = dir.openNextFile();

        if (!entry) {
            break;
        }

        // Ignore everything but files
        if (entry.isDirectory()) {
            continue;
        }

        DPRINTF("Found file: ");
        DPRINTLN(entry.name());

        if (!musicPlayer.isMP3File(entry.name())) {
            DPRINTLNF("Not a MP3 skipping");
            continue;
        }

        int sequence = detectFileSequence(entry.name());
        DPRINTF("Adding file to position: ");
        DPRINTLN(sequence);

        free(p.album[sequence]);
        p.album[sequence] = malloc(strlen(entry.name()) + 1);
        strcpy(p.album[sequence], entry.name());
        i++;

        entry.close();
    }

    p.currentAlbumTrackCount = i;
}

// ##################################
// VS1053 controls
// ##################################

// Set volume
void setVolume() {
    if (p.volume > VOLUME_MIN) {
        p.volume = VOLUME_MIN;
    }

    //DPRINTF("Setting volume to: ");
    //DPRINTLN(p.volume);
    musicPlayer.setVolume(p.volume, p.volume);
}

// Increase volume
void increaseVolume() {
    if (p.volume - VOLUME_STEP < 0) {
        p.volume = 0;
    } else {
        p.volume -= VOLUME_STEP;
    }
    setVolume();
}

// Decrease volume
void decreaseVolume() {
    if (p.volume + VOLUME_STEP > VOLUME_MIN) {
        p.volume = VOLUME_MIN;
    } else {
        p.volume += VOLUME_STEP;
    }
    setVolume();
}


// ##################################
// EEPROM state saving
// ##################################

// Save current player state to EEPROM
// We need to save 4 bytes to EEPROM
// Bytes 1-3: Current album name (i.e "k01")
// Byte 4: Current track
void saveState() {
    uint8_t i = 0;

    // Save current album
    DPRINTF("Saving album to EEPROM: ");
    for (; i < FOLDER_NAME_LENGTH - 1; i++) {
        EEPROM.write(i, p.currentAlbum[i]);
        DPRINT(i);
        DPRINTF("->");
        DPRINT(p.currentAlbum[i]);
        DPRINTF(" ");
    }
    DPRINTLNF("");

    // Save the current track
    EEPROM.write(i, p.currentTrack);
    DPRINTF("Saving track to EEPROM: ");
    DPRINT(i);
    DPRINTF("->");
    DPRINTLN(p.currentTrack);
}

// Load current player state from EEPROM
bool loadState() {
    bool valid = false;
    uint8_t i = 0;

    // Load current album
    char currentAlbum[FOLDER_NAME_LENGTH];
    for (; i < FOLDER_NAME_LENGTH - 1; i++) {
        currentAlbum[i] = EEPROM.read(i);
        if (currentAlbum[i] != 0) {
            valid = true;
        }
    }
    currentAlbum[i] = '\0';

    if (!valid) {
        return valid;
    }

    strcpy(p.currentAlbum, currentAlbum);

    DPRINTF("Loaded album from EEPROM: ");
    DPRINTLN(p.currentAlbum);

    // Load the current track
    p.currentTrack = EEPROM.read(i);

    DPRINTF("Loaded track from EEPROM: ");
    DPRINTLN(p.currentTrack);

    return valid;
}

// Clear player state from EEPROM
void clearState() {
    for (int i = 0; i < FOLDER_NAME_LENGTH + 1; i++) {
        EEPROM.write(i, 0);
    }
}

// ##################################
// Power reminder
// ##################################

// Set when to play the next power reminder
void setPowerReminder() {
    p.powerReminderDelay = millis() + POWER_REMINDER_PERIOD;
}

// Play the power reminder
void powerReminder() {
    if (p.isPlaying || p.powerReminderDelay == 0) {
        setPowerReminder();
        return;
    }

    if (p.powerReminderDelay > millis()) {
        return;
    }

    DPRINTLNF("Playing power reminder");

    for (uint8_t i = 0; i < 3; i++) {
        musicPlayer.sineTest(1000, 500);
        musicPlayer.stopPlaying();
        delay(500);
    }

    setPowerReminder();
}

// ##################################
// Player controls
// ##################################

// Reset album playback
void resetPlayback() {
    p.isPlaying = false;
    musicPlayer.stopPlaying();
    p.currentTrack = 0;
    clearState();
}

// Play individual file
void playFile() {
    char path[99];
    sprintf(path, "/%s/%s", p.currentAlbum, p.album[p.currentTrack]);

    musicPlayer.stopPlaying();

    if (!SD.exists(path)) {
        DPRINTF("File missing: ");
        DPRINTLN(path);
        resetPlayback();
        return;
    }

    saveState();

    DPRINTF("Playing file ");
    DPRINTLN(path);

    p.isPlaying = true;
    musicPlayer.startPlayingFile(path);
}

// Toggle Play/Pause
void togglePlayPause() {
    if (musicPlayer.paused()) {
        musicPlayer.pausePlaying(false);
    } else {
        musicPlayer.pausePlaying(true);
    }
}

// Skip forward
void playNextTrack() {
    if (p.currentTrack == p.currentAlbumTrackCount - 1) {
        return;
    }

    p.currentTrack++;
    playFile();
}

// Skip backward
void playPreviousTrack() {
    if (p.currentTrack == 0) {
        return;
    }

    p.currentTrack--;
    playFile();
}

// Sets the current album
void setAlbum(char c) {
    p.currentAlbum[2] = c;
}

// Resumes playback after power cycle
void resumePlayback() {
    if (p.hasResumed) {
        return;
    }
    p.hasResumed = true;

    if (loadState()) {
        DPRINTLNF("Resuming playback");
        if (p.currentAlbum[0] == 'g') {
            p.isGodMode = true;
        }
        loadAlbum();
        playFile();
    }
}

// Play album
void playAlbum() {
    loadAlbum();

    if (p.currentAlbumTrackCount == 0) {
        DPRINTLNF("No tracks found");
        return;
    }

    resetPlayback();

    DPRINTF("Playing album ");
    DPRINTLN(p.currentAlbum);

    playFile();
}

// Advance to the next track
void advanceTrack()
{
    // Don't do anything if we are not playing
    if (!p.isPlaying) {
        return;
    }

    // Exit if we are currently playing or paused
    if (musicPlayer.playingMusic || musicPlayer.paused()) {
        return;
    }

    // Exit if we reached the last track
    if (p.currentTrack == p.currentAlbumTrackCount - 1) {
        DPRINTLNF("End of album reached");
        resetPlayback();
        return;
    }

    playNextTrack();
}

// Set up player
void setupPlayer() {
    if (!musicPlayer.begin()) {
        DPRINTLNF("Couldn't find VS1053");
        while (1);
    }
    DPRINTLNF("VS1053 found");

    if (!SD.begin(PIN_VS1053_CARDCS)) {
        DPRINTLNF("SD failed, or not present");
        while (1);
    }

    musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);

    // Set up volume knob
    pinMode(PIN_VOLUME, INPUT);
    setVolume();
}

// Handle player state
void handlePlayer() {
    // Handle volume pot
    p.volume = map(analogRead(PIN_VOLUME), 0, 1023, 0, VOLUME_MIN);
    setVolume();

    resumePlayback();
    powerReminder();
    advanceTrack();
}

// ##################################
// Handles the god mode
// Sequence: 1 - 2 - 4 - 8
// ##################################

// Toggles god mode
void toggleGodMode() {
    resetPlayback();

    musicPlayer.sineTest(150, 500);
    musicPlayer.stopPlaying();

    p.isGodMode = !p.isGodMode;

    if (p.isGodMode) {
        DPRINTLNF("Godmode enabled");
        p.currentAlbum[0] = 'g';
    } else {
        DPRINTLNF("Godmode disabled");
        p.currentAlbum[0] = 'k';
    }
}

// Detects god mode sequence
bool detectGodMode(char c) {
    if (p.godModeFlag == 0 && c == '1') {
        p.godModeFlag++;
        return false;
    } else if (p.godModeFlag == 1 && c == '2') {
        p.godModeFlag++;
        return false;
    } else if (p.godModeFlag == 2 && c == '4') {
        p.godModeFlag++;
        return false;
    } else if (p.godModeFlag == 3 && c == '8') {
        toggleGodMode();
        return true;
    }

    p.godModeFlag = 0;
    return false;
}
