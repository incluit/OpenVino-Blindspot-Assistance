# Copyright (C) 2018-2019 Intel Corporation

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#      http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required (VERSION 3.10)

include(CMakeLists_common.cmake)

set(TARGET_NAME "blindspot-assistance")

if( BUILD_DEMO_NAME AND NOT ${BUILD_DEMO_NAME} STREQUAL ${TARGET_NAME} )
    message(STATUS "DEMO ${TARGET_NAME} SKIPPED")
    return()
endif()

# Find OpenCV components if exist
find_package(OpenCV COMPONENTS highgui QUIET)
if(OpenCV_FOUND)
    message(STATUS "OPENCV is enabled")
    message(STATUS "OpenCV_INCLUDE_DIRS=${OpenCV_INCLUDE_DIRS}")
    message(STATUS "OpenCV_LIBS=${OpenCV_LIBS}")
else()
    message(STATUS "OPENCV is disabled or not found, " ${TARGET_NAME} " skiped")
    return()
endif()

find_package(ngraph)

file (GLOB MAIN_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        )

file (GLOB MAIN_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
        )

# Create named folders for the sources within the .vcproj
# Empty name lists them directly under the .vcproj
source_group("src" FILES ${MAIN_SRC})
source_group("include" FILES ${MAIN_HEADERS})

# Create library file from sources.
add_executable(${TARGET_NAME} ${MAIN_SRC} ${MAIN_HEADERS})

set_target_properties(${TARGET_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    COMPILE_PDB_NAME ${TARGET_NAME})

if(MULTICHANNEL_DEMO_USE_TBB)
    find_package(TBB REQUIRED tbb)
    target_link_libraries(${TARGET_NAME} ${TBB_IMPORTED_TARGETS})
    target_compile_definitions(${TARGET_NAME} PRIVATE
        USE_TBB=1
        __TBB_ALLOW_MUTABLE_FUNCTORS=1)

    if(FALSE) # disable task isolation for now due to bugs in tbb
        target_compile_definitions(${TARGET_NAME} PRIVATE
            TBB_PREVIEW_TASK_ISOLATION=1
            TBB_TASK_ISOLATION=1)
    endif()
endif()

add_subdirectory(src/common)
target_link_libraries(${TARGET_NAME} ${InferenceEngine_LIBRARIES} gflags ${OpenCV_LIBRARIES} ngraph::ngraph common)

if(UNIX)
    target_link_libraries( ${TARGET_NAME} pthread)
endif()

if(COMMAND add_cpplint_target)
    add_cpplint_target(${TARGET_NAME}_cpplint FOR_TARGETS ${TARGET_NAME})
endif()

if(NOT TARGET ie_samples)
    add_custom_target(ie_samples ALL)
endif()

add_dependencies(ie_samples ${TARGET_NAME})

target_link_libraries(${TARGET_NAME} monitors)

target_link_libraries(${TARGET_NAME}    eismsgbus
                                        eisutils
                                        eismsgenv
                                        pthread)

# Copy over TCP configuration files
file(GLOB CONFIGS "/app/BlindspotAssistance/common/eis_common/libs/EISMessageBus/examples/configs/*.json")

foreach(CONFIG ${CONFIGS})
    file(COPY "${CONFIG}" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/${ARCH}/${CMAKE_BUILD_TYPE}/configs")
endforeach()
