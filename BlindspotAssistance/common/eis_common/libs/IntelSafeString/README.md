# Intel SafeString library

This library includes routines for safe string operations (like strcpy) and memory routines (like memcpy) that are recommended for Linux/Android operating systems, and will also work for Windows. This library is especially useful for cross-platform situations where one library for these routines is preferred.

## Compilation

```sh
$ mkdir build
$ cd build
$ cmake ..
$ make
```

## Installation

```sh
$ sudo make install
```

By default, this command will install the Intel safestring library into
`/usr/local/lib/`. On some platforms this is not included in the `LD_LIBRARY_PATH`
by default. As a result, you must add this directory to you `LD_LIBRARY_PATH`.
This can be accomplished with the following `export`:

```sh
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/
```

> **NOTE:** You can also specify a different library prefix to CMake through
> the `CMAKE_INSTALL_PREFIX` flag.