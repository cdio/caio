# caio - 8-bit home computers emulator

## Compile & Install

### Linux

External dependencies:
* [cmake](https://cmake.org/)
* [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config)
* [freetype](https://www.freetype.org/)
* [SDL2](https://libsdl.org)

```
sudo apt update && sudo apt install cmake pkg-config libfreetype-dev libsdl2-dev
```

Compile and install:
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
* [cmake](https://cmake.org/)
* [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config)
* [freetype](https://www.freetype.org/)
* [SDL2](https://libsdl.org)

To install xcode, xcode command line tools, and brew follow the instructions
[here](https://developer.apple.com/xcode) and [here](https://brew.sh), then:

```
xcode-select --install
```
```
brew update && brew upgrade && brew install cmake pkg-config freetype sdl2
```

Compile and install:

```
make PREFIX=/usr/local
```
```
sudo make PREFIX=/usr/local install
```

The default `PREFIX` is `/opt/caio`.


## Binary Packages

### Linux

Install the external dependencies specified in the previous section and
run the following command to generate a debian package:

```
make package
```

The previous command could ask for the root password during the generation
of the actual package.

### macOS

Install the external dependencies specified in the previous section and
run the following command to generate dmg and pkg packages:

```
make package
```

Note that the generated packages are not signed.

