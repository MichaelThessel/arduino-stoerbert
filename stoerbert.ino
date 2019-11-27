/**
TODO:

* File sorting/file name based playback
* Mono Mode
* God mode album select
* Button handling:
 * Play/Pause
 * Forward
 * Backward
 * Album select regular
 * Album select god mode
* Sleep mode
* Save current track on EEPROM

Reseach if headphone jack is possible

*/
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

#define PIN_VS1053_SHIELD_RESET -1 // VS1053 reset pin (unused!)
#define PIN_VS1053_SHIELD_CS 7 // VS1053 chip select pin (output)
#define PIN_VS1053_SHIELD_DCS 6 // VS1053 Data/command select pin (output)
#define PIN_VS1053_CARDCS 4 // Card chip select pin
#define PIN_VS1053_DREQ 3 // VS1053 Data request, ideally an Interrupt pin
#define PIN_VOLUME A0 // Volume knob pin
#define PIN_SR_DATA 2 // Shift register data pin
#define PIN_SR_CLOCK 5 // Shift register data pin
#define PIN_SR_LATCH 8 // Shift register latch pin

#define DEBUG
#ifdef DEBUG
#define DPRINTLN(x) Serial.println(x)
#define DPRINTBINLN(x) Serial.println(x, BIN)
#define DPRINTLNF(x) Serial.println(F(x))
#define DPRINT(x) Serial.print(x)
#define DPRINTBIN(x) Serial.print(x, BIN)
#define DPRINTF(x) Serial.print(F(x))
#else
#define DPRINTLN(x)
#define DPRINTBINLN(x)
#define DPRINTLNF(x)
#define DPRINT(x)
#define DPRINTBIN(x)
#define DPRINTF(x)
#endif

Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(
    PIN_VS1053_SHIELD_RESET,
    PIN_VS1053_SHIELD_CS,
    PIN_VS1053_SHIELD_DCS,
    PIN_VS1053_DREQ,
    PIN_VS1053_CARDCS
);

const uint8_t VOLUME_STEP = 10;
const uint8_t VOLUME_MIN = 75;
const uint8_t MAX_TRACKS = 30;
const uint8_t DEBOUNCE_DELAY = 200;

uint8_t volume = 50; // Volume level
char *album[MAX_TRACKS]; // Album track buffer
char currentAlbum[] = "k01"; // Currently selected album
uint8_t currentAlbumTrackCount = 0; // Track count for currently selected album
uint8_t currentTrack = 0; // Current track
bool isPlaying = false; // Whether or not there is currently an album playing
uint8_t gmflag = 0; // God detction mode flag
bool isGodMode = false; // Whether or not god mode is enabled

uint8_t sr1State = 0; // Shift register 1 current state
uint8_t sr2State = 0; // Shift register 2 current state
uint8_t sr1StatePrevious = -1; // Shift register 1 previous state
uint8_t sr2StatePrevious = -1; // Shift register 2 previouse state
unsigned long sr1LastDebounceTime = 0; // Shift register 1 time since last debounce
unsigned long sr2LastDebounceTime = 0; // Shift register 2 time since last debounce

// ##################################
// Setup
// ##################################

void setup() {
    setupSerial();
    setupVS1053();
    setupButtons();
}

// Set up serial connection
void setupSerial() {
    #ifdef DEBUG
    Serial.begin(9600);
    #endif
}

// Set up player
void setupVS1053() {
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

void setupButtons() {
    // Set up volume knob
    pinMode(PIN_VOLUME, INPUT);
    setVolume(volume);

    // Set up shift registers for buttons
    pinMode(PIN_SR_LATCH, OUTPUT);
    pinMode(PIN_SR_CLOCK, OUTPUT);
    pinMode(PIN_SR_DATA, INPUT);
}

// ##################################
// Main loop
// ##################################

void loop() {
    #ifdef DEBUG
    handleSerial();
    #endif

    advanceTrack();

    handleButtons();
}

// Button handling
void handleButtons() {
    // Handle volume pot
    setVolume(map(analogRead(PIN_VOLUME), 0, 1023, 0, VOLUME_MIN));

    // Handle button shift registers
    digitalWrite(PIN_SR_LATCH, 1);
    delayMicroseconds(20);
    digitalWrite(PIN_SR_LATCH, 0);
    sr1State = shiftIn();
    sr2State = shiftIn();
    //DPRINTBINLN(sr1State);
    //DPRINTBINLN(sr2State);

    if (debounce(&sr1State, &sr1StatePrevious, &sr1LastDebounceTime)) {
        if (sr1State & 0b00001000) {
            togglePlayPause();
        }

        if (sr1State & 0b10000000) {
            currentAlbum[2] = '1';
            playAlbum();
        }
    }
}

// ##################################
// VS1053 controls
// ##################################

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

// Set volume
void setVolume(uint8_t volume) {
    if (volume > VOLUME_MIN) {
        volume = VOLUME_MIN;
    }

    //DPRINTF("Setting volume to: ");
    //DPRINTLN(volume);
    musicPlayer.setVolume(volume, volume);
}

// ##################################
// Player controls
// ##################################

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
    if (currentTrack == currentAlbumTrackCount - 1) {
        return;
    }

    currentTrack++;
    musicPlayer.stopPlaying();
    playFile();
}

// Skip backward
void playPreviousTrack() {
    if (currentTrack == 0) {
        return;
    }

    currentTrack--;
    musicPlayer.stopPlaying();
    playFile();
}

// Play album
void playAlbum() {
    loadAlbum();

    if (currentAlbumTrackCount == 0) {
        DPRINTLNF("No tracks found");
        return;
    }

    resetPlayback();
    isPlaying = true;

    DPRINTF("Playing album ");
    DPRINTLN(currentAlbum);

    playFile();
}

// Play individual file
void playFile() {
    char path[99];
    sprintf(path, "/%s/%s", currentAlbum, album[currentTrack]);

    DPRINTF("Playing file ");
    DPRINTLN(path);

    musicPlayer.startPlayingFile(path);
}

// Advance to the next track
void advanceTrack()
{
    // Don't do anything if we are not playing
    if (!isPlaying) {
        return;
    }

    // Exit if we are currently playing or paused
    if (musicPlayer.playingMusic || musicPlayer.paused()) {
        return;
    }

    // Exit if we reached the last track
    if (currentTrack == currentAlbumTrackCount - 1) {
        resetPlayback();
        DPRINTLNF("End of album reached");
        return;
    }

    playNextTrack();
}

// Reset album playback
void resetPlayback() {
    isPlaying = false;
    musicPlayer.stopPlaying();
    currentTrack = 0;
}

// Detects the god mode sequence
// Sequence: 1 - 2 - 4 - 8
bool detectGodMode(char c) {
    if (gmflag == 0 && c == '1') {
        gmflag++;
        return false;
    } else if (gmflag == 1 && c == '2') {
        gmflag++;
        return false;
    } else if (gmflag == 2 && c == '4') {
        gmflag++;
        return false;
    } else if (gmflag == 3 && c == '8') {
        toggleGodMode();
        return true;
    }

    gmflag = 0;
    return false;
}

// Toggles god mode
void toggleGodMode() {
    resetPlayback();

    musicPlayer.stopPlaying();
    musicPlayer.sineTest(150, 1000);
    delay(1000);
    musicPlayer.stopPlaying();

    isGodMode = !isGodMode;

    if (isGodMode) {
        DPRINTLNF("Godmode enabled");
        currentAlbum[0] = 'g';
    } else {
        DPRINTLNF("Godmode disabled");
        currentAlbum[0] = 'k';
    }
}

// ##################################
// File handling
// ##################################

// Load album files
// Directory and file structure needs to be:
// /[1-9]/[01-99].mp3
void loadAlbum() {
    char folder[6];
    sprintf(folder, "/%s/", currentAlbum);

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

        DPRINTLN(entry.name());

        if (!musicPlayer.isMP3File(entry.name())) {
            DPRINTLNF("Not a MP3 skipping");
            continue;
        }

        // TODO: sorting

        album[i] = malloc(strlen(entry.name()) + 1);
        strcpy(album[i], entry.name());
        i++;

        entry.close();
    }

    currentAlbumTrackCount = i;
}

// ##################################
// Serial controls
// ##################################
void handleSerial() {
    if (Serial.available()) {
        char c = Serial.read();

        switch (c) {
            // Start playing
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                DPRINTF("Received Command: PLAY ");
                DPRINTLN(c);
                currentAlbum[2] = c;
                if (!detectGodMode(c)) {
                    playAlbum();
                };
                break;

            // GOD mode !!!
            case 'g':
                DPRINTLNF("Received Command: God Mode");
                toggleGodMode();
                break;

            // Toggle play/pause
            case 'p':
                DPRINTLNF("Received Command: TOGGLE PLAY/PAUSE");
                togglePlayPause();
                break;

            // Next track
            case 'f':
                DPRINTLNF("Received Command: NEXT TRACK");
                playNextTrack();
                break;

            // Previous track
            case 'b':
                DPRINTLNF("Received Command: PREVIOUS TRACK");
                playPreviousTrack();
                break;

            // Increase volume
            case '+':
                DPRINTLNF("Received Command: VOLUME +");
                increaseVolume();
                break;

            // Decrease volume
            case '-':
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
}

// ##################################
// Helper functions
// ##################################

// Returns current shift register state as byte
// Bit 7 = Pin 7 / Bit 0= Pin 0
byte shiftIn() {
    byte data = 0;

    for (int i = 7; i >= 0; i--) {
        digitalWrite(PIN_SR_CLOCK, 0);
        delayMicroseconds(2);
        if (digitalRead(PIN_SR_DATA)) {
            data = data | (1 << i);
        }
        digitalWrite(PIN_SR_CLOCK, 1);

    }

    return data;
}

// Debounce button presses
bool debounce(uint8_t *current, uint8_t *previous, unsigned long *time) {
    // First run
    if (*previous == -1) {
        *previous = *current;
        return false;
    }

    if (*current != *previous) {
        *previous = *current;

        if ((millis() - *time) > DEBOUNCE_DELAY) {
            return true;
        }

        *time = millis();
    }

    return false;
}
