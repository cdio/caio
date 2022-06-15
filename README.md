# cemu - 8 bits home computers emulator

cemu is an emulator of 8 bit home computers.

cemu is currently **work-in-progress** and at the moment only the Commodore 64
is emulated, under Linux.

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
By default the installation directory is */opt/cemu*.

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
For more information about usage, keyboard, joysticks, etc read the
[usage documentation](doc/usage.md).


## status

Although cemu is usable, it is not finished and it is not free of bugs.
the chipset emulation is not 100% complete neither accurate.

For more information about some of the internal characteristics of the
emulator read the [internals documentation](doc/sid.md).


## todo

In no particular order:

* Finish Commodore 64
* Apple-2
* BBC Micro
* Daewoo DPC-200
* NEC PC-8001
* Amstrad CPC-464
* ZX Spectrum

