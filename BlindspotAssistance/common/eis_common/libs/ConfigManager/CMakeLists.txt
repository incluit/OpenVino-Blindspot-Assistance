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

cmake_minimum_required(VERSION 3.11)
project(eis-configmanager VERSION 1.0.0 LANGUAGES C CXX)

# Define CMake options

option(WITH_GO_ENV_CONFIG "Install Go EnvConfig" OFF)
option(WITH_PYTHON   "Compile with Py Config Manager and Py EnvConfig" OFF)
option(WITH_EXAMPLES "Compile with examples" OFF)
option(WITH_TESTS    "Compile with unit tests" OFF)

# Globals
set(EIS_COMMON_CMAKE "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
set(EIS_PYTHON_CFG_MGR "${CMAKE_CURRENT_SOURCE_DIR}/python/")

# Set C standard to C11
set(CMAKE_CXX_STANDARD 11)

# Set CFLAGS
set(CMAKE_C_FLAGS "-fPIE -fPIC -O2 -Wall -pedantic -fstack-protector-strong -D_FORTIFY_SOURCE=2")

# Set LDFLAGS
set(CMAKE_SHARED_LINKER_FLAGS "-z noexecstack -z relro -z now")

# Add cmake module path
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${EIS_COMMON_CMAKE}")

# Set EIS GO Config Manger src path 
set(EIS_GO_CFGMGR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/go/ConfigManager)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json")
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
        ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
    )
endif()

# Finding libraries
find_package(cJSON REQUIRED)
find_package(EISUtils REQUIRED)
find_package(IntelSafeString REQUIRED)

# Install Go EnvConfig if specified to
if(WITH_GO_ENV_CONFIG)
    message("-- Configuring Go EnvConfig installation")

    # Verify the $GOPATH environmental variable exists
    if("$ENV{GOPATH}" STREQUAL "")
        message(FATAL_ERROR "-- $GOPATH envronmental variable is not set")
    endif()

    # Configure install command to copy the Go EnvConfig to the
    # user's $GOPATH
    install(
        DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}/go/EnvConfig
        DESTINATION
            $ENV{GOPATH}/src/
    )
endif()

# Finding libraries
find_package(EISUtils REQUIRED)
find_package(EISMsgEnv REQUIRED)
find_package(IntelSafeString REQUIRED)

# Include header directories
include_directories(
    include/
    ${EISMsgEnv_INCLUDE}
    ${EISUtils_INCLUDE}
    ${IntelSafeString_INCLUDE})

if(WITH_PYTHON)
    add_subdirectory("${EIS_PYTHON_CFG_MGR}")
endif()

# Genreate CGO static library
find_program(GO_EXECUTABLE "go" DOC "Go Executable")

add_custom_target(cgo-config-mgr-code
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/cgo/cgo.go)
add_custom_target(cgo-config-mgr
    COMMENT "Generating cgo static library"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/cgo/cgo.go
    COMMAND cp -rf ${EIS_GO_CFGMGR_SRC} $ENV{GOPATH}/src/
    COMMAND ${GO_EXECUTABLE} build -buildmode=c-archive -o ${CMAKE_CURRENT_BINARY_DIR}/go_config_manager.a
    COMMAND mv ${CMAKE_CURRENT_BINARY_DIR}/go_config_manager.h ${CMAKE_CURRENT_SOURCE_DIR}/src/cgo/
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/cgo/
    VERBATIM)

# Get all source files
file(GLOB SOURCES "src/*.c" "src/*.cpp")
set_source_files_properties(${SOURCES} PROPERTIES LANGUAGE C)

add_library(eisconfigmgr SHARED ${SOURCES})
add_dependencies(eisconfigmgr cgo-config-mgr)

add_library(goconfigmanager STATIC IMPORTED)
add_dependencies(goconfigmanager cgo-config-mgr)
set_target_properties(goconfigmanager PROPERTIES IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/go_config_manager.a)
target_link_libraries(eisconfigmgr
    PUBLIC
        pthread
        ${EISMsgEnv_LIBRARIES}
        ${EISUtils_LIBRARIES}
        goconfigmanager
    PRIVATE
        ${IntelSafeString_LIBRARIES})

# If compile in debug mode, set DEBUG flag for C code
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    target_compile_definitions(eisconfigmgr PRIVATE DEBUG=1)
endif()

if(WITH_EXAMPLES)
    add_subdirectory(examples/)
endif()
if(WITH_TESTS)
    add_subdirectory(tests/)
endif()

##
## Configure pkg-config file to be installed for the EIS ConfigMgr
##
set(PKG_CONFIG_IN  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/libeisconfigmgr.pc.in")
set(PKG_CONFIG_OUT "${CMAKE_CURRENT_BINARY_DIR}/libeisconfigmgr.pc")
set(DEST_DIR       "${CMAKE_INSTALL_PREFIX}")
set(PRIVATE_LIBS   "-lzmq -lcjson")

configure_file(${PKG_CONFIG_IN} ${PKG_CONFIG_OUT} @ONLY)

##
## Add CMake configuration for installing the library including files for other
## projects finding the library using CMake
##

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/EISConfigMgr)

install(TARGETS eisconfigmgr
    EXPORT eisconfigmgr-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

set_target_properties(eisconfigmgr PROPERTIES EXPORT_NAME EISConfigMgr)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install pkg-config libeisconfigmgr.pc file
install(
    FILES
        ${PKG_CONFIG_OUT}
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# Export targets to a script
install(EXPORT eisconfigmgr-targets
    FILE
        EISConfigMgrTargets.cmake
    DESTINATION
        ${INSTALL_CONFIGDIR}
)

# Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/EISConfigMgrConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/EISConfigMgrConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/EISConfigMgrConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

# Install the config, configversion and custom find modules
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/EISConfigMgrConfigVersion.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/EISConfigMgrConfig.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

export(EXPORT eisconfigmgr-targets
       FILE ${CMAKE_CURRENT_BINARY_DIR}/EISConfigMgrTargets.cmake)

# Register package in user's package registry
export(PACKAGE EISConfigMgr)
