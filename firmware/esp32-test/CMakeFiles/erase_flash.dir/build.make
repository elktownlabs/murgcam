# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_SOURCE_DIR = /home/toby/git/cam/firmware/esp32-test

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/toby/git/cam/firmware/esp32-test

# Utility rule file for erase_flash.

# Include the progress variables for this target.
include CMakeFiles/erase_flash.dir/progress.make

CMakeFiles/erase_flash:
	cd /home/toby/esp/esp-idf/components/esptool_py && /usr/bin/cmake -D IDF_PATH="/home/toby/esp/esp-idf" -D ESPTOOLPY="python /home/toby/esp/esp-idf/components/esptool_py/esptool/esptool.py --chip esp32" -D ESPTOOL_ARGS="erase_flash" -P run_esptool.cmake

erase_flash: CMakeFiles/erase_flash
erase_flash: CMakeFiles/erase_flash.dir/build.make

.PHONY : erase_flash

# Rule to build all files generated by this target.
CMakeFiles/erase_flash.dir/build: erase_flash

.PHONY : CMakeFiles/erase_flash.dir/build

CMakeFiles/erase_flash.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/erase_flash.dir/cmake_clean.cmake
.PHONY : CMakeFiles/erase_flash.dir/clean

CMakeFiles/erase_flash.dir/depend:
	cd /home/toby/git/cam/firmware/esp32-test && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/toby/git/cam/firmware/esp32-test /home/toby/git/cam/firmware/esp32-test /home/toby/git/cam/firmware/esp32-test /home/toby/git/cam/firmware/esp32-test /home/toby/git/cam/firmware/esp32-test/CMakeFiles/erase_flash.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/erase_flash.dir/depend

