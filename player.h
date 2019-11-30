#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>

const uint8_t VOLUME_STEP = 10;
const uint8_t VOLUME_MIN = 75;
const uint8_t MAX_TRACKS = 30;

// Player holds current player state
struct player {
    char *album[MAX_TRACKS];        // Album track buffer
    char currentAlbum[4];           // Currently selected album
    uint8_t currentAlbumTrackCount; // Track count for currently selected album
    uint8_t currentTrack;           // Current track
    bool isPlaying;                 // Whether or not there is currently an album playing
    uint8_t godModeFlag;            // God detction mode flag
    bool isGodMode;                 // Whether or not god mode is enabled
};

void setupPlayer();
void handlePlayer();

void increaseVolume();
void decreaseVolume();
void togglePlayPause();
void playNextTrack();
void playPreviousTrack();
void setAlbum(char c);
void playAlbum();
bool detectGodMode(char c);
void toggleGodMode();

#endif
