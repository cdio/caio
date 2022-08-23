# caio - 8 bits home computers emulator

## Compile, install and packaging

### Linux

Required dependencies:

* C++20 compiler: [clang-10.0](https://clang.llvm.org) or later or
  [g++-9.3](https://en.wikipedia.org/wiki/GNU_Compiler_Collection) or later.

* [SDL2](https://libsdl.org).

* [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config).

```
    $ sudo apt install clang pkg-config libsdl2-dev libsdl2-image-dev
    ...
```

```
    $ make PREFIX=/usr/local
    ...
```

The default compiler is [clang++](https://clang.llvm.org), it can be changed
to [g++](https://en.wikipedia.org/wiki/GNU_Compiler_Collection) by setting
the `CXX` environment variable:

```
    $ make CXX=g++ PREFIX=/usr/local
    ...
```

```
    $ sudo make PREFIX=/usr/local install
    ...
```

The default `PREFIX` is `/opt/caio`.

### MacOS

Required dependencies:

* [xcode](https://developer.apple.com/xcode).

* xcode command line tools.

* [brew](https://brew.sh).

* [SDL2](https://libsdl.org).

* [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config).

To install xcode and brew follow the instructions
[here](https://developer.apple.com/xcode) and [here](https://brew.sh).

```
   $ xcode-select --install
   ...
   $ brew install pkg-config sdl2 sdl2_image
   ...
```

```
    $ make PREFIX=/usr/local
    ...
```

```
    $ sudo make PREFIX=/usr/local install
    ...
```

The default `PREFIX` is `/opt/caio`.

### Packaging

#### Source code package:

```
    $ make src-package RELEASE=<tag>
```

The previous command creates a `.tgz` tarball source package containing the
specified release. The tarball file is placed inside the `build/` directory.
`tag` specifies the caio's git tag to build.

#### Debian binary package:

```
    $ make deb-package RELEASE=<tag>
```

The previous command creates a `.deb` binary package under the `build/`
directory. `tag` specifies the caio's git tag to build.

#### Generic binary package:

```
    $ make bin-package RELEASE=<tag>
```

The previous command creates a `.tgz` tarball binary package under the
`build/` directory. `tag` specifies the caio's git branch or tag to build.

Note that this package does not contain any runtime dependencies
(libSDL2, libstdc++, etc). Those must be installed separately.

