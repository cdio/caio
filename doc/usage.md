# cemu - 8 bits home computers emulator

## usage & configuration

```
    $ cemu c64 --help
```
or
```
    $ c64 --help
```
All the command line options can be specified in a
[configuration file](/src/main/cemu.conf).

cemu will search for the configuration file as follows, stopping at the first match:
1. Command line option               *--conf*
2. Current working directory:        *./cemu.conf*
3. User's configuration directory:   *$HOME/.config/cemu/cemu.conf*
4. System's configuration directory: *$PREFIX/etc/cemu/cemu.conf*

Command line options take the precedence over those defined in the
configuration file.


## usage examples

To attach a cartridge (*.crt*) image:

```
    $ c64 --scanlines h --scale 4 --cart ./gyruss.crt
```
To launch a program (*.prg*) file as soon as the basic is started:

```
    $ c64 --prg ./rambo.prg
```


## keyboard

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

At the moment the following layouts are available (not all of them fully
tested):
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


## joysticks

Up to two gamepads should work (tested using only one Logitech F710).
The PS3 controller is known to work.


## hot-keys

* *ALT-F* toggles between *windowed* and *fullscreen* modes.

* *PAUSE* or *ALT-P* toggles between *pause* and *running* modes.

* *ALT-J* swaps joysticks #1 and #2.

* *ALT-M* enters the CPU monitor (if it is active).

* *CTRL-C* on the terminal enters the CPU monitor (if the monitor is not
  active the emulation is terminated).

* *ALT-V* toggles the visibility of the info panel.


## disk drive

There is a [C1541](https://en.wikipedia.org/wiki/Commodore_1541) implementation
that access the host filesystem. Host directories are recursively traversed so
it must be used with care.

D64 files are not supported yet.

For more information please see the *--8* command line option.

