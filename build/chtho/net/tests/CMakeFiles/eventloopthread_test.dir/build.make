# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /data/projects/chtho

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /data/projects/chtho/build

# Include any dependencies generated for this target.
include chtho/net/tests/CMakeFiles/eventloopthread_test.dir/depend.make

# Include the progress variables for this target.
include chtho/net/tests/CMakeFiles/eventloopthread_test.dir/progress.make

# Include the compile flags for this target's objects.
include chtho/net/tests/CMakeFiles/eventloopthread_test.dir/flags.make

chtho/net/tests/CMakeFiles/eventloopthread_test.dir/EventLoopThread_test.cpp.o: chtho/net/tests/CMakeFiles/eventloopthread_test.dir/flags.make
chtho/net/tests/CMakeFiles/eventloopthread_test.dir/EventLoopThread_test.cpp.o: ../chtho/net/tests/EventLoopThread_test.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/data/projects/chtho/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object chtho/net/tests/CMakeFiles/eventloopthread_test.dir/EventLoopThread_test.cpp.o"
	cd /data/projects/chtho/build/chtho/net/tests && g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/eventloopthread_test.dir/EventLoopThread_test.cpp.o -c /data/projects/chtho/chtho/net/tests/EventLoopThread_test.cpp

chtho/net/tests/CMakeFiles/eventloopthread_test.dir/EventLoopThread_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/eventloopthread_test.dir/EventLoopThread_test.cpp.i"
	cd /data/projects/chtho/build/chtho/net/tests && g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /data/projects/chtho/chtho/net/tests/EventLoopThread_test.cpp > CMakeFiles/eventloopthread_test.dir/EventLoopThread_test.cpp.i

chtho/net/tests/CMakeFiles/eventloopthread_test.dir/EventLoopThread_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/eventloopthread_test.dir/EventLoopThread_test.cpp.s"
	cd /data/projects/chtho/build/chtho/net/tests && g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /data/projects/chtho/chtho/net/tests/EventLoopThread_test.cpp -o CMakeFiles/eventloopthread_test.dir/EventLoopThread_test.cpp.s

chtho/net/tests/CMakeFiles/eventloopthread_test.dir/EventLoopThread_test.cpp.o.requires:

.PHONY : chtho/net/tests/CMakeFiles/eventloopthread_test.dir/EventLoopThread_test.cpp.o.requires

chtho/net/tests/CMakeFiles/eventloopthread_test.dir/EventLoopThread_test.cpp.o.provides: chtho/net/tests/CMakeFiles/eventloopthread_test.dir/EventLoopThread_test.cpp.o.requires
	$(MAKE) -f chtho/net/tests/CMakeFiles/eventloopthread_test.dir/build.make chtho/net/tests/CMakeFiles/eventloopthread_test.dir/EventLoopThread_test.cpp.o.provides.build
.PHONY : chtho/net/tests/CMakeFiles/eventloopthread_test.dir/EventLoopThread_test.cpp.o.provides

chtho/net/tests/CMakeFiles/eventloopthread_test.dir/EventLoopThread_test.cpp.o.provides.build: chtho/net/tests/CMakeFiles/eventloopthread_test.dir/EventLoopThread_test.cpp.o


# Object files for target eventloopthread_test
eventloopthread_test_OBJECTS = \
"CMakeFiles/eventloopthread_test.dir/EventLoopThread_test.cpp.o"

# External object files for target eventloopthread_test
eventloopthread_test_EXTERNAL_OBJECTS =

bin/eventloopthread_test: chtho/net/tests/CMakeFiles/eventloopthread_test.dir/EventLoopThread_test.cpp.o
bin/eventloopthread_test: chtho/net/tests/CMakeFiles/eventloopthread_test.dir/build.make
bin/eventloopthread_test: lib/libchtho_net.a
bin/eventloopthread_test: lib/libchtho_logging.a
bin/eventloopthread_test: lib/libchtho_base.a
bin/eventloopthread_test: lib/libchtho_time.a
bin/eventloopthread_test: lib/libchtho_threads.a
bin/eventloopthread_test: chtho/net/tests/CMakeFiles/eventloopthread_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/data/projects/chtho/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../../bin/eventloopthread_test"
	cd /data/projects/chtho/build/chtho/net/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/eventloopthread_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
chtho/net/tests/CMakeFiles/eventloopthread_test.dir/build: bin/eventloopthread_test

.PHONY : chtho/net/tests/CMakeFiles/eventloopthread_test.dir/build

chtho/net/tests/CMakeFiles/eventloopthread_test.dir/requires: chtho/net/tests/CMakeFiles/eventloopthread_test.dir/EventLoopThread_test.cpp.o.requires

.PHONY : chtho/net/tests/CMakeFiles/eventloopthread_test.dir/requires

chtho/net/tests/CMakeFiles/eventloopthread_test.dir/clean:
	cd /data/projects/chtho/build/chtho/net/tests && $(CMAKE_COMMAND) -P CMakeFiles/eventloopthread_test.dir/cmake_clean.cmake
.PHONY : chtho/net/tests/CMakeFiles/eventloopthread_test.dir/clean

chtho/net/tests/CMakeFiles/eventloopthread_test.dir/depend:
	cd /data/projects/chtho/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /data/projects/chtho /data/projects/chtho/chtho/net/tests /data/projects/chtho/build /data/projects/chtho/build/chtho/net/tests /data/projects/chtho/build/chtho/net/tests/CMakeFiles/eventloopthread_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : chtho/net/tests/CMakeFiles/eventloopthread_test.dir/depend

