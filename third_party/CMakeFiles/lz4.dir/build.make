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
CMAKE_SOURCE_DIR = E:\MyEngine\MyEngine\AdAstrisEngine

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = E:\MyEngine\MyEngine\AdAstrisEngine

# Include any dependencies generated for this target.
include third_party/CMakeFiles/lz4.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include third_party/CMakeFiles/lz4.dir/compiler_depend.make

# Include the progress variables for this target.
include third_party/CMakeFiles/lz4.dir/progress.make

# Include the compile flags for this target's objects.
include third_party/CMakeFiles/lz4.dir/flags.make

third_party/CMakeFiles/lz4.dir/lz4/lz4.c.obj: third_party/CMakeFiles/lz4.dir/flags.make
third_party/CMakeFiles/lz4.dir/lz4/lz4.c.obj: third_party/CMakeFiles/lz4.dir/includes_C.rsp
third_party/CMakeFiles/lz4.dir/lz4/lz4.c.obj: third_party/lz4/lz4.c
third_party/CMakeFiles/lz4.dir/lz4/lz4.c.obj: third_party/CMakeFiles/lz4.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\MyEngine\MyEngine\AdAstrisEngine\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object third_party/CMakeFiles/lz4.dir/lz4/lz4.c.obj"
	cd /d E:\MyEngine\MyEngine\AdAstrisEngine\third_party && C:\msys64\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT third_party/CMakeFiles/lz4.dir/lz4/lz4.c.obj -MF CMakeFiles\lz4.dir\lz4\lz4.c.obj.d -o CMakeFiles\lz4.dir\lz4\lz4.c.obj -c E:\MyEngine\MyEngine\AdAstrisEngine\third_party\lz4\lz4.c

third_party/CMakeFiles/lz4.dir/lz4/lz4.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lz4.dir/lz4/lz4.c.i"
	cd /d E:\MyEngine\MyEngine\AdAstrisEngine\third_party && C:\msys64\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E E:\MyEngine\MyEngine\AdAstrisEngine\third_party\lz4\lz4.c > CMakeFiles\lz4.dir\lz4\lz4.c.i

third_party/CMakeFiles/lz4.dir/lz4/lz4.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lz4.dir/lz4/lz4.c.s"
	cd /d E:\MyEngine\MyEngine\AdAstrisEngine\third_party && C:\msys64\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S E:\MyEngine\MyEngine\AdAstrisEngine\third_party\lz4\lz4.c -o CMakeFiles\lz4.dir\lz4\lz4.c.s

# Object files for target lz4
lz4_OBJECTS = \
"CMakeFiles/lz4.dir/lz4/lz4.c.obj"

# External object files for target lz4
lz4_EXTERNAL_OBJECTS =

third_party/liblz4.a: third_party/CMakeFiles/lz4.dir/lz4/lz4.c.obj
third_party/liblz4.a: third_party/CMakeFiles/lz4.dir/build.make
third_party/liblz4.a: third_party/CMakeFiles/lz4.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=E:\MyEngine\MyEngine\AdAstrisEngine\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C static library liblz4.a"
	cd /d E:\MyEngine\MyEngine\AdAstrisEngine\third_party && $(CMAKE_COMMAND) -P CMakeFiles\lz4.dir\cmake_clean_target.cmake
	cd /d E:\MyEngine\MyEngine\AdAstrisEngine\third_party && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\lz4.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
third_party/CMakeFiles/lz4.dir/build: third_party/liblz4.a
.PHONY : third_party/CMakeFiles/lz4.dir/build

third_party/CMakeFiles/lz4.dir/clean:
	cd /d E:\MyEngine\MyEngine\AdAstrisEngine\third_party && $(CMAKE_COMMAND) -P CMakeFiles\lz4.dir\cmake_clean.cmake
.PHONY : third_party/CMakeFiles/lz4.dir/clean

third_party/CMakeFiles/lz4.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" E:\MyEngine\MyEngine\AdAstrisEngine E:\MyEngine\MyEngine\AdAstrisEngine\third_party E:\MyEngine\MyEngine\AdAstrisEngine E:\MyEngine\MyEngine\AdAstrisEngine\third_party E:\MyEngine\MyEngine\AdAstrisEngine\third_party\CMakeFiles\lz4.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : third_party/CMakeFiles/lz4.dir/depend

