# CEMU

CEMU is an emulator of 8 bit Home Computers.

CEMU is currently **Work-in-Progress** and at the moment only the Commodore 64 is
emulated, under Linux.

![c64](/images/c64.gif "CEMU C64")
![gyruss](/images/gyruss.gif "Gyruss")
![q-bert](/images/q-bert.gif "Q*Bert")
![uridium](/images/uridium.gif "Uridium")
![giana](/images/giana.gif "Great Giana Sisters")

[![zauberwald](/images/zauberwald.png "Zauberwald")](https://csdb.dk/release/?id=188005)


## Compile, Install and Package

### Dependencies:
* C++20 compiler: [clang-10.0](https://clang.llvm.org) or later or
  [g++-9.3](https://en.wikipedia.org/wiki/GNU_Compiler_Collection) or later.

* [SFML-2.5.1](https://www.sfml-dev.org) or later.

* [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config)


```
    $ sudo apt install clang
    ...
    $ sudo apt install libsfml-dev
    ...
    $ sudo apt install pkg-config
    ...
```

### Compile:

```
    $ make PREFIX=/usr/local
    ...
```

The default compiler is [clang++](https://clang.llvm.org), it can be changed to
[g++](https://en.wikipedia.org/wiki/GNU_Compiler_Collection) by setting the *CXX*
environment variable:

```
    $ make CXX=g++ PREFIX=/usr/local
    ...
```

### Install:

```
    $ sudo make PREFIX=/usr/local install
    ...
```

The default *PREFIX* is */opt/cemu*.

### Package:

#### Source code package:

```
    $ make src-package RELEASE=<tag>
```

That creates a *.tgz* tarball file under the *build/* directory.

*tag* specifies the CEMU's git branch or tag to pack.

#### Debian binary package:

```
    $ make deb-package RELEASE=<tag>
```

That creates a *.deb* binary file under the *build/* directory.

*tag* specifies the CEMU's git branch or tag to build.

#### Generic binary package:

```
    $ make bin-package RELEASE=<tag>
```

That creates a *.tgz* (tarball) binary file under the *build/* directory.

*tag* specifies the CEMU's git branch or tag to build.

Note that this tarball file does not contain any runtime dependencies (libsfml, libstdc++, etc).
Those must be installed separately.


## Usage & Configuration

```
    $ cemu c64 --help
```
or
```
    $ c64 --help
```
All the command line options can be specified in a
[configuration file](/src/main/cemu.conf).

CEMU will search for the configuration file as follows, stopping at the first match:
1. Command line option               *--conf*
2. Current working directory:        *./cemu.conf*
3. User's configuration directory:   *$HOME/.config/cemu/cemu.conf*
4. System's configuration directory: *$PREFIX/etc/cemu/cemu.conf*

Command line options take precedence over those defined in the configuration file.


## Usage Examples

To attach a cartridge (*.crt*) image:

```
    $ c64 --scanlines h --scale 4 --cart ./gyruss.crt
```
To launch a program (*.prg*) file as soon as the basic is started:

```
    $ c64 --prg ./rambo.prg
```


## Keyboard

The keyboard layout can be set using the *keymaps* configuration option
(the default is [US-ANSI](https://en.wikipedia.org/wiki/File:ANSI_Keyboard_Layout_Diagram_with_Form_Factor.svg)).

For example, to use the italian layout:
```
    $ c64 --keymaps it
```
to use the [VICEKB](https://vice-emu.pokefinder.org/index.php/File:C64keyboard.gif)
positional layout:
```
    $ c64 --keymaps vice
```

At the moment the following layouts are available (not all of them fully tested):
* Italian (it)
* German (de)
* Swiss (ch)
* UK (gb)
* US-ANSI (default)
* VICEKB (vice)

Keyboard layouts are simple text files, existing layouts can be modified using
a text editor and new layouts can be added to the system by just placing the
new file inside the *keymasdir* directory which defaults to
*$PREFIX/share/cemu/keymaps*.

Like the [VICE](https://en.wikipedia.org/wiki/VICE) emulator, the *RESTORE*
key is mapped as *Page-Up*, *RUN/STOP* as *ESC*, *CTRL* as *TAB* and
*CBM* as *LEFT-CTRL*.


## Joysticks

Up to two gamepads should work (tested using only one Logitech F710).
The PS3 controller is known to work.


## Hot-Keys

* *ALT-F* toggles between *windowed* and *fullscreen* modes.

* *PAUSE* or *ALT-P* toggles between *pause* and *running* modes.

* *ALT-J* swaps joysticks #1 and #2.

* *ALT-M* enters the CPU monitor (if it is active).

* *CTRL-C* on the terminal enters the CPU monitor (if the monitor is not
  active the emulation is terminated).

* *ALT-V* toggles the visibility of the info panel.


## Disk Drive

There is a [C1541](https://en.wikipedia.org/wiki/Commodore_1541) implementation that
access the host filesystem. Host directories are recursively traversed so it must be
used with care.

D64 files are not supported yet.

For more information please see the *--8* command line option.


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
