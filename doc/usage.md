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
<details>
<summary>Generic configuration</summary>

### Generic configuration

Configuration values can be specified as command line options or as values in a
[configuration file](../src/main/caio.conf).
The configuration file contains two types of sections: One ***generic***
section whose values are inherited by all platforms, and one ***specific***
section for each emulated platform.

If a parameter is specified twice (as a platform specific and generic), the
platform specific value takes the precedence.

The configuration file is searched in the following directories (stop at
first match):

1. Command line option               `--conf`
2. User's configuration directory:   `$HOME/.config/caio/caio.conf`
3. System's configuration directory: `$PREFIX/etc/caio/caio.conf`

Command line options take precedence over those defined in the
configuration file.

The following list shows the generic command line options:

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
 --vjoy <yes|no>         Enable virtual joystick (default is no)
 --vjoy-up <keyname>     Virtual joystick UP key (default is KEY_NUMPAD_8)
 --vjoy-down <keyname>   Virtual joystick DOWN key (default is KEY_NUMPAD_2)
 --vjoy-left <keyname>   Virtual joystick LEFT key (default is KEY_NUMPAD_4)
 --vjoy-right <keyname>  Virtual joystick RIGHT key (default is KEY_NUMPAD_6)
 --vjoy-fire <keyname>   Virtual joystick FIRE key (default is KEY_NUMPAD_5)
 -v|--version            Show version information and exit
 -h|--help               Print this message and exit
```

Platforms are not required to support all these options, unsupported options
are ignored.

### Hot-Keys

caio accepts the following key combinations at runtime:

* `ALT-F` toggles between *windowed* and *fullscreen* modes.

* `PAUSE` or `ALT-P` toggles between *pause* and *running* modes.

* `ALT-J` swaps joysticks #1 and #2.

* `ALT-K` toggles the status of the keyboard (active/inactive).

* `ALT-M` enters the CPU monitor (if it is active). Like `CTRL-C`
  on the terminal.

* `CTRL-C` on the terminal enters the CPU monitor (if the monitor is not
  active the emulation is terminated).

* `ALT-V` toggles the visibility of the info panel.

MAC users must replace the `ALT` key with the command key &#8984;.

### Info Panel

The Info Panel is a basic control panel containing widgets that allow
minimal settings during runtime. It can be made visible/invisible with a
mouse right click or through the `ALT-V` key combination.
The default panel widgets are:

* Toggle Fullscreen mode
* Platform reset
* Suspend/Resume emulation
* Audio Volume control

There are other widgets that depend on the specifc platform such as:

* Joystick status
* Disk drives status

</details>
<details>
<summary>Commodore 64</summary>

### Commodore 64

```
$ caio c64 --help
usage: caio c64 <options>
where <options> are:
 ...
Commodore C64 specific:
 --prg <prg>             Load a PRG file as soon as the basic is ready
 --resid <yes|no>        Use the MOS6581 reSID library (default is no)
 --swapj                 Swap Joysticks
 --8 <path>              Attach a disk drive as unit 8
 --9 <path>              Attach a disk drive as unit 9
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

* Gamepads or real joysticks:

  Up to two gamepads should work (tested using only one Logitech F710).
  The PS3 controller is known to work.

* Virtual Joystick:

  A virtual joystick is available and it can be enabled using the `vjoy`
  configuration option.
  The default mappings are:
    - UP: `KEY_NUMPAD_8`
    - DOWN: `KEY_NUMPAD_2`
    - LEFT: `KEY_NUMPAD_4`
    - RIGHT: `KEY_NUMPAD_3`
    - FIRE: `KEY_NUMPAD_5`

  These mappings can be changed using the following configuration options:
    - `vjoy_up`
    - `vjoy_down`
    - `vjoy_left`
    - `vjoy_right`
    - `vjoy_fire`

  Unexpected results can occur when the same key is shared by both the
  emulated keyboard and the virtual joystick (both will process the keystroke).
  To help deal with this situation the state of the emulated keyboard can be
  toggled (activated/deactivated) at runtime using the `ALT-K` key combination.

  Games or other applications that use the keyboard in conjunction with the
  joystick should never share the same keys. If the default joystick keys are
  not available or just difficult to use, the user is always free to redefine
  both the keyboard keys and the virtual joystick ones as desired.

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

The program is injected directly into RAM with the emulator suspended,
this means that the previous command won't work for advanced or big files
that are expected to overwrite memory areas not configured as RAM.

</details>
<details>
<summary>Sinclair ZX80</summary>

### Sinclair ZX80
```
$ caio zx80 --help
usage: caio z80 <options>
where <options> are:
 ...
Sinclair ZX80 specific:
 --ram16                 Attach a 16K RAM instead of the default 1K RAM
 --rom8                  Attach the 8K ROM instead of the default 4K ROM
 --prg <.o|.p>           Load a .o/.p file as soon as the basic is started
```

#### Keyboard

The default keyboard layout depends on the installed ROM:

##### 4K ROM keyboard layout:

<img src="../images/zx80-4K-layout.jpg" width="640">

##### 8K ROM keyboard layout:

<img src="../images/zx80-8K-layout.jpg" width="640">

#### Software

The following site has not only good information about the internals
of the machine but it is also the main site of really beautiful games
that exploit the flicker-free technique:
[ZX Resource Centre](http://www.fruitcake.plus.com/Sinclair/ZX80/FlickerFree/ZX80_DisplayMechanism.htm "ZX Resource Centre")

</details>
