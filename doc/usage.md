# caio - 8 bits home computers emulator

## Usage & Configuration

At the moment, the best way to launch caio is from the command line terminal:
```
$ caio --help
usage: caio <arch> [--help]
where arch is one of:
c64
zx80
```

### Info Panel

The Info Panel is a primitive control panel containing widgets that allow
minimal settings during runtime. It can be made visible/invisible with a
mouse right click. The default widgets are:

* Toggle Fullscreen mode
* Suspend/Resume emulation
* Platform reset
* Audio Volume control

There are other widgets that depend on the specifc emulated platform such as:

* Joystick status
* Disk activity

### Hot-Keys

caio accepts the following key combinations:

* `ALT-F` toggles between *windowed* and *fullscreen* modes.

* `PAUSE` or `ALT-P` toggles between *pause* and *running* modes.

* `ALT-J` swaps joysticks #1 and #2.

* `ALT-M` enters the CPU monitor (if it is active). Like `CTRL-C`
  on the terminal.

* `CTRL-C` on the terminal enters the CPU monitor (if the monitor is not
  active the emulation is terminated).

* `ALT-V` toggles the visibility of the info panel.

### Caio generic configuration

Configuration values can be set as command line options or in a
[configuration file](../src/main/caio.conf).
The configuration file contains two types of sections: One _generic_ section
whose values are inherited by all the platforms, and one _specific_ section
for each of the emulated platform.

If a parameter is specified twice (as a platform specific and as a generic
one) the platform specific value takes the precedence.

The configuration file is searched in the following directories (stop at
first match):

1. Command line option               `--conf`
2. User's configuration directory:   `$HOME/.config/caio/caio.conf`
3. System's configuration directory: `$PREFIX/etc/caio/caio.conf`

Command line options take precedence over those defined in the
configuration file.

The following list shows the generic command line options (valid for all
platforms):

```
 --conf <cfile>          Configuration file
 --romdir <romdir>       ROMs directory
 --palettedir <pdir>     Colour palette directory
 --palette <palette>     Colour palette name or filename
 --keymapsdir <kdir>     Key mappings directory
 --keymaps <keymaps>     Key mappings name or filename
 --cart <cfile>          Cartridge filename
 --fps <rate>            Frame rate (default is 50)
 --scale <scale>         Window scale factor (default is 1)
 --scanlines <n|h|v|H|V> Scanlines effect: (n)one, (h)orizontal, (v)ertical,
                         advanced (H)orizontal, advanced (V)ertical
                         (default is n)
 --fullscreen            Start in fullscreen mode
 --sresize <yes|no>      Smooth window resize (default is yes)
 --audio <yes|no>        Enable audio (default is yes)
 --delay <delay>         Clock delay factor (default is 1)
 --monitor               Activate the CPU monitor during boot
 --logfile <file>        Send log information to the specified destination
                         (default is /dev/tty)
 --loglevel <lv>         Loglevel, bitwise combination of:
                         error|warn|info|debug|all (default is none)
 -v|--version            Show version information and exit
 -h|--help               Print this message and exit
```

### Commodore 64 specific

```
$ caio c64 --help
usage: caio c64 <options>
where <options> are:
 ...
Commodore C64 specific:
 --prg <prg>             Load a PRG file as soon as the basic is ready
 --resid <yes|no>        Use the MOS6581 reSID library (default is no)
 --swapj                 Swap Joysticks
 --8 <path>              Attach a disk drive unit 8
 --9 <path>              Attach a disk drive unit 9
```

#### Keyboard

The keyboard layout can be set using the `keymaps` configuration option
(the default is [US-ANSI](https://en.wikipedia.org/wiki/File:ANSI_Keyboard_Layout_Diagram_with_Form_Factor.svg)).

For example, to use the italian layout:

```
    $ caio c64 --keymaps it
```

to use the [VICEKB](https://vice-emu.pokefinder.org/images/b/b8/C64keyboard.gif)
positional layout:

```
    $ caio c64 --keymaps vice
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
file inside the `keymapsdir` directory which defaults to
`$PREFIX/share/caio/keymaps`.

Like the [VICE](https://en.wikipedia.org/wiki/VICE) emulator, the `RESTORE`
key is mapped as `Page-Up`, `RUN/STOP` as `ESC`, `CTRL` as `TAB` and
`CBM` as `LEFT-CTRL`.

#### Disk drives

There is a [C1541](https://en.wikipedia.org/wiki/Commodore_1541) implementation
that access the host filesystem. Host directories are recursively traversed so
it must be used with care.

`D64` files are not supported yet.

For more information see the `--8` and `--9` command line options.

#### Joysticks

Up to two gamepads should work (tested using only one Logitech F710).
The PS3 controller is known to work.

#### Examples:

The following command activates the horizontal scanlines visual effect, scales
up the emulated screen resolution 3 times (that is, a 320x200 screen is scaled
up to 960x600), loads and launches the cartridge named *ghostngobblins*:

```
    $ caio c64 --scanlines h --scale 3 --cart /games/c64/ghostngobblins.crt
```

The next command activates the advanced horizontal scanlines visual effect
(note the captial H), in this mode the specified scale factor is doubled
(that is, a 320x200 screen is scaled up to 1280x800), loads and launches the
cartridge *Simon's Basic*:

```
    $ caio c64 --scanlines H --scale 2 --cart ./simons_basic.crt
```

The next command instructs caio to build a
[Commodore 64](https://en.wikipedia.org/wiki/Commodore_64) using the
[reSID](https://en.wikipedia.org/wiki/ReSID) implementation of the
[MOS 6581](https://en.wikipedia.org/wiki/MOS_Technology_6581) chip,
it then injects a `PRG` program into memory and launches it as soon as the
basic is started:

```
    $ caio c64 --prg /sid/fanatics/music.prg --resid yes
```

The program is injected directly into RAM before the actual emulator is
started, this means that the previous command won't work for advanced or
big files that are expected to overwrite memory areas not configured as RAM.

### Sinclair ZX80 specific
```
$ caio zx80 --help
usage: caio z80 <options>
where <options> are:
 ...
Sinclair ZX80 specific:
 --ram16                 Attach a 16K RAM instead of the default 1K RAM
 --rom8                  Attach the 8K ROM instead of the default 4K ROM
```

#### Keyboard

Only the [positional](https://www.homecomputermuseum.nl/wp-content/uploads/2020/09/ZX80-1200x896.jpg)
keyboard layout is available for the ZX80 and it is used as default.

