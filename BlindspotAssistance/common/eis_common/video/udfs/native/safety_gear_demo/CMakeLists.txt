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
project(eis-udf-safety_gear_demo VERSION 1.0.0 LANGUAGES C CXX)

#find_package(InferenceEngine 2.1 REQUIRED)
add_library(safety_gear_demo SHARED "safety_gear_demo.cpp")
#find_library(CPU_LIB cpu_extension PATH /EIS/go/src/IEdgeInsights/common/udfs/native/safety_gear_demo)

target_include_directories(safety_gear_demo
    PRIVATE
        .
        ${EISMsgEnv_INCLUDE}
        ${EISUtils_INCLUDE}
        ${InferenceEngine_INCLUDE_DIRS}
        ${InferenceEngine_INCLUDE_DIRS}/../src/extension
        ${IE_EXTERNAL_DIR}
        ${IntelSafeString_INCLUDE}
    )

if(TARGET IE::ie_cpu_extension)
    add_definitions(-DWITH_EXTENSIONS)
endif()

if(TARGET IE::ie_cpu_extension)
    target_link_libraries(safety_gear_demo IE::ie_cpu_extension)
endif()

target_link_libraries(safety_gear_demo
    PUBLIC
        udfloader
        ${CPU_LIB}
        ${EISMsgEnv_LIBRARIES}
        ${EISUtils_LIBRARIES}
        ${OpenCV_LIBRARIES}
        ${InferenceEngine_LIBRARIES}
    PRIVATE
        ${IntelSafeString_LIBRARIES}
    )

install(
    TARGETS safety_gear_demo
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
