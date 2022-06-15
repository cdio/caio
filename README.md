# cemu - 8 bits home computers emulator

cemu provides an architecture used to implement hardware emulators,
with a strong focus on old home computers.

The core of the architecture is a library that provides generic methods
(signal processing, GUIs, address space decoders, keyboard translators,
joystick support, etc.) and a set of devices that implement the behaviour
of several hardware chips (RAMs, ROMs, micro-processors, video controllers,
audio controllers, etc.).
The devices provided by this library are combined together in order to
create emulators of different hardware platforms.


## accuracy of the emulation

It is not the goal of this project to emulate all the hardware devices
with 100% of accuracy. At least not at this stage.

This is so because the most difficult part of implementing an emulator
of a specific hardware device is not the device itself but the bugs
contained in it.
In some cases bugs are hidden making the behaviour of the device very
difficult to reproduce, sometimes bugs are known but the documentation
explaining them are inaccurate or difficult to find, and in other cases,
bugs were discovered over the years and exploited as features, infamous
examples are the
[MOS 6581](https://en.wikipedia.org/wiki/MOS_Technology_6581) audio chip
and the [MOS 6569](https://en.wikipedia.org/wiki/MOS_Technology_VIC-II)
video controller both found on the
[Commodore 64](https://en.wikipedia.org/wiki/Commodore_64).

Sometimes, implementing bugs can be a real nightmare.

That said, cemu tries to emulate the hardware devices as much accurately
as possible, given the available time, documentation, and eventual
injections of passion.
When that is not possible, third party solutions are used. An example of
this is the [MOS 6581](https://en.wikipedia.org/wiki/MOS_Technology_6581)
chip, cemu provides two different implementations of this device, the
internal one and another that uses the
[reSID](https://en.wikipedia.org/wiki/ReSID) library which emulates that
chip with a great deal of accuracy.

For more information about cemu read the source code or the documentation
about its [internals](doc/sid.md).


## status

cemu is currently **work-in-progress** and at the moment only the
[Commodore 64](https://en.wikipedia.org/wiki/Commodore_64) is emulated,
under Linux.

The first (beta) release of cemu will come with the following emulators:

* [Commodore 64](https://en.wikipedia.org/wiki/Commodore_64)
* [Sinclair ZX80](https://en.wikipedia.org/wiki/ZX80)

There is no date for this release.

It is desired for future releases to provide emulators for:

* [Amstrad CPC-464](https://en.wikipedia.org/wiki/Amstrad_CPC_464)
* [Apple-2](https://en.wikipedia.org/wiki/Apple_II)
* [BBC Micro](https://en.wikipedia.org/wiki/BBC_Micro)
* [Daewoo DPC-200](https://www.msx.org/wiki/Daewoo_DPC-200)
* [NEC PC-8001](https://en.wikipedia.org/wiki/PC-8000_series#PC-8001)
* [Sinclair ZX Spectrum](https://en.wikipedia.org/wiki/ZX_Spectrum)

The following pictures show some programs running on the current
implementation of the
[Commodore 64](https://en.wikipedia.org/wiki/Commodore_64):

![c64](/images/c64.gif "CEMU C64")
![gyruss](/images/gyruss.gif "Gyruss")
![q-bert](/images/q-bert.gif "Q*Bert")
![uridium](/images/uridium.gif "Uridium")
![giana](/images/giana.gif "Great Giana Sisters")
[![zauberwald](/images/zauberwald.png "Zauberwald")](https://csdb.dk/release/?id=188005)


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
By default the installation directory is */opt/cemu*,
it can be changed by setting the PREFIX make variable.

For more information read the [install documentation](doc/compile.md).


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

For example, to attach a cartridge (*.crt*) image:

```
    $ c64 --scanlines h --scale 4 --cart ./gyruss.crt
```
To launch a program (*.prg*) file as soon as the basic is started:

```
    $ c64 --prg ./rambo.prg
```
For more information about usage, keyboard, joysticks, etc., read the
[usage documentation](doc/usage.md).


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

