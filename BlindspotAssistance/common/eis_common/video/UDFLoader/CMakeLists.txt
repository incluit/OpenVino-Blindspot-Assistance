# Copyright (c) 2019 Intel Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

cmake_minimum_required(VERSION 3.12)
project(eis-udf-loader VERSION 1.0.0 LANGUAGES C CXX)

# Define CMake options
option(WITH_EXAMPLES "Compile with examples" OFF)
option(WITH_TESTS    "Compile with unit tests" OFF)

# Globals
set(EIS_COMMON_CMAKE "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")

# Include directory for CMake modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${EIS_COMMON_CMAKE}")

# Set C++ version
set(CMAKE_CXX_STANDARD 14)

# Set C version
set(CMAKE_C_STANDARD 11)

# Set CFLAGS for C/C++
set(CMAKE_C_FLAGS "-fPIE -fPIC -O2 -Wall -pedantic -fstack-protector-strong -D_FORTIFY_SOURCE=2")
set(CMAKE_CXX_FLAGS "-fPIE -fPIC -O2 -Wall -pedantic -fstack-protector-strong -D_FORTIFY_SOURCE=2")

# Set LDFLAGS
set(CMAKE_SHARED_LINKER_FLAGS "-z noexecstack -z relro -z now")

# Finding packages
find_package(OpenCV REQUIRED)
find_package(EISMsgEnv REQUIRED)
find_package(EISUtils REQUIRED)
find_package(IntelSafeString REQUIRED)
find_package(Python3 COMPONENTS Development NumPy REQUIRED)

# Export compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json")
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
        ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
    )
endif()

# Include header directories
include_directories(
    include/
    ${OpenCV_INCLUDE_DIRS}
    ${EISMsgEnv_INCLUDE}
    ${EISUtils_INCLUDE}
    ${Python3_INCLUDE_DIRS}
    ${Python3_NumPy_INCLUDE_DIRS}
    ${IntelSafeString_INCLUDE})

# Generate Cython sources
find_program(CYTHON_EXECUTABLE "cythonize"
    DOC "Cython executable")

add_custom_target(cython-udf
    COMMENT "Generating Cython sources"
    COMMAND ${CYTHON_EXECUTABLE} -3
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cython/udf.pyx
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cython
    VERBATIM)
execute_process(
    COMMAND ${CYTHON_EXECUTABLE} udf.pyx
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/cython"
    RESULT_VARIABLE cython_result)

# Get all source files
file(GLOB SOURCES "src/*.cpp" "src/cython/udf.cpp")

# Add target
add_library(udfloader SHARED ${SOURCES})
add_dependencies(udfloader cython-udf)
target_link_libraries(udfloader
    PUBLIC
        ${EISMsgEnv_LIBRARIES}
        ${EISUtils_LIBRARIES}
        ${OpenCV_LIBS}
        pthread
    PRIVATE
        ${Python3_LIBRARIES}
        ${IntelSafeString_LIBRARIES})

# If compile in debug mode, set DEBUG flag for C code
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    target_compile_definitions(udfloader PRIVATE DEBUG=1)
endif()

# Add examples if the option was selected
if(WITH_EXAMPLES)
    add_subdirectory(examples/)
endif()

# Add unit tests if the option was selected
if(WITH_TESTS)
    add_subdirectory(tests/)
endif()

##
## Configure pkg-config file to be installed for the EIS Message Envelope lib
##

set(PKG_CONFIG_IN  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/libudfloader.pc.in")
set(PKG_CONFIG_OUT "${CMAKE_CURRENT_BINARY_DIR}/libudfloader.pc")
set(DEST_DIR       "${CMAKE_INSTALL_PREFIX}")
set(PRIVATE_LIBS   "-l")

configure_file(${PKG_CONFIG_IN} ${PKG_CONFIG_OUT} @ONLY)

##
## Add CMake configuration for installing the library including files for other
## projects finding the library using CMake
##

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/UDFLoader)

install(TARGETS udfloader
    EXPORT udfloader-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

set_target_properties(udfloader PROPERTIES EXPORT_NAME UDFLoader)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install pkg-config libudfloader.pc file
install(
    FILES
        ${PKG_CONFIG_OUT}
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# Export targets to a script
install(EXPORT udfloader-targets
    FILE
        UDFLoaderTargets.cmake
    DESTINATION
        ${INSTALL_CONFIGDIR}
)

# Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/UDFLoaderConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/UDFLoaderConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/UDFLoaderConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR})

# Install the config, configversion and custom find modules
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/UDFLoaderConfigVersion.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/UDFLoaderConfig.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

export(EXPORT udfloader-targets
       FILE ${CMAKE_CURRENT_BINARY_DIR}/UDFLoaderTargets.cmake)

# Register package in user's package registry
export(PACKAGE UDFLoader)

##
## Documentation generation
##
if(WITH_DOCS)
    # TODO: Add documentation generation
    message(FATAL_ERROR "Documentation generation not supported currently")
endif()
