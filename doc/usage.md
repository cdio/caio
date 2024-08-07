# caio - 8-bit home computers emulator

## Usage & Configuration

At the moment, the best way to launch caio is from the command line terminal:
```
$ caio --help
usage: caio <arch> [--help]
where arch is one of:
c64
zx80
zxspectrum
```

<details>
<summary>Generic Configuration</summary>

### Generic Configuration

Configuration parameters can be specified as command line options or as
*key-value* pairs in a [configuration file](../src/main/caio.conf).<br>
The configuration file contains two types of sections: One ***generic***
section whose values are inherited by all platforms, and one ***specific***
section for each emulated platform.<br>
If a parameter is specified twice (as platform specific and generic), the
platform specific value takes the precedence.

The configuration file is searched in the following places (stop at first
match):

1. Command line option               `--conf`
2. User's configuration directory:   `$HOME/.config/caio/caio.conf`
3. System's configuration directory: `$PREFIX/etc/caio/caio.conf`

Command line options take precedence over those defined in the configuration
file.

Generic options as command line parameters:
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
 --fullscreen [yes|no]   Start in fullscreen mode
 --sresize [yes|no]      Smooth window resize (default is yes)
 --audio [yes|no]        Enable audio (default is yes)
 --delay <delay>         Clock delay factor (default is 1)
 --monitor [yes|no]      Activate the CPU monitor (default is no)
 --logfile <file>        Send log information to the specified destination
                         (default is /dev/tty)
 --loglevel <lv>         Loglevel, bitwise combination of:
                         none|error|warn|info|debug|all (default is none)
 --vjoy [yes|no]         Enable virtual joystick (default is no)
 --vjoy-up <keyname>     Virtual joystick UP key (default is KEY_NUMPAD_8)
 --vjoy-down <keyname>   Virtual joystick DOWN key (default is KEY_NUMPAD_2)
 --vjoy-left <keyname>   Virtual joystick LEFT key (default is KEY_NUMPAD_4)
 --vjoy-right <keyname>  Virtual joystick RIGHT key (default is KEY_NUMPAD_6)
 --vjoy-fire <keyname>   Virtual joystick FIRE key (default is KEY_NUMPAD_5)
 -v|--version            Show version information and exit
 -h|--help               Print this message and exit
```

Emulated platforms are not required to support all these options, unsupported
parameters are ignored.

### Hot-Keys

The following key combinations are accepted at runtime:

* `ALT-F` toggles between *windowed* and *fullscreen* modes.
* `PAUSE` or `ALT-P` toggles between *pause* and *running* modes.
* `ALT-J` swaps joysticks #1 and #2.
* `ALT-K` toggles the status of the keyboard (active/inactive).
* `ALT-M` enters the CPU monitor (if it is active). Like `CTRL-C`
  on the terminal.
* `CTRL-C` on the terminal enters the CPU monitor (if the monitor is not
  active the emulation is terminated).
* `ALT-V` toggles the visibility of the info panel.

Under macOS, the `ALT` key is equivalent of the option key &#8997;.

### Info Panel

The Info Panel is a basic control panel containing widgets that allow minimal
settings at runtime. It can be made visible/invisible with a mouse right
click or using the `ALT-V` key combination.<br>
The default panel widgets are:

* Toggle Fullscreen mode
* Platform reset
* Suspend/Resume emulation
* Audio Volume control

There are other widgets that depend on the specifc platform, such as:

* Joystick status
* Disk drive status
* Cassette status

### Joysticks

* Gamepads or real joysticks:
  When a real gamepad is detected and the emulated platform supports a
  joystick it is attached to it.

* Virtual Joystick:
  A virtual joystick is available and it can be enabled using the `vjoy`
  configuration option.
  The default mappings are:
    - UP: `KEY_NUMPAD_8`
    - DOWN: `KEY_NUMPAD_2`
    - LEFT: `KEY_NUMPAD_4`
    - RIGHT: `KEY_NUMPAD_3`
    - FIRE: `KEY_NUMPAD_5`

  These mappings can be changed using the following configuration parameters:
    - `vjoy_up`
    - `vjoy_down`
    - `vjoy_left`
    - `vjoy_right`
    - `vjoy_fire`

<hr>
</details>
<details>
<summary>Commodore 64</summary>

### Commodore 64

```
$ caio c64 --help
usage: c64 <options> [<file>]
where <file> is the name of a program, cartridge or
snapshot to launch (the file format is auto-detected)
and <options> are:
 ...
Commodore C64 specific:
 --prg <prg>             Load a PRG file as soon as the basic is ready
 --swapj [yes|no]        Swap Joysticks (default is no)
 --unit8 <dir>           Attach a disk drive as unit 8
 --unit9 <dir>           Attach a disk drive as unit 9
```

#### Keyboard

The keyboard layout can be set using the `keymaps` configuration option
(the default is [US-ANSI](https://en.wikipedia.org/wiki/File:ANSI_Keyboard_Layout_Diagram_with_Form_Factor.svg)).
<br>
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

Like the [VICE](https://en.wikipedia.org/wiki/VICE) emulator, the `RESTORE`
key is mapped as `Page-Up`, `RUN/STOP` as `ESC`, `CTRL` as `TAB` and
`CBM` as `LEFT-CTRL`.

#### Joysticks

The Commodore 64 has two joystick ports. If a gamepad is detected it is
attached to one of these ports. If the virtual joystick is also enabled
one port is associated to the gamepad and the other to the virtual
joystick.

Unexpected results could occur when the same key is shared by both the
emulated keyboard and the virtual joystick (both try to process the
keystroke). To help deal with this situation the state of the emulated
keyboard can be toggled (activated/deactivated) at runtime using the
`ALT-K` key combination.<br>
Games or other applications that use the keyboard in conjunction with the
joystick should never share the same keys. If the default joystick keys are
not available or just difficult to use, the user is always free to redefine
both the keyboard keys and the virtual joystick as desired.

For more information refer to the Generic Configuration section.

#### Disk drives

There is an implementation of the
[C1541](https://en.wikipedia.org/wiki/Commodore_1541) disk drive unit that
access the host filesystem. Host directories are recursively traversed so it
must be used with care.<br>
Configuration options `unit8` and `unit9` must be used to associate a host
directory to a floppy disk.

`D64` disk drive images are not supported yet.

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
    $ caio c64 --scanlines H --scale 2 --cart /apps/c64/simons_basic.crt
```

The next command loads and run a `PRG` program:
```
    $ caio c64 --scale 3 --prg /games/c64/rambo.prg
```

The program is injected directly into RAM while the emulator is suspended,
this means that the previous command won't work for advanced or big files
that are expected to overwrite memory areas not configured as RAM. In that
case configuration options `unit8` and `unit9` must be used as follows:
```
    $ caio c64 --scale 3 --unit8 /games/c64
```
then, from basic:
```
LOAD "RAMBO",8,1
```

Cartridges and programs can be specified without using their specific
command line options, in this case the format is auto-detected:
```
    $ caio c64 --scale 3 /games/c64/ghostbusters.crt
```

<hr>
</details>
<details>
<summary>Sinclair ZX-80</summary>

### Sinclair ZX-80

```
$ caio zx80 --help
usage: zx80 <options> [<file>]
where <file> is the name of a program, cartridge or
snapshot to launch (the file format is auto-detected)
and <options> are:
 ...
Sinclair ZX80 specific:
 --ram16 [yes|no]        Attach a 16K RAM instead of the default 1K RAM
 --rom8 [yes|no]         Attach the 8K ROM instead of the default 4K ROM
 --rvideo [yes|no]       Reverse video output
 --cassdir <dir>         Set the basic save/load directory (default is ./)
 --prg <.o|.p>           Load a .o/.p file as soon as the basic is started
```

The original Sinclair ZX-80 came with 4K ROM and 1K RAM. Several RAM upgrades
were available at the time and caio supports the 16K RAM upgrade.
An 8K ROM was also available for the ZX-80 and it is supported.

The cassette interface is associated to a directory in the host filesystem,
files contained there are considered part of a single tape (see the Cassette
interface section below).

#### Keyboard

The default keyboard layout depends on the installed ROM:

##### 4K ROM keyboard layout:

<img src="../images/zx80-4K-layout.jpg" width="430">

##### 8K ROM keyboard layout:

<img src="../images/zx80-8K-layout.jpg" width="430">

#### Cassette interface

The cassette interface is emulated and both cassette file formats
***.o*** (4K ROM) and ***.p*** (8K ROM) are supported. Audio files
(WAV, PCM, etc.) are not supported.
<br>
The behaviour of basic commands `LOAD` and `SAVE` depend on the ROM being
used:
- 4K ROM: The filename is *ALWAYS* set to `basic.o` and it is located under
  the cassette directory. Because of this, a `SAVE` operation overwrites an
  existing `basic.o` file.
- 8K ROM: In the case of a `SAVE` operation, a file is created in the cassette
  directory using the name specified by the user.
  In the case of a `LOAD` operation, the content of all ***.p*** files present
  inside the cassette directory are concatenated into a continuous sequence of
  bytes emulating a real tape.

See the `cassdir` configuration option.

The `prg` configuration option can be used to automatically load a program as
soon the basic is started by-passing the slow cassette interface. For example:
```
    $ caio zx80 --scale 2 --ram16 --prg /games/zx80/ZX80.4K.ROM.Pacman.o
```

Programs can be specified without using the `--prg` command line option,
in this case the format is auto-detected:
```
    $ caio zx80 --scale 2 --ram16 --rom8 yes /games/zx80/ZX80.8K.ROM.Kong.o
```

#### Software

The following site has not only good information about the internals
of the machine but it is also the main site of really beautiful games
that exploit the flicker-free technique:
[ZX Resource Centre](http://www.fruitcake.plus.com/Sinclair/ZX80/FlickerFree/ZX80_DisplayMechanism.htm "ZX Resource Centre").

<hr>
</details>
<details>
<summary>Sinclair ZX-Spectrum 48K</summary>

### Sinclair ZX-Spectrum 48K

```
$ caio zxspectrum --help
usage: zxspectrum <options> [<file>]
where <file> is the name of a program, cartridge or
snapshot to launch (the file format is auto-detected)
and <options> are:
...
Sinclair ZX-Spectrum 48K specific:
 --tape <fname|dir>      Set the input tape file (TAP) or directory
 --otape <fname|dir>     Set the output tape file (TAP) or directory
                         (default is ./)
 --fastload [yes|no]     Fast tape loading (default is no)
 --snap <fname>          Load a snapshot image (Z80 or SNA formats)
```

#### Keyboard

The default is the positional keyboard layout:

<a href="https://upload.wikimedia.org/wikipedia/commons/3/33/ZXSpectrum48k.jpg">
<img src="../images/640px-ZXSpectrum48k.jpg" width=430
title="ZX-Spectrum - Photo By Bill Bertram - Own work, CC BY-SA 2.5">
</a>

The `CAPS SHIFT` key is mapped as `SHIFT` and the `SYMBOL SHIFT` key
is mapped as `CTRL`.

#### Joysticks

The [Kempston](https://en.wikipedia.org/wiki/Kempston_Micro_Electronics)
joystick interface is supported.<br>
If the virtual joystick is enabled (`vjoy` configuration option), the
Kempston interface is automatically connected to it. If a gamepad is
detected and the virtual joystick is **not** enabled then the Kempston
interface is connected to it.

For more information refer to the Generic Configuration section.

#### Cassette interface

The cassette interface is emulated and it only supports the ***TAP***
file format. Audio files (WAV, PCM, etc.) are not supported.

The cassette interface can be specified as a file or as a directory
and there are two configuration options:

- `tape`: Sets the input tape (*LOAD*)<br>
If a directory is specified as input tape, the content of a number of *TAP*
files found inside it are concatenated into a big virtual *TAP* which
is then used as input tape.

- `otape`: Sets the output tape (*SAVE*)<br>
If a directory is specified as output tape, any SAVE operation creates
a new *TAP* file inside it.<br>
If a file is specified as output tape, any SAVE operation appends data
to it.

The following command specifies the file *starquak.tap* as input tape,
this file contains the game *Starquake*; the `fastload` option is used
to accelerate loading:
```
    $ caio zxspectrum --fastload --tape /games/spectrum/starquak.tap
```

In order to load and run the actual game the command shown below must be
entered from *BASIC*:
```
    LOAD ""
```

#### Snapshots

The `snap` option can be used to load a snapshot file.
Supported formats are ***SNA*** and ***Z80***.<br>

The following command launches the game called *Jet Set Willy* which is
embedded inside a snapshot:
```
    $ caio zxspectrum --snap /games/spectrum/Jet.Set.Willy.z80
```

Snapshots can be specified without using the `--snap` command line option,
in this case the snapshot format is auto-detected:
```
    $ caio zxspectrum /games/spectrum/elite.z80
```

<hr>
</details>
