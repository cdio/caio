# caio - 8-bit home computers emulator

## Compile & Install

### Linux

External dependencies:
* [clang-15.0](https://clang.llvm.org) or later
* [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config)
* [SDL2](https://libsdl.org)

```
    sudo apt install clang pkg-config libsdl2-dev libsdl2-image-dev
```
```
    make PREFIX=/usr/local
```
```
    sudo make PREFIX=/usr/local install
```
The default `PREFIX` is `/opt/caio`.

### macOS

External dependencies:
* [xcode](https://developer.apple.com/xcode)
* xcode command line tools
* [brew](https://brew.sh)
* [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config)
* [SDL2](https://libsdl.org)

To install xcode, xcode command line tools, and brew follow the instructions
[here](https://developer.apple.com/xcode) and [here](https://brew.sh).

```
   xcode-select --install
```
```
   brew install pkg-config sdl2 sdl2_image
```
Once these packages are installed and updated:

```
    make PREFIX=/usr/local
```
```
    sudo make PREFIX=/usr/local install
```
The default `PREFIX` is `/opt/caio`.

