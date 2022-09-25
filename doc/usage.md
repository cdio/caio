# caio - 8 bits home computers emulator

## Usage & Configuration

```
$ caio --help
usage: ./caio <arch> [--help]
where arch is one of:
c64

```

```
$ caio c64 --help
usage: c64 <options>
where <options> are:
 --conf <confile>       Configuration file
                        (default is caio.conf)
 --romdir <romdir>      ROMs directory
 --cartdir <cdir>       Cartridge directory
 --palettedir <pdir>    Colour palette directory
 --palette <palette>    Colour palette file
 --keymapsdir <kdir>    Key mappings directory
 --keymaps <keymaps>    Key mappings file
 --fps <rate>           Frame rate
                        (default is 50)
 --scale <scale>        Graphics scale factor
                        (default is 1)
 --scanlines <v|h|n>    Scanlines effect (horizontal, vertical or none)
                        (default is "n")
 --fullscreen           Launch in fullscreen mode
 --sresize <yes|no>     Smooth window resize
                        (default is yes)
 --audio <yes|no>       Enable (disable) audio
                        (default is yes)
 --delay <delay>        Speed delay factor
                        (default is 1)
 --monitor              Activate the CPU monitor
 --logfile <file>       Write log information into the specified file
                        (default is /dev/tty)
 --loglevel <lv>        Use the specified loglevel; a combination of:
                        error|warn|info|debug|all
                        (default is "")
 -v|--version           Show version information and exit
 -h|--help              Print this message and exit

C64 specific:
 --cart <cart>          Attach a cartridge (CRT format)
 --prg <prg>            Load a PRG file as soon as the basic is ready
 --8 <path>             Attach a disk drive unit 8
 --9 <path>             Attach a disk drive unit 9
 --resid <yes|no>       Use the MOS6581 reSID library
                        (default is no; caio's implementation is used)
 --swapj                Swap Joysticks

```

All these command line options can be specified in a
[configuration file](../src/main/caio.conf).

caio will search for the configuration file as follows, stopping at the first
match:

1. Command line option               `--conf`
2. User's configuration directory:   `$HOME/.config/caio/caio.conf`
3. System's configuration directory: `$PREFIX/etc/caio/caio.conf`

Command line options take precedence over those defined in the
configuration file.

Examples:

The following command activates the horizontal scanlines visual effect, scales
up the emulated screen resolution 3 times (that is, a 320x200 screen is scaled
to 960x600), loads and launches the cartridge named *ghostngobblins*:

```
    $ caio c64 --scanlines h --scale 3 --cart /games/c64/ghostngobblins.crt
```

The next command instructs caio to build a
[Commodore 64](https://en.wikipedia.org/wiki/Commodore_64) using the
[reSID](https://en.wikipedia.org/wiki/ReSID) implementation of the
[MOS 6581](https://en.wikipedia.org/wiki/MOS_Technology_6581) chip,
it then injects a `PRG` program into memory and launches it as soon as the
basic is started:

```
    $ c64 --prg /sid/fanatics/music.prg --resid yes
```

Since the program is injected directly into RAM before the actual emulator
is started, the previous command won't work for advanced or big files that are
expected to overwrite memory areas not configured as RAM.

Under linux, caio can be launched from a GUI (like a file manager) only if it
is not configured to send log messages to the terminal (because there won't
be one) if that happens the emulator is silently terminated (see the `logfile`
configuration file option).


## Keyboard

The keyboard layout can be set using the `keymaps` configuration option
(the default is [US-ANSI](https://en.wikipedia.org/wiki/File:ANSI_Keyboard_Layout_Diagram_with_Form_Factor.svg)).

For example, to use the italian layout:

```
    $ c64 --keymaps it
```

to use the [VICEKB](https://vice-emu.pokefinder.org/images/b/b8/C64keyboard.gif)
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
a text editor and new layouts can be added to the system by placing the new
file inside the `keymasdir` directory which defaults to
`$PREFIX/share/caio/keymaps`.

Like the [VICE](https://en.wikipedia.org/wiki/VICE) emulator, the `RESTORE`
key is mapped as `Page-Up`, `RUN/STOP` as `ESC`, `CTRL` as `TAB` and
`CBM` as `LEFT-CTRL`.


## Joysticks

Up to two gamepads should work (tested using only one Logitech F710).
The PS3 controller is known to work.


## Hot-Keys

* `ALT-F` toggles between *windowed* and *fullscreen* modes.

* `PAUSE` or `ALT-P` toggles between *pause* and *running* modes.

* `ALT-J` swaps joysticks #1 and #2.

* `ALT-M` enters the CPU monitor (if it is active).

* `CTRL-C` on the terminal enters the CPU monitor (if the monitor is not
  active the emulation is terminated).

* `ALT-V` toggles the visibility of the info panel.


## Disk drive

There is a [C1541](https://en.wikipedia.org/wiki/Commodore_1541) implementation
that access the host filesystem. Host directories are recursively traversed so
it must be used with care.

`D64` files are not supported yet.

For more information see the `--8` command line option.

