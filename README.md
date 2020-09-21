# CEMU

CEMU is an emulator of 8 bit Home Computers.
The current status is **Work-in-Progress alpha** and only the Commodore 64
is emulated, under Linux.

![c64](/images/c64.gif "CEMU C64")
![gyruss](/images/gyruss.gif "Gyruss")
![q-bert](/images/q-bert.gif "Q*Bert")

[![zauberwald](/images/zauberwald.png "Zauberwald")](https://csdb.dk/release/?id=188005)
[![compopicasso](/images/compopicasso.png "Compopicasso")](https://csdb.dk/release/?id=185650)


## Compile & Install

CEMU requires [clang-6](https://clang.llvm.org) or later,
[SFML](https://www.sfml-dev.org) 2.5.1 or later and
[pkg-config](https://en.wikipedia.org/wiki/Pkg-config):

```
    $ sudo apt install clang
    ...
    $ sudo apt install libsfml-dev
    ...
    $ sudo apt install pkg-config
    ...
    $ make PREFIX=/usr/local
    ...
    $ sudo make PREFIX=/usr/local install
```

The default *PREFIX* is */opt/cemu*.


## Usage

```
    $ cemu c64 --help
```
or
```
    $ c64 --help
```
All the command line options can be specified in a
[configuration file](/bin/cemu.conf).

To launch a program built in a CRT file:

```
    $ c64 --scanlines h --scale 4 --cart ./gyruss.crt
```
if it is a PRG file:

```
    $ c64 --prg ./rambo.prg
```


## Keyboard

The keyboard layout can be set using the *keymaps* configuration option
(the default is [US-ANSI](https://en.wikipedia.org/wiki/File:ANSI_Keyboard_Layout_Diagram_with_Form_Factor.svg)).

For exapmple, to use the italian layout:
```
    $ c64 --keymaps it
```
to use the [VICEKB](https://vice-emu.pokefinder.org/index.php/File:C64keyboard.gif)
positional layout:
```
    $ c64 --keymaps vice
```

At the moment the following layouts are available (not all of them tested):
* Italian (it)
* German (de)
* Swiss (ch)
* UK (gb)
* US-ANSI (default)
* VICEKB (vice)

One of the problems of these mappings (with the exception of VICEKB) is that
some C64 "special characters" cannot be entered; for instance, *SHIFT-** or
*SHIFT-@* on the C64 display characters that cannot be entered when using any
of these layouts.
This limitation can be workarounded by just editing (using a text editor)
any of the layout files or adding a new one with new key combinations to
reproduce those characters.

Layouts files (with the exception of the default one) are stored in the
*keymapsdir* directory.

Like the [VICE](https://en.wikipedia.org/wiki/VICE) emulator, the *RESTORE*
key is mapped as *Page-Up*, *RUN/STOP* as *ESC*, *CTRL* as *TAB* and
*CBM* as *LEFT-CTRL*.


## Joysticks

Up to two gamepads should work (tested using only one Logitech F710).


## Hot-Keys

* *F11* toggles between *windowed* and *fullscreen* modes.

* *PAUSE* or *ALT-P* toggles between *pause* and *running* modes.

* *ALT-J* swap joysticks #1 and #2.

* *ALT-M* enters the CPU monitor (if it is active).

* *CTRL-C* on the terminal enters the CPU monitor (if the monitor is not
  active the emulation is terminated).


## Disk Drive

There is a [C1541](https://en.wikipedia.org/wiki/Commodore_1541) implementation that
access the host filesystem. Host directories are recursively traversed so it must be
used with care.

D64 files are not supported yet.

For more information please check the *--8* command line option.


## C64 SID Chip

The emulation of the MOS6581 chip is based on the specifications found on the
[C64 User's Guide](https://www.c64-wiki.com/wiki/Commodore_64_User%27s_Guide)
but since ["Many characteristics of the SID sound are bugs"](https://www.c64-wiki.com/wiki/SID#Trivia)
CEMU's implementation does not sound 100% like the original chip.
Nevertheless, the volume "click" bug and others are implemented so programs
like [SAM](https://en.wikipedia.org/wiki/Software_Automatic_Mouth) work.

Besides the internal implementation, CEMU has a wrapper that uses the very
accurate [reSID](https://en.wikipedia.org/wiki/ReSID) library; it can be
enabled using the *resid* option.


## Status

Although CEMU is usable, it is not finished and it is not free of bugs.
the chipset emulation is not 100% complete neither accurate.


## TODO

In no particular order:

* Finish Commodore 64
* Apple-2
* BBC Micro
* Daewoo DPC-200
* NEC PC-8001
* Amstrad CPC-464
* ZX Spectrum
