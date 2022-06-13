# cemu - 8 bits home computers emulator

## compile, install and packaging

### dependencies:
* C++20 compiler: [clang-10.0](https://clang.llvm.org) or later or
  [g++-9.3](https://en.wikipedia.org/wiki/GNU_Compiler_Collection) or later.

* [SFML-2.5.1](https://www.sfml-dev.org) or later.

* [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config)


```
    $ sudo apt install clang
    ...
    $ sudo apt install libsfml-dev
    ...
    $ sudo apt install pkg-config
    ...
```

### compile:

```
    $ make PREFIX=/usr/local
    ...
```

The default compiler is [clang++](https://clang.llvm.org), it can be changed
to [g++](https://en.wikipedia.org/wiki/GNU_Compiler_Collection) by setting the
*CXX* environment variable:

```
    $ make CXX=g++ PREFIX=/usr/local
    ...
```

### install:

```
    $ sudo make PREFIX=/usr/local install
    ...
```

The default *PREFIX* is */opt/cemu*.

### packaging:

#### source code package:

```
    $ make src-package RELEASE=<tag>
```

That creates a *.tgz* tarball file under the *build/* directory.

*tag* specifies the CEMU's git branch or tag to pack.

#### debian binary package:

```
    $ make deb-package RELEASE=<tag>
```

That creates a *.deb* binary file under the *build/* directory.

*tag* specifies the CEMU's git branch or tag to build.

#### generic binary package:

```
    $ make bin-package RELEASE=<tag>
```

That creates a *.tgz* (tarball) binary file under the *build/* directory.

*tag* specifies the cemu's git branch or tag to build.

Note that this tarball file does not contain any runtime dependencies
(libsfml, libstdc++, etc).
Those must be installed separately.

