# EIS Message Envelope

EIS Message Envelope is a C library providing APIs to build the message envelope
structure that will be put onto the EIS Message Bus. This library will be used
by EIS Utils and EIS Message Bus libraries.

## Compilation

The EIS Message Envelope utilizes CMake as the build tool for compiling the C
library. The simplest sequence of commands for building the library are
shown below.

```sh
$ mkdir build
$ cd build
$ cmake ..
$ make
```

If you wish to compile the EIS Message Envelope in debug mode, then you can set
the `CMAKE_BUILD_TYPE` to `Debug` when executing the `cmake` command (as shown
below).

```sh
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
```

## Installation

> **NOTE:** This is a mandatory step to use this library in EIS Utils and
> EIS Message Bus libraries

If you wish to install the EIS Message Envelope on your system, execute the
following command after building the library:

```sh
$ sudo make install
```

By default, this command will install the EIS Message Envelope C library into
`/usr/local/lib/`. On some platforms this is not included in the `LD_LIBRARY_PATH`
by default. As a result, you must add this directory to you `LD_LIBRARY_PATH`,
otherwise you will encounter issues using the EIS Message Bus. This can
be accomplished with the following `export`:

```sh
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/
```
> **NOTE:** You can also specify a different library prefix to CMake through
> the `CMAKE_INSTALL_PREFIX` flag.

## Running Unit Tests

> **NOTE:** The unit tests will only be compiled if the `WITH_TESTS=ON` option
> is specified when running CMake.

Run the following commands from the `build/tests` folder to cover the unit
tests.

```sh
$ ./msg-envelope-tests
$ ./crc32-tests
```
