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
include assetlib/CMakeFiles/assetlib.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include assetlib/CMakeFiles/assetlib.dir/compiler_depend.make

# Include the progress variables for this target.
include assetlib/CMakeFiles/assetlib.dir/progress.make

# Include the compile flags for this target's objects.
include assetlib/CMakeFiles/assetlib.dir/flags.make

assetlib/CMakeFiles/assetlib.dir/mesh_asset.cpp.obj: assetlib/CMakeFiles/assetlib.dir/flags.make
assetlib/CMakeFiles/assetlib.dir/mesh_asset.cpp.obj: assetlib/CMakeFiles/assetlib.dir/includes_CXX.rsp
assetlib/CMakeFiles/assetlib.dir/mesh_asset.cpp.obj: assetlib/mesh_asset.cpp
assetlib/CMakeFiles/assetlib.dir/mesh_asset.cpp.obj: assetlib/CMakeFiles/assetlib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\zaver\projects\VulkanEngine\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object assetlib/CMakeFiles/assetlib.dir/mesh_asset.cpp.obj"
	cd /d C:\Users\zaver\projects\VulkanEngine\assetlib && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT assetlib/CMakeFiles/assetlib.dir/mesh_asset.cpp.obj -MF CMakeFiles\assetlib.dir\mesh_asset.cpp.obj.d -o CMakeFiles\assetlib.dir\mesh_asset.cpp.obj -c C:\Users\zaver\projects\VulkanEngine\assetlib\mesh_asset.cpp

assetlib/CMakeFiles/assetlib.dir/mesh_asset.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/assetlib.dir/mesh_asset.cpp.i"
	cd /d C:\Users\zaver\projects\VulkanEngine\assetlib && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\zaver\projects\VulkanEngine\assetlib\mesh_asset.cpp > CMakeFiles\assetlib.dir\mesh_asset.cpp.i

assetlib/CMakeFiles/assetlib.dir/mesh_asset.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/assetlib.dir/mesh_asset.cpp.s"
	cd /d C:\Users\zaver\projects\VulkanEngine\assetlib && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\zaver\projects\VulkanEngine\assetlib\mesh_asset.cpp -o CMakeFiles\assetlib.dir\mesh_asset.cpp.s

assetlib/CMakeFiles/assetlib.dir/material_asset.cpp.obj: assetlib/CMakeFiles/assetlib.dir/flags.make
assetlib/CMakeFiles/assetlib.dir/material_asset.cpp.obj: assetlib/CMakeFiles/assetlib.dir/includes_CXX.rsp
assetlib/CMakeFiles/assetlib.dir/material_asset.cpp.obj: assetlib/material_asset.cpp
assetlib/CMakeFiles/assetlib.dir/material_asset.cpp.obj: assetlib/CMakeFiles/assetlib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\zaver\projects\VulkanEngine\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object assetlib/CMakeFiles/assetlib.dir/material_asset.cpp.obj"
	cd /d C:\Users\zaver\projects\VulkanEngine\assetlib && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT assetlib/CMakeFiles/assetlib.dir/material_asset.cpp.obj -MF CMakeFiles\assetlib.dir\material_asset.cpp.obj.d -o CMakeFiles\assetlib.dir\material_asset.cpp.obj -c C:\Users\zaver\projects\VulkanEngine\assetlib\material_asset.cpp

assetlib/CMakeFiles/assetlib.dir/material_asset.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/assetlib.dir/material_asset.cpp.i"
	cd /d C:\Users\zaver\projects\VulkanEngine\assetlib && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\zaver\projects\VulkanEngine\assetlib\material_asset.cpp > CMakeFiles\assetlib.dir\material_asset.cpp.i

assetlib/CMakeFiles/assetlib.dir/material_asset.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/assetlib.dir/material_asset.cpp.s"
	cd /d C:\Users\zaver\projects\VulkanEngine\assetlib && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\zaver\projects\VulkanEngine\assetlib\material_asset.cpp -o CMakeFiles\assetlib.dir\material_asset.cpp.s

assetlib/CMakeFiles/assetlib.dir/texture_asset.cpp.obj: assetlib/CMakeFiles/assetlib.dir/flags.make
assetlib/CMakeFiles/assetlib.dir/texture_asset.cpp.obj: assetlib/CMakeFiles/assetlib.dir/includes_CXX.rsp
assetlib/CMakeFiles/assetlib.dir/texture_asset.cpp.obj: assetlib/texture_asset.cpp
assetlib/CMakeFiles/assetlib.dir/texture_asset.cpp.obj: assetlib/CMakeFiles/assetlib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\zaver\projects\VulkanEngine\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object assetlib/CMakeFiles/assetlib.dir/texture_asset.cpp.obj"
	cd /d C:\Users\zaver\projects\VulkanEngine\assetlib && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT assetlib/CMakeFiles/assetlib.dir/texture_asset.cpp.obj -MF CMakeFiles\assetlib.dir\texture_asset.cpp.obj.d -o CMakeFiles\assetlib.dir\texture_asset.cpp.obj -c C:\Users\zaver\projects\VulkanEngine\assetlib\texture_asset.cpp

assetlib/CMakeFiles/assetlib.dir/texture_asset.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/assetlib.dir/texture_asset.cpp.i"
	cd /d C:\Users\zaver\projects\VulkanEngine\assetlib && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\zaver\projects\VulkanEngine\assetlib\texture_asset.cpp > CMakeFiles\assetlib.dir\texture_asset.cpp.i

assetlib/CMakeFiles/assetlib.dir/texture_asset.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/assetlib.dir/texture_asset.cpp.s"
	cd /d C:\Users\zaver\projects\VulkanEngine\assetlib && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\zaver\projects\VulkanEngine\assetlib\texture_asset.cpp -o CMakeFiles\assetlib.dir\texture_asset.cpp.s

assetlib/CMakeFiles/assetlib.dir/prefab_asset.cpp.obj: assetlib/CMakeFiles/assetlib.dir/flags.make
assetlib/CMakeFiles/assetlib.dir/prefab_asset.cpp.obj: assetlib/CMakeFiles/assetlib.dir/includes_CXX.rsp
assetlib/CMakeFiles/assetlib.dir/prefab_asset.cpp.obj: assetlib/prefab_asset.cpp
assetlib/CMakeFiles/assetlib.dir/prefab_asset.cpp.obj: assetlib/CMakeFiles/assetlib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\zaver\projects\VulkanEngine\CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object assetlib/CMakeFiles/assetlib.dir/prefab_asset.cpp.obj"
	cd /d C:\Users\zaver\projects\VulkanEngine\assetlib && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT assetlib/CMakeFiles/assetlib.dir/prefab_asset.cpp.obj -MF CMakeFiles\assetlib.dir\prefab_asset.cpp.obj.d -o CMakeFiles\assetlib.dir\prefab_asset.cpp.obj -c C:\Users\zaver\projects\VulkanEngine\assetlib\prefab_asset.cpp

assetlib/CMakeFiles/assetlib.dir/prefab_asset.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/assetlib.dir/prefab_asset.cpp.i"
	cd /d C:\Users\zaver\projects\VulkanEngine\assetlib && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\zaver\projects\VulkanEngine\assetlib\prefab_asset.cpp > CMakeFiles\assetlib.dir\prefab_asset.cpp.i

assetlib/CMakeFiles/assetlib.dir/prefab_asset.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/assetlib.dir/prefab_asset.cpp.s"
	cd /d C:\Users\zaver\projects\VulkanEngine\assetlib && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\zaver\projects\VulkanEngine\assetlib\prefab_asset.cpp -o CMakeFiles\assetlib.dir\prefab_asset.cpp.s

assetlib/CMakeFiles/assetlib.dir/asset_loader.cpp.obj: assetlib/CMakeFiles/assetlib.dir/flags.make
assetlib/CMakeFiles/assetlib.dir/asset_loader.cpp.obj: assetlib/CMakeFiles/assetlib.dir/includes_CXX.rsp
assetlib/CMakeFiles/assetlib.dir/asset_loader.cpp.obj: assetlib/asset_loader.cpp
assetlib/CMakeFiles/assetlib.dir/asset_loader.cpp.obj: assetlib/CMakeFiles/assetlib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\zaver\projects\VulkanEngine\CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object assetlib/CMakeFiles/assetlib.dir/asset_loader.cpp.obj"
	cd /d C:\Users\zaver\projects\VulkanEngine\assetlib && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT assetlib/CMakeFiles/assetlib.dir/asset_loader.cpp.obj -MF CMakeFiles\assetlib.dir\asset_loader.cpp.obj.d -o CMakeFiles\assetlib.dir\asset_loader.cpp.obj -c C:\Users\zaver\projects\VulkanEngine\assetlib\asset_loader.cpp

assetlib/CMakeFiles/assetlib.dir/asset_loader.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/assetlib.dir/asset_loader.cpp.i"
	cd /d C:\Users\zaver\projects\VulkanEngine\assetlib && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\zaver\projects\VulkanEngine\assetlib\asset_loader.cpp > CMakeFiles\assetlib.dir\asset_loader.cpp.i

assetlib/CMakeFiles/assetlib.dir/asset_loader.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/assetlib.dir/asset_loader.cpp.s"
	cd /d C:\Users\zaver\projects\VulkanEngine\assetlib && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\zaver\projects\VulkanEngine\assetlib\asset_loader.cpp -o CMakeFiles\assetlib.dir\asset_loader.cpp.s

# Object files for target assetlib
assetlib_OBJECTS = \
"CMakeFiles/assetlib.dir/mesh_asset.cpp.obj" \
"CMakeFiles/assetlib.dir/material_asset.cpp.obj" \
"CMakeFiles/assetlib.dir/texture_asset.cpp.obj" \
"CMakeFiles/assetlib.dir/prefab_asset.cpp.obj" \
"CMakeFiles/assetlib.dir/asset_loader.cpp.obj"

# External object files for target assetlib
assetlib_EXTERNAL_OBJECTS =

assetlib/libassetlib.a: assetlib/CMakeFiles/assetlib.dir/mesh_asset.cpp.obj
assetlib/libassetlib.a: assetlib/CMakeFiles/assetlib.dir/material_asset.cpp.obj
assetlib/libassetlib.a: assetlib/CMakeFiles/assetlib.dir/texture_asset.cpp.obj
assetlib/libassetlib.a: assetlib/CMakeFiles/assetlib.dir/prefab_asset.cpp.obj
assetlib/libassetlib.a: assetlib/CMakeFiles/assetlib.dir/asset_loader.cpp.obj
assetlib/libassetlib.a: assetlib/CMakeFiles/assetlib.dir/build.make
assetlib/libassetlib.a: assetlib/CMakeFiles/assetlib.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:\Users\zaver\projects\VulkanEngine\CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX static library libassetlib.a"
	cd /d C:\Users\zaver\projects\VulkanEngine\assetlib && $(CMAKE_COMMAND) -P CMakeFiles\assetlib.dir\cmake_clean_target.cmake
	cd /d C:\Users\zaver\projects\VulkanEngine\assetlib && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\assetlib.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
assetlib/CMakeFiles/assetlib.dir/build: assetlib/libassetlib.a
.PHONY : assetlib/CMakeFiles/assetlib.dir/build

assetlib/CMakeFiles/assetlib.dir/clean:
	cd /d C:\Users\zaver\projects\VulkanEngine\assetlib && $(CMAKE_COMMAND) -P CMakeFiles\assetlib.dir\cmake_clean.cmake
.PHONY : assetlib/CMakeFiles/assetlib.dir/clean

assetlib/CMakeFiles/assetlib.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Users\zaver\projects\VulkanEngine C:\Users\zaver\projects\VulkanEngine\assetlib C:\Users\zaver\projects\VulkanEngine C:\Users\zaver\projects\VulkanEngine\assetlib C:\Users\zaver\projects\VulkanEngine\assetlib\CMakeFiles\assetlib.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : assetlib/CMakeFiles/assetlib.dir/depend

