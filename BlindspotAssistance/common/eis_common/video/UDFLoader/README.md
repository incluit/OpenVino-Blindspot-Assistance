# EIS UDFLoader

UDFLoader is a library providing APIs for loading and executing native and python UDFs.

## Dependency Installation

UDFLoader depends on the below libraries. Follow their documentation to install them.
* OpenCV - Run `source /opt/intel/openvino/bin/setupvars.sh` command
* [EISMsgEnv](../EISMsgEnv/README.md)
* [EISUtils](../../util/c/README.md)
* [IntelSafeString](../IntelSafeString/README.md)
* Python3 Numpy package

## Compilation

Utilizes CMake as the build tool for compiling the library. The simplest sequence of commands for building the library are
shown below.

```sh
$ mkdir build
$ cd build
$ cmake ..
$ make
```

If you wish to compile in debug mode, then you can set
the `CMAKE_BUILD_TYPE` to `Debug` when executing the `cmake` command (as shown
below).

```sh
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
```

## Installation

> **NOTE:** This is a mandatory step to use this library in
> C/C++ EIS modules

If you wish to install this library on your system, execute the
following command after building the library:

```sh
$ sudo make install
```

By default, this command will install the `udfloader` library into
`/usr/local/lib/`. On some platforms this is not included in the `LD_LIBRARY_PATH`
by default. As a result, you must add this directory to you `LD_LIBRARY_PATH`. This can
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
# First, source the source.sh file to setup the PYTHONPATH environment
$ source ./source.sh

# Execute frame abstraction unit tests
$ ./frame-tests

# Execute UDF loader unit tests
$ ./udfloader-tests
```
