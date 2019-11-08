#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

#define SHIELD_RESET -1 // VS1053 reset pin (unused!)
#define SHIELD_CS 7 // VS1053 chip select pin (output)
#define SHIELD_DCS 6 // VS1053 Data/command select pin (output)
#define CARDCS 4 // Card chip select pin
#define DREQ 3 // VS1053 Data request, ideally an Interrupt pin


#define DEBUG

#ifdef DEBUG
#define DPRINTLN(x) Serial.println(x)
#define DPRINTLNF(x) Serial.println(F(x))
#define DPRINT(x) Serial.print(x)
#define DPRINTF(x) Serial.print(F(x))
#else
#define DPRINTLN(x)
#define DPRINTLNF(x)
#define DPRINT(x)
#define DPRINTF(x)
#endif


Adafruit_VS1053_FilePlayer musicPlayer =
 Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

const uint8_t VOLUME_STEP = 10;
const uint8_t VOLUME_MIN = 75;
const uint8_t MAX_TRACKS = 255;

uint8_t volume = 50;
char *album[MAX_TRACKS];

void setup() {
    #ifdef DEBUG
    Serial.begin(9600);
    #endif

    if (!musicPlayer.begin()) {
        DPRINTLNF("Couldn't find VS1053");
        while (1);
    }
    DPRINTLNF("VS1053 found");

     if (!SD.begin(CARDCS)) {
        DPRINTLNF("SD failed, or not present");
        while (1);
    }

    // Init player
    musicPlayer.setVolume(volume, volume);
    musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);
}

void loop() {
    #ifdef DEBUG
    if (Serial.available()) {
        char c = Serial.read();

        switch (c) {
            // Start playing
            case'1':
            case'2':
            case'3':
            case'4':
            case'5':
            case'6':
            case'7':
            case'8':
            case'9':
                DPRINTF("Received Command: PLAY ");
                DPRINTLN(c);
                playAlbum(c);
                break;
            // Toggle play/pause
            case'p':
                DPRINTLNF("Received Command: TOGGLE PLAY/PAUSE");
                playTogglePause();
                break;
            case'f':
                DPRINTLNF("Received Command: NEXT TRACK");
                playNextTrack();
                break;
            case'b':
                DPRINTLNF("Received Command: PREVIOUS TRACK");
                playPreviousTrack();
                break;
            // Increase volume
            case'+':
                DPRINTLNF("Received Command: VOLUME +");
                increaseVolume();
                break;
            // Decrease volume
            case'-':
                DPRINTLNF("Received Command: VOLUME -");
                decreaseVolume();
                break;
        }
    }
    #endif

    delay(100);
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

void setVolume(uint8_t volume) {
    DPRINTF("Setting volume to: ");
    DPRINTLN(volume);
    musicPlayer.setVolume(volume, volume);
}

// ##################################
// Player controls
// ##################################

// Toggle Play/Pause
void playTogglePause() {
    if (musicPlayer.paused()) {
        musicPlayer.pausePlaying(false);
    } else {
        musicPlayer.pausePlaying(true);
    }
}

// Skip forward
void playNextTrack() {
}

// Skip backward
void playPreviousTrack() {
}

void playAlbum(char c) {
    char folder[4] = {'/', c, '/', '\0'};
    loadAlbum(folder);

    char b[10];
    sprintf(b, "%s/%s", folder, album[0]);
    playFile(b);
}

void playFile(char path[]) {
    DPRINTF("Playing file ");
    DPRINTLN(path);
    musicPlayer.startPlayingFile(path);
}



// ##################################
// File handling
// ##################################

// Load album files
// Directory and file structure needs to be:
// /[1-9]/[01-99].mp3
void loadAlbum(char folder[]) {
    DPRINTF("Loading files from directory: ");
    DPRINTLN(folder);

    File dir = SD.open(folder);

    // Process diretories in /
    int i = 0;
    while (true) {
        File entry = dir.openNextFile();

        if (!entry) {
            break;
        }

        // Ignore everything but files
        if (entry.isDirectory()) {
            continue;
        }

        // TODO: file extension validatiion

        DPRINTLN(entry.name());
        album[i] = malloc(strlen(entry.name()) + 1);
        strcpy(album[i], entry.name());
        i++;

        entry.close();
    }
}
