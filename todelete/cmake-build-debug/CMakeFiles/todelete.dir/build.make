# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.9

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
CMAKE_COMMAND = /opt/clion-2017.3.4/bin/cmake/bin/cmake

# The command to remove a file.
RM = /opt/clion-2017.3.4/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/jencmart/Dropbox/development/fitcvut/bi-osy/todelete

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/jencmart/Dropbox/development/fitcvut/bi-osy/todelete/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/todelete.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/todelete.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/todelete.dir/flags.make

CMakeFiles/todelete.dir/main.cpp.o: CMakeFiles/todelete.dir/flags.make
CMakeFiles/todelete.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jencmart/Dropbox/development/fitcvut/bi-osy/todelete/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/todelete.dir/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/todelete.dir/main.cpp.o -c /home/jencmart/Dropbox/development/fitcvut/bi-osy/todelete/main.cpp

CMakeFiles/todelete.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/todelete.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/jencmart/Dropbox/development/fitcvut/bi-osy/todelete/main.cpp > CMakeFiles/todelete.dir/main.cpp.i

CMakeFiles/todelete.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/todelete.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/jencmart/Dropbox/development/fitcvut/bi-osy/todelete/main.cpp -o CMakeFiles/todelete.dir/main.cpp.s

CMakeFiles/todelete.dir/main.cpp.o.requires:

.PHONY : CMakeFiles/todelete.dir/main.cpp.o.requires

CMakeFiles/todelete.dir/main.cpp.o.provides: CMakeFiles/todelete.dir/main.cpp.o.requires
	$(MAKE) -f CMakeFiles/todelete.dir/build.make CMakeFiles/todelete.dir/main.cpp.o.provides.build
.PHONY : CMakeFiles/todelete.dir/main.cpp.o.provides

CMakeFiles/todelete.dir/main.cpp.o.provides.build: CMakeFiles/todelete.dir/main.cpp.o


# Object files for target todelete
todelete_OBJECTS = \
"CMakeFiles/todelete.dir/main.cpp.o"

# External object files for target todelete
todelete_EXTERNAL_OBJECTS =

todelete: CMakeFiles/todelete.dir/main.cpp.o
todelete: CMakeFiles/todelete.dir/build.make
todelete: CMakeFiles/todelete.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/jencmart/Dropbox/development/fitcvut/bi-osy/todelete/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable todelete"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/todelete.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/todelete.dir/build: todelete

.PHONY : CMakeFiles/todelete.dir/build

CMakeFiles/todelete.dir/requires: CMakeFiles/todelete.dir/main.cpp.o.requires

.PHONY : CMakeFiles/todelete.dir/requires

CMakeFiles/todelete.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/todelete.dir/cmake_clean.cmake
.PHONY : CMakeFiles/todelete.dir/clean

CMakeFiles/todelete.dir/depend:
	cd /home/jencmart/Dropbox/development/fitcvut/bi-osy/todelete/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jencmart/Dropbox/development/fitcvut/bi-osy/todelete /home/jencmart/Dropbox/development/fitcvut/bi-osy/todelete /home/jencmart/Dropbox/development/fitcvut/bi-osy/todelete/cmake-build-debug /home/jencmart/Dropbox/development/fitcvut/bi-osy/todelete/cmake-build-debug /home/jencmart/Dropbox/development/fitcvut/bi-osy/todelete/cmake-build-debug/CMakeFiles/todelete.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/todelete.dir/depend

