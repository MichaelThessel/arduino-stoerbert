#include "player.h"

#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <Arduino.h>
#include <SD.h>
#include <stdint.h>
#include <stdlib.h>

#include "pins.h"
#include "debug.h"

uint8_t volume = 50; // Current Volume level
player p = {{}, "k01", 0, 0, false, 0, false};

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
void setVolume(uint8_t volume) {
    if (volume > VOLUME_MIN) {
        volume = VOLUME_MIN;
    }

    //DPRINTF("Setting volume to: ");
    //DPRINTLN(volume);
    musicPlayer.setVolume(volume, volume);
}

// Increase volume
void increaseVolume() {
    if (volume - VOLUME_STEP < 0) {
        volume = 0;
    } else {
        volume -= VOLUME_STEP;
    }
    setVolume(volume);
}

// Decrease volume
void decreaseVolume() {
    if (volume + VOLUME_STEP > VOLUME_MIN) {
        volume = VOLUME_MIN;
    } else {
        volume += VOLUME_STEP;
    }
    setVolume(volume);
}

// ##################################
// Player controls
// ##################################

// Reset album playback
void resetPlayback() {
    p.isPlaying = false;
    musicPlayer.stopPlaying();
    p.currentTrack = 0;
}

// Play individual file
void playFile() {
    char path[99];
    sprintf(path, "/%s/%s", p.currentAlbum, p.album[p.currentTrack]);

    if (!SD.exists(path)) {
        DPRINTF("File missing: ");
        DPRINTLN(path);
        return;
    }

    DPRINTF("Playing file ");
    DPRINTLN(path);

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
    musicPlayer.stopPlaying();
    playFile();
}

// Skip backward
void playPreviousTrack() {
    if (p.currentTrack == 0) {
        return;
    }

    p.currentTrack--;
    musicPlayer.stopPlaying();
    playFile();
}

// Sets the current album
void setAlbum(char c) {
    p.currentAlbum[2] = c;
}

// Play album
void playAlbum() {
    loadAlbum();

    if (p.currentAlbumTrackCount == 0) {
        DPRINTLNF("No tracks found");
        return;
    }

    resetPlayback();
    p.isPlaying = true;

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
        resetPlayback();
        DPRINTLNF("End of album reached");
        return;
    }

    playNextTrack();
}

// Set up player
void setupPlayer() {
    // Set up volume knob
    pinMode(PIN_VOLUME, INPUT);
    setVolume(volume);

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
}

// Handle player state
void handlePlayer() {
    // Handle volume pot
    setVolume(map(analogRead(PIN_VOLUME), 0, 1023, 0, VOLUME_MIN));

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
