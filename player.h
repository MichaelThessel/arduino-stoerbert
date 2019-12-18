#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>

const uint8_t VOLUME_STEP = 10;
const uint8_t VOLUME_MIN = 75;
const uint8_t MAX_TRACKS = 30;

const uint8_t FOLDER_NAME_LENGTH = 4;

const uint32_t POWER_REMINDER_PERIOD = 300000;

// Player holds current player state
struct player {
    char *album[MAX_TRACKS];                // Album track buffer
    char currentAlbum[FOLDER_NAME_LENGTH];  // Currently selected album
    uint8_t currentAlbumTrackCount;         // Track count for currently selected album
    uint8_t currentTrack;                   // Current track
    bool isPlaying;                         // Whether or not there is currently an album playing
    uint8_t godModeFlag;                    // God detction mode flag
    bool isGodMode;                         // Whether or not god mode is enabled
    uint8_t godModeAlbumIndex;              // Which index to set when selecting the album in god mode
    uint8_t volume;                         // Current Volume level
    bool hasResumed;                        // Whether or not the player has tried to resume
    uint32_t powerReminderDelay;            // Time when to run the next power reminder
};

void setupPlayer();
void handlePlayer();

void increaseVolume();
void decreaseVolume();
void togglePlayPause();
void playNextTrack();
void playPreviousTrack();
bool setAlbum(char c);
void playAlbum();
bool detectGodMode(char c);
void toggleGodMode();
bool isGodMode();

#endif
