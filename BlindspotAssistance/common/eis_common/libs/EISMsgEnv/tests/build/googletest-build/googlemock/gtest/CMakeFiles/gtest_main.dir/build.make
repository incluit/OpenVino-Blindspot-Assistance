# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.15

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /opt/edge_insights_industrial/Edge_Insights_for_Industrial_2.3.1/IEdgeInsights/common/libs/EISMsgEnv/tests

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /opt/edge_insights_industrial/Edge_Insights_for_Industrial_2.3.1/IEdgeInsights/common/libs/EISMsgEnv/tests/build

# Include any dependencies generated for this target.
include googletest-build/googlemock/gtest/CMakeFiles/gtest_main.dir/depend.make

# Include the progress variables for this target.
include googletest-build/googlemock/gtest/CMakeFiles/gtest_main.dir/progress.make

# Include the compile flags for this target's objects.
include googletest-build/googlemock/gtest/CMakeFiles/gtest_main.dir/flags.make

googletest-build/googlemock/gtest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.o: googletest-build/googlemock/gtest/CMakeFiles/gtest_main.dir/flags.make
googletest-build/googlemock/gtest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.o: googletest-src/googletest/src/gtest_main.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/opt/edge_insights_industrial/Edge_Insights_for_Industrial_2.3.1/IEdgeInsights/common/libs/EISMsgEnv/tests/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object googletest-build/googlemock/gtest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.o"
	cd /opt/edge_insights_industrial/Edge_Insights_for_Industrial_2.3.1/IEdgeInsights/common/libs/EISMsgEnv/tests/build/googletest-build/googlemock/gtest && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/gtest_main.dir/src/gtest_main.cc.o -c /opt/edge_insights_industrial/Edge_Insights_for_Industrial_2.3.1/IEdgeInsights/common/libs/EISMsgEnv/tests/build/googletest-src/googletest/src/gtest_main.cc

googletest-build/googlemock/gtest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/gtest_main.dir/src/gtest_main.cc.i"
	cd /opt/edge_insights_industrial/Edge_Insights_for_Industrial_2.3.1/IEdgeInsights/common/libs/EISMsgEnv/tests/build/googletest-build/googlemock/gtest && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /opt/edge_insights_industrial/Edge_Insights_for_Industrial_2.3.1/IEdgeInsights/common/libs/EISMsgEnv/tests/build/googletest-src/googletest/src/gtest_main.cc > CMakeFiles/gtest_main.dir/src/gtest_main.cc.i

googletest-build/googlemock/gtest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/gtest_main.dir/src/gtest_main.cc.s"
	cd /opt/edge_insights_industrial/Edge_Insights_for_Industrial_2.3.1/IEdgeInsights/common/libs/EISMsgEnv/tests/build/googletest-build/googlemock/gtest && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /opt/edge_insights_industrial/Edge_Insights_for_Industrial_2.3.1/IEdgeInsights/common/libs/EISMsgEnv/tests/build/googletest-src/googletest/src/gtest_main.cc -o CMakeFiles/gtest_main.dir/src/gtest_main.cc.s

# Object files for target gtest_main
gtest_main_OBJECTS = \
"CMakeFiles/gtest_main.dir/src/gtest_main.cc.o"

# External object files for target gtest_main
gtest_main_EXTERNAL_OBJECTS =

googletest-build/googlemock/gtest/libgtest_main.a: googletest-build/googlemock/gtest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.o
googletest-build/googlemock/gtest/libgtest_main.a: googletest-build/googlemock/gtest/CMakeFiles/gtest_main.dir/build.make
googletest-build/googlemock/gtest/libgtest_main.a: googletest-build/googlemock/gtest/CMakeFiles/gtest_main.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/opt/edge_insights_industrial/Edge_Insights_for_Industrial_2.3.1/IEdgeInsights/common/libs/EISMsgEnv/tests/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libgtest_main.a"
	cd /opt/edge_insights_industrial/Edge_Insights_for_Industrial_2.3.1/IEdgeInsights/common/libs/EISMsgEnv/tests/build/googletest-build/googlemock/gtest && $(CMAKE_COMMAND) -P CMakeFiles/gtest_main.dir/cmake_clean_target.cmake
	cd /opt/edge_insights_industrial/Edge_Insights_for_Industrial_2.3.1/IEdgeInsights/common/libs/EISMsgEnv/tests/build/googletest-build/googlemock/gtest && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/gtest_main.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
googletest-build/googlemock/gtest/CMakeFiles/gtest_main.dir/build: googletest-build/googlemock/gtest/libgtest_main.a

.PHONY : googletest-build/googlemock/gtest/CMakeFiles/gtest_main.dir/build

googletest-build/googlemock/gtest/CMakeFiles/gtest_main.dir/clean:
	cd /opt/edge_insights_industrial/Edge_Insights_for_Industrial_2.3.1/IEdgeInsights/common/libs/EISMsgEnv/tests/build/googletest-build/googlemock/gtest && $(CMAKE_COMMAND) -P CMakeFiles/gtest_main.dir/cmake_clean.cmake
.PHONY : googletest-build/googlemock/gtest/CMakeFiles/gtest_main.dir/clean

googletest-build/googlemock/gtest/CMakeFiles/gtest_main.dir/depend:
	cd /opt/edge_insights_industrial/Edge_Insights_for_Industrial_2.3.1/IEdgeInsights/common/libs/EISMsgEnv/tests/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /opt/edge_insights_industrial/Edge_Insights_for_Industrial_2.3.1/IEdgeInsights/common/libs/EISMsgEnv/tests /opt/edge_insights_industrial/Edge_Insights_for_Industrial_2.3.1/IEdgeInsights/common/libs/EISMsgEnv/tests/build/googletest-src/googletest /opt/edge_insights_industrial/Edge_Insights_for_Industrial_2.3.1/IEdgeInsights/common/libs/EISMsgEnv/tests/build /opt/edge_insights_industrial/Edge_Insights_for_Industrial_2.3.1/IEdgeInsights/common/libs/EISMsgEnv/tests/build/googletest-build/googlemock/gtest /opt/edge_insights_industrial/Edge_Insights_for_Industrial_2.3.1/IEdgeInsights/common/libs/EISMsgEnv/tests/build/googletest-build/googlemock/gtest/CMakeFiles/gtest_main.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : googletest-build/googlemock/gtest/CMakeFiles/gtest_main.dir/depend

