# caio - 8 bits home computers emulator

caio is an emulator of hardware platforms, with a strong focus on old 8 bits
home computers.

The core of the emulator is a library that provides generic methods (signal
processing, keyboard translators, joystick support, GUIs, etc.) and a set of
*devices* that implement the behaviour of hardware devices (RAMs, ROMs,
micro-processors, video controllers, audio controllers, etc.).
These elements are then combined together to emulate different hardware
platforms.


## goal

The goal of caio is to provide a single architecture to emulate several
different hardware platforms. To do that the chips used by those platforms
are emulated.
Sometimes, the emulation of some of them cannot be 100% accurate and usually
the problem is not the implementation of a specific device itslef but its
internal bugs.

In some cases bugs are hidden, making the behaviour of the device quite
divergent from those described in technical specifications and difficult to
reproduce, sometimes bugs are known but the documentation explaining them are
inaccurate or difficult to find, and in other cases bugs were discovered over
the years and exploited as features, infamous examples of these are the
[MOS 6581](https://en.wikipedia.org/wiki/MOS_Technology_6581) audio chip and
the [MOS 6569](https://en.wikipedia.org/wiki/MOS_Technology_VIC-II) video
controller both found on the
[Commodore 64](https://en.wikipedia.org/wiki/Commodore_64).

The efforts required to implement bugs can be enormous, making the development
of a device a time consuming nightmare.
For this reason, when dealing with buggy devices, an implementation is done as
much accurate as possible, given the availability of time, documentation,
and several doses of passion. If the final result is *not good enough*
and third party solutions are available then those solutions are also used.
An example of this is the emulator of the
[MOS 6581](https://en.wikipedia.org/wiki/MOS_Technology_6581) chip, where
two different implementations are provided, the internal one which reached an
(arguably) decent level of accuracy and another one that uses the
[reSID](https://en.wikipedia.org/wiki/ReSID) library which emulates the device
very accurately thanks to a big deal of reverse engineering work done by its
author.

For more information about caio and its internal structure, read the source
code or the [documentation done during development](doc/sid.md).


## status

caio is currently in alpha state, it is **work-in-progress** and at the
moment only the [Commodore 64](https://en.wikipedia.org/wiki/Commodore_64)
is emulated, under Linux. See the [releases](#releases) section below.

That said, lots of programs for the
[Commodore 64](https://en.wikipedia.org/wiki/Commodore_64) work just fine:

![c64](images/c64.gif "caio c64")
![gyruss](images/gyruss.gif "Gyruss")
![q-bert](images/q-bert.gif "Q*Bert")
![uridium](images/uridium.gif "Uridium")
![giana](images/giana.gif "Great Giana Sisters")
[![zauberwald](images/zauberwald.png "Zauberwald")](https://csdb.dk/release/?id=188005)


## compile, install and packaging

```
    $ sudo apt install clang
    ...
    $ sudo apt install libsfml-dev
    ...
    $ sudo apt install pkg-config
    ...
    $ make all
    ...
    $ sudo make install
```

The default installation directory is `/opt/caio`, it can be changed by
setting the `PREFIX` make variable.

For more information read the [compile, install and packaging](doc/compile.md)
documentation.


## usage & configuration

There are three ways to launch the emulator, two of them from the console:

```
    $ caio c64 --help
```
or

```
    $ c64 --help
```

All the command line options can be specified in a
[configuration file](src/main/caio.conf).

Examples:

The following command activates the horizontal scanlines visual effect, scales
the emulated screen resolution 4 times (that is, a 320x200 screen is scaled to
1280x800), loads and launches the cartridge named *gyruss*:

```
    $ c64 --scanlines h --scale 4 --cart ./gyruss.crt
```

The next command injects a program (`.prg`) into memory and launches it as soon
as the basic is started:

```
    $ c64 --prg ./rambo.prg
```

Note that this won't work for advanced or big program files that overwrite
memory areas not configured as RAM.

As stated before, all command line parameters can be specified in the
configuration file, if that is the case the emulator can be launched from a
GUI (like a file manager) but care must be taken not to send the logs to the
current terminal (because there won't be one), if that happens the emulator will
silently terminate as soon as it is started (see the `logfile` configuration
file option).

For more information about usage, keyboard mappings, joysticks, etc., read the
[usage documentation](doc/usage.md).


## releases

The plan is to release a beta version containing the following emulators:

* [Commodore 64](https://en.wikipedia.org/wiki/Commodore_64)
* [Sinclair ZX80](https://en.wikipedia.org/wiki/ZX80)

There is no date for this release.

It is desired for future releases to provide emulation for:

* [Amstrad CPC-464](https://en.wikipedia.org/wiki/Amstrad_CPC_464)
* [Apple-2](https://en.wikipedia.org/wiki/Apple_II)
* [BBC Micro](https://en.wikipedia.org/wiki/BBC_Micro)
* [Daewoo DPC-200](https://www.msx.org/wiki/Daewoo_DPC-200)
* [NEC PC-8001](https://en.wikipedia.org/wiki/PC-8000_series#PC-8001)
* [Sinclair ZX Spectrum](https://en.wikipedia.org/wiki/ZX_Spectrum)


## todo

In no particular order:

* Finish Commodore 64
* Apple-2
* BBC Micro
* Daewoo DPC-200
* NEC PC-8001
* Amstrad CPC-464
* ZX Spectrum
* ZX80
