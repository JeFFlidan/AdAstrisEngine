# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.23

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\CMake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\CMake\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = E:\MyEngine\MyEngine\VulkanEngine

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = E:\MyEngine\MyEngine\VulkanEngine

# Utility rule file for logger.

# Include any custom commands dependencies for this target.
include engine/src/CMakeFiles/logger.dir/compiler_depend.make

# Include the progress variables for this target.
include engine/src/CMakeFiles/logger.dir/progress.make

logger: engine/src/CMakeFiles/logger.dir/build.make
.PHONY : logger

# Rule to build all files generated by this target.
engine/src/CMakeFiles/logger.dir/build: logger
.PHONY : engine/src/CMakeFiles/logger.dir/build

engine/src/CMakeFiles/logger.dir/clean:
	cd /d E:\MyEngine\MyEngine\VulkanEngine\engine\src && $(CMAKE_COMMAND) -P CMakeFiles\logger.dir\cmake_clean.cmake
.PHONY : engine/src/CMakeFiles/logger.dir/clean

engine/src/CMakeFiles/logger.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" E:\MyEngine\MyEngine\VulkanEngine E:\MyEngine\MyEngine\VulkanEngine\engine\src E:\MyEngine\MyEngine\VulkanEngine E:\MyEngine\MyEngine\VulkanEngine\engine\src E:\MyEngine\MyEngine\VulkanEngine\engine\src\CMakeFiles\logger.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : engine/src/CMakeFiles/logger.dir/depend
