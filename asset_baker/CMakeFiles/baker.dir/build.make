# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.24

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
CMAKE_SOURCE_DIR = C:\Users\zaver\projects\VulkanEngine

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\zaver\projects\VulkanEngine

# Include any dependencies generated for this target.
include asset_baker/CMakeFiles/baker.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include asset_baker/CMakeFiles/baker.dir/compiler_depend.make

# Include the progress variables for this target.
include asset_baker/CMakeFiles/baker.dir/progress.make

# Include the compile flags for this target's objects.
include asset_baker/CMakeFiles/baker.dir/flags.make

asset_baker/CMakeFiles/baker.dir/asset_main.cpp.obj: asset_baker/CMakeFiles/baker.dir/flags.make
asset_baker/CMakeFiles/baker.dir/asset_main.cpp.obj: asset_baker/CMakeFiles/baker.dir/includes_CXX.rsp
asset_baker/CMakeFiles/baker.dir/asset_main.cpp.obj: asset_baker/asset_main.cpp
asset_baker/CMakeFiles/baker.dir/asset_main.cpp.obj: asset_baker/CMakeFiles/baker.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\zaver\projects\VulkanEngine\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object asset_baker/CMakeFiles/baker.dir/asset_main.cpp.obj"
	cd /d C:\Users\zaver\projects\VulkanEngine\asset_baker && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT asset_baker/CMakeFiles/baker.dir/asset_main.cpp.obj -MF CMakeFiles\baker.dir\asset_main.cpp.obj.d -o CMakeFiles\baker.dir\asset_main.cpp.obj -c C:\Users\zaver\projects\VulkanEngine\asset_baker\asset_main.cpp

asset_baker/CMakeFiles/baker.dir/asset_main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/baker.dir/asset_main.cpp.i"
	cd /d C:\Users\zaver\projects\VulkanEngine\asset_baker && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\zaver\projects\VulkanEngine\asset_baker\asset_main.cpp > CMakeFiles\baker.dir\asset_main.cpp.i

asset_baker/CMakeFiles/baker.dir/asset_main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/baker.dir/asset_main.cpp.s"
	cd /d C:\Users\zaver\projects\VulkanEngine\asset_baker && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\zaver\projects\VulkanEngine\asset_baker\asset_main.cpp -o CMakeFiles\baker.dir\asset_main.cpp.s

# Object files for target baker
baker_OBJECTS = \
"CMakeFiles/baker.dir/asset_main.cpp.obj"

# External object files for target baker
baker_EXTERNAL_OBJECTS =

bin/baker.exe: asset_baker/CMakeFiles/baker.dir/asset_main.cpp.obj
bin/baker.exe: asset_baker/CMakeFiles/baker.dir/build.make
bin/baker.exe: third_party/liblz4.a
bin/baker.exe: assetlib/libassetlib.a
bin/baker.exe: third_party/libtinyobjloader.a
bin/baker.exe: third_party/libfmt_lib.a
bin/baker.exe: third_party/liblz4.a
bin/baker.exe: asset_baker/CMakeFiles/baker.dir/linklibs.rsp
bin/baker.exe: asset_baker/CMakeFiles/baker.dir/objects1.rsp
bin/baker.exe: asset_baker/CMakeFiles/baker.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:\Users\zaver\projects\VulkanEngine\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ..\bin\baker.exe"
	cd /d C:\Users\zaver\projects\VulkanEngine\asset_baker && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\baker.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
asset_baker/CMakeFiles/baker.dir/build: bin/baker.exe
.PHONY : asset_baker/CMakeFiles/baker.dir/build

asset_baker/CMakeFiles/baker.dir/clean:
	cd /d C:\Users\zaver\projects\VulkanEngine\asset_baker && $(CMAKE_COMMAND) -P CMakeFiles\baker.dir\cmake_clean.cmake
.PHONY : asset_baker/CMakeFiles/baker.dir/clean

asset_baker/CMakeFiles/baker.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Users\zaver\projects\VulkanEngine C:\Users\zaver\projects\VulkanEngine\asset_baker C:\Users\zaver\projects\VulkanEngine C:\Users\zaver\projects\VulkanEngine\asset_baker C:\Users\zaver\projects\VulkanEngine\asset_baker\CMakeFiles\baker.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : asset_baker/CMakeFiles/baker.dir/depend

