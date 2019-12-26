# Intro

This repository contains the files required to build an easy to use MP3 player for small children.
This project is heavily inspired by [Hoerbert](https://en.hoerbert.com).

![PCB CAD](/assets/final.jpg)
![Inside](/assets/inner_front.jpg)
![Inside](/assets/inner_back.jpg)

Main components required to build:

* Arduino Uno
* Adafruit Music Maker shield
* Speaker
* Button PCB (see below)
* Volume Pot
* On/Off switch
* Battery holders
* Hookup wire

# Button PCB schematics

I created a custom PCB for the input buttons. The schematics for that are in the
cad directory in KiCad format. The Arduino UNO does not have enough GPIO pins
for the Music Maker shield, the volume pot, and 12 buttons. To reduce the amount
of GPIO pins required for the buttons I used 2 shift registers. That reduces the
amount of required pins for the buttons to 3.

I used the following components:

* [2x CD4021BC shift registers](http://www.redrok.com/CMOS_CD4021BC_8-StageStaticShiftRegister_Fairchild.pdf)
* [OmronB3F Buttons](https://www.amazon.ca/gp/product/B07CW1XJTS)
* as well as some 0805 1k resistors

![PCB CAD](/assets/pcb_cad.jpg)
![PCB etched](/assets/pcb_etched.jpg)
![PCB populated](/assets/pcb_populated.jpg)

If you want to etch the PCB yourself you can print this PDF on a transparency.
Make sure to use US letter sized transparencies if you use the PDF below.
Alternatively you can just create a new PDF from the KiCad layout.

[PCB transparency](/assets/pcb_cad.pdf)

I followed this [PCB etching process](https://www.youtube.com/watch?v=tWnfnt2rNO0)

# Button layout

## Regular mode

    [album 1]   [album 2]       [album 3]
    [album 4]   [album 5]       [album 6]
    [album 7]   [album 8]       [album 9]
    [prev]      [play_pause]    [next]

## God mode

    [1]         [2]             [3]
    [4]         [5]             [6]
    [7]         [8]             [9]
    [0]         [play_pause]    [next]

# God mode

The player supports 2 different modes. Regular mode and god mode.
In regular mode one album is assigned to each button. This is the default mode
and what the device is mainly intended for.
To add more functionality, god mode has been added. This mode allows to add an
alternative set of albums which makes it a handy MP3 player for parents as well.
In that mode instead of directly playing an album for each button press, the
buttons act as numerical input buttons. 2 buttons have to be pressed to select
an album. I.e. 4 - 2 to select album 42. Up to 99 albums are supported in
god mode.

To enter god mode the god mode sequence has to be pressed. The god mode sequence
is: prev - play_pause - play_ pause - next (regular mode keyboard layout). The
same sequence can be used to switch back to regular mode. When the sequence has
been entered correctly a short beep will sound.

# SD card

## Structure

The SD card needs to be formated with FAT32.

The root folder structure needs to be:

k01 ... k09 for the regular albums and

g01 ... g99 for the god mode albums

The files in each folder need to be named:

01.mp3 ... 99.mp3

i.e.

    /k06/01.mp3
    /k06/02.mp3
    /k06/03.mp3
    /k06/04.mp3
    /k06/05.mp3
    /k06/06.mp3

## Convert files to supported file format

    a=`find ./ -type f -name "*.mp3" | awk '{print "mv", "\""$0"\"", substr($0, 3, 2)".mp3;"}'`; eval $a

This only works if the files are named with the first 2 digits indicating their
track name already. I.e. `01-meshuggah-bleed.mp3`. A great alternative for batch
renaming the files is [Mp3Tag](https://www.mp3tag.de/en).

# Auto resume

The player will automatically resume the last played track after power off (if
powered off before an album finished playing).

# Power saving

To remind the user to turn of the device once the album has finished playing the
player will sound a short beep every 5 minutes once the last album has played.

# Mono mode

To keep the player compact and reduce power consumption this has been designed
to work with a single speaker. The Music Maker shield is configured to work in
mono mode. You can connect your speaker to either of the 2 speaker ports. In
case you want to build a stereo version you can simply remove these lines from
player.cpp

    // Enable mono mode
    vs1053.sciWrite(VS1053_REG_WRAMADDR, 0x1e09);
    vs1053.sciWrite(VS1053_REG_WRAM, 0x0001);
