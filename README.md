# Intro

This repository contains the files required to build a
[Hoerbert](https://en.hoerbert.com) clone. This is intended to be an easy to use
MP3 player for small children.

Main commponents required to build:

* Arduino Uno
* Adafruit Music Maker shield

# God mode

The player supptorts 2 differnt modes. Regular mode and god mode.
In regular mode one album is assigned to each button. This is the default mode
and what the device is mainly intended for.
To add more functionality, god mode has been added. This mode allows to add an
alternative set of albums which makes it a handy MP3 player for parents as well.
In that mode instead of dirictly playing an album for each button press, the
buttons act as numerical input buttons. 2 buttons have to be pressed to select
an album. I.e. 4 - 2 to select album 42. Up to 99 albums are supported in
god mode.

To enter god mode the god mode sequence has to be pressed. The god mode sequence
is: prev - play_pause - play_ pause - next (regular mode keyboard layout). The
same sequence can be used to switch back to regular mode. When the seqence has
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

    a=`find ./ -type f | awk '{print "mv", "\""$0"\"", substr($0, 3, 2)".mp3;"}'`; eval $a

This only works if the files are named with the first 2 digits indicating their
track name already.

# Button layout

## Regular mode

    [1]         [2]             [3]
    [4]         [5]             [6]
    [7]         [8]             [9]
    [prev]      [play_pause]    [next]

## God mode

    [1]         [2]             [3]
    [4]         [5]             [6]
    [7]         [8]             [9]
    [0]         [play_pause]    [next]

# TODO

* [Sleep mode](http://www.vlsi.fi/fileadmin/app_notes/vs1003an_adcpwrdn.pdf)
* [Mono mode](https://github.com/sparkfun/LilyPad_MP3_Player/blob/7c8aa00545772183db7df8860a754d2f9288356a/Arduino/libraries/SFEMP3Shield/SFEMP3Shield.cpp#L989)

