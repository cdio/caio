# caio - 8 bits home computers emulator

## compile, install and packaging

### linux

Required dependencies:

* C++20 compiler: [clang-10.0](https://clang.llvm.org) or later or
  [g++-9.3](https://en.wikipedia.org/wiki/GNU_Compiler_Collection) or later.

* [SFML-2.5.1](https://www.sfml-dev.org) or later.

* [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config).

```
    $ sudo apt install clang
    ...
    $ sudo apt install libsfml-dev
    ...
    $ sudo apt install pkg-config
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

### macos

Required dependencies:

* [xcode](https://developer.apple.com/xcode).

* xcode command line tools.

* [brew](https://brew.sh).

* [SFML-2.5.1](https://www.sfml-dev.org) or later.

* [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config).

To install xcode and brew follow the instructions
[here](https://developer.apple.com/xcode) and [here](https://brew.sh).

```
   $ xcode-select --install
   ...
   $ brew install sfml
   ...
   $ brew install pkg-config
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

### packaging

#### source code package:

```
    $ make src-package RELEASE=<tag>
```

The previous command creates a `.tgz` tarball source package containing the
specified release. The tarball file is placed inside the `build/` directory.
`tag` specifies the caio's git tag to build.

#### debian binary package:

```
    $ make deb-package RELEASE=<tag>
```

The previous command creates a `.deb` binary package under the `build/`
directory. `tag` specifies the caio's git tag to build.

#### generic binary package:

```
    $ make bin-package RELEASE=<tag>
```

The previous command creates a `.tgz` tarball binary package under the
`build/` directory. `tag` specifies the caio's git branch or tag to build.

Note that this package does not contain any runtime dependencies
(libsfml, libstdc++, etc). Those must be installed separately.
