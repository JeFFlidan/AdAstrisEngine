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
CMAKE_SOURCE_DIR = E:\VulkanEngine

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = E:\VulkanEngine

# Include any dependencies generated for this target.
include src/CMakeFiles/vulkan_guide.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/CMakeFiles/vulkan_guide.dir/compiler_depend.make

# Include the progress variables for this target.
include src/CMakeFiles/vulkan_guide.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/vulkan_guide.dir/flags.make

src/CMakeFiles/vulkan_guide.dir/main.cpp.obj: src/CMakeFiles/vulkan_guide.dir/flags.make
src/CMakeFiles/vulkan_guide.dir/main.cpp.obj: src/CMakeFiles/vulkan_guide.dir/includes_CXX.rsp
src/CMakeFiles/vulkan_guide.dir/main.cpp.obj: src/main.cpp
src/CMakeFiles/vulkan_guide.dir/main.cpp.obj: src/CMakeFiles/vulkan_guide.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\VulkanEngine\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/CMakeFiles/vulkan_guide.dir/main.cpp.obj"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/vulkan_guide.dir/main.cpp.obj -MF CMakeFiles\vulkan_guide.dir\main.cpp.obj.d -o CMakeFiles\vulkan_guide.dir\main.cpp.obj -c E:\VulkanEngine\src\main.cpp

src/CMakeFiles/vulkan_guide.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/vulkan_guide.dir/main.cpp.i"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E E:\VulkanEngine\src\main.cpp > CMakeFiles\vulkan_guide.dir\main.cpp.i

src/CMakeFiles/vulkan_guide.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/vulkan_guide.dir/main.cpp.s"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S E:\VulkanEngine\src\main.cpp -o CMakeFiles\vulkan_guide.dir\main.cpp.s

src/CMakeFiles/vulkan_guide.dir/vk_engine.cpp.obj: src/CMakeFiles/vulkan_guide.dir/flags.make
src/CMakeFiles/vulkan_guide.dir/vk_engine.cpp.obj: src/CMakeFiles/vulkan_guide.dir/includes_CXX.rsp
src/CMakeFiles/vulkan_guide.dir/vk_engine.cpp.obj: src/vk_engine.cpp
src/CMakeFiles/vulkan_guide.dir/vk_engine.cpp.obj: src/CMakeFiles/vulkan_guide.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\VulkanEngine\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/CMakeFiles/vulkan_guide.dir/vk_engine.cpp.obj"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/vulkan_guide.dir/vk_engine.cpp.obj -MF CMakeFiles\vulkan_guide.dir\vk_engine.cpp.obj.d -o CMakeFiles\vulkan_guide.dir\vk_engine.cpp.obj -c E:\VulkanEngine\src\vk_engine.cpp

src/CMakeFiles/vulkan_guide.dir/vk_engine.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/vulkan_guide.dir/vk_engine.cpp.i"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E E:\VulkanEngine\src\vk_engine.cpp > CMakeFiles\vulkan_guide.dir\vk_engine.cpp.i

src/CMakeFiles/vulkan_guide.dir/vk_engine.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/vulkan_guide.dir/vk_engine.cpp.s"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S E:\VulkanEngine\src\vk_engine.cpp -o CMakeFiles\vulkan_guide.dir\vk_engine.cpp.s

src/CMakeFiles/vulkan_guide.dir/vk_initializers.cpp.obj: src/CMakeFiles/vulkan_guide.dir/flags.make
src/CMakeFiles/vulkan_guide.dir/vk_initializers.cpp.obj: src/CMakeFiles/vulkan_guide.dir/includes_CXX.rsp
src/CMakeFiles/vulkan_guide.dir/vk_initializers.cpp.obj: src/vk_initializers.cpp
src/CMakeFiles/vulkan_guide.dir/vk_initializers.cpp.obj: src/CMakeFiles/vulkan_guide.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\VulkanEngine\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object src/CMakeFiles/vulkan_guide.dir/vk_initializers.cpp.obj"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/vulkan_guide.dir/vk_initializers.cpp.obj -MF CMakeFiles\vulkan_guide.dir\vk_initializers.cpp.obj.d -o CMakeFiles\vulkan_guide.dir\vk_initializers.cpp.obj -c E:\VulkanEngine\src\vk_initializers.cpp

src/CMakeFiles/vulkan_guide.dir/vk_initializers.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/vulkan_guide.dir/vk_initializers.cpp.i"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E E:\VulkanEngine\src\vk_initializers.cpp > CMakeFiles\vulkan_guide.dir\vk_initializers.cpp.i

src/CMakeFiles/vulkan_guide.dir/vk_initializers.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/vulkan_guide.dir/vk_initializers.cpp.s"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S E:\VulkanEngine\src\vk_initializers.cpp -o CMakeFiles\vulkan_guide.dir\vk_initializers.cpp.s

src/CMakeFiles/vulkan_guide.dir/vk_mesh.cpp.obj: src/CMakeFiles/vulkan_guide.dir/flags.make
src/CMakeFiles/vulkan_guide.dir/vk_mesh.cpp.obj: src/CMakeFiles/vulkan_guide.dir/includes_CXX.rsp
src/CMakeFiles/vulkan_guide.dir/vk_mesh.cpp.obj: src/vk_mesh.cpp
src/CMakeFiles/vulkan_guide.dir/vk_mesh.cpp.obj: src/CMakeFiles/vulkan_guide.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\VulkanEngine\CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object src/CMakeFiles/vulkan_guide.dir/vk_mesh.cpp.obj"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/vulkan_guide.dir/vk_mesh.cpp.obj -MF CMakeFiles\vulkan_guide.dir\vk_mesh.cpp.obj.d -o CMakeFiles\vulkan_guide.dir\vk_mesh.cpp.obj -c E:\VulkanEngine\src\vk_mesh.cpp

src/CMakeFiles/vulkan_guide.dir/vk_mesh.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/vulkan_guide.dir/vk_mesh.cpp.i"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E E:\VulkanEngine\src\vk_mesh.cpp > CMakeFiles\vulkan_guide.dir\vk_mesh.cpp.i

src/CMakeFiles/vulkan_guide.dir/vk_mesh.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/vulkan_guide.dir/vk_mesh.cpp.s"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S E:\VulkanEngine\src\vk_mesh.cpp -o CMakeFiles\vulkan_guide.dir\vk_mesh.cpp.s

src/CMakeFiles/vulkan_guide.dir/vk_textures.cpp.obj: src/CMakeFiles/vulkan_guide.dir/flags.make
src/CMakeFiles/vulkan_guide.dir/vk_textures.cpp.obj: src/CMakeFiles/vulkan_guide.dir/includes_CXX.rsp
src/CMakeFiles/vulkan_guide.dir/vk_textures.cpp.obj: src/vk_textures.cpp
src/CMakeFiles/vulkan_guide.dir/vk_textures.cpp.obj: src/CMakeFiles/vulkan_guide.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\VulkanEngine\CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object src/CMakeFiles/vulkan_guide.dir/vk_textures.cpp.obj"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/vulkan_guide.dir/vk_textures.cpp.obj -MF CMakeFiles\vulkan_guide.dir\vk_textures.cpp.obj.d -o CMakeFiles\vulkan_guide.dir\vk_textures.cpp.obj -c E:\VulkanEngine\src\vk_textures.cpp

src/CMakeFiles/vulkan_guide.dir/vk_textures.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/vulkan_guide.dir/vk_textures.cpp.i"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E E:\VulkanEngine\src\vk_textures.cpp > CMakeFiles\vulkan_guide.dir\vk_textures.cpp.i

src/CMakeFiles/vulkan_guide.dir/vk_textures.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/vulkan_guide.dir/vk_textures.cpp.s"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S E:\VulkanEngine\src\vk_textures.cpp -o CMakeFiles\vulkan_guide.dir\vk_textures.cpp.s

src/CMakeFiles/vulkan_guide.dir/vk_descriptors.cpp.obj: src/CMakeFiles/vulkan_guide.dir/flags.make
src/CMakeFiles/vulkan_guide.dir/vk_descriptors.cpp.obj: src/CMakeFiles/vulkan_guide.dir/includes_CXX.rsp
src/CMakeFiles/vulkan_guide.dir/vk_descriptors.cpp.obj: src/vk_descriptors.cpp
src/CMakeFiles/vulkan_guide.dir/vk_descriptors.cpp.obj: src/CMakeFiles/vulkan_guide.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\VulkanEngine\CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object src/CMakeFiles/vulkan_guide.dir/vk_descriptors.cpp.obj"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/vulkan_guide.dir/vk_descriptors.cpp.obj -MF CMakeFiles\vulkan_guide.dir\vk_descriptors.cpp.obj.d -o CMakeFiles\vulkan_guide.dir\vk_descriptors.cpp.obj -c E:\VulkanEngine\src\vk_descriptors.cpp

src/CMakeFiles/vulkan_guide.dir/vk_descriptors.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/vulkan_guide.dir/vk_descriptors.cpp.i"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E E:\VulkanEngine\src\vk_descriptors.cpp > CMakeFiles\vulkan_guide.dir\vk_descriptors.cpp.i

src/CMakeFiles/vulkan_guide.dir/vk_descriptors.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/vulkan_guide.dir/vk_descriptors.cpp.s"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S E:\VulkanEngine\src\vk_descriptors.cpp -o CMakeFiles\vulkan_guide.dir\vk_descriptors.cpp.s

src/CMakeFiles/vulkan_guide.dir/vk_camera.cpp.obj: src/CMakeFiles/vulkan_guide.dir/flags.make
src/CMakeFiles/vulkan_guide.dir/vk_camera.cpp.obj: src/CMakeFiles/vulkan_guide.dir/includes_CXX.rsp
src/CMakeFiles/vulkan_guide.dir/vk_camera.cpp.obj: src/vk_camera.cpp
src/CMakeFiles/vulkan_guide.dir/vk_camera.cpp.obj: src/CMakeFiles/vulkan_guide.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\VulkanEngine\CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object src/CMakeFiles/vulkan_guide.dir/vk_camera.cpp.obj"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/vulkan_guide.dir/vk_camera.cpp.obj -MF CMakeFiles\vulkan_guide.dir\vk_camera.cpp.obj.d -o CMakeFiles\vulkan_guide.dir\vk_camera.cpp.obj -c E:\VulkanEngine\src\vk_camera.cpp

src/CMakeFiles/vulkan_guide.dir/vk_camera.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/vulkan_guide.dir/vk_camera.cpp.i"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E E:\VulkanEngine\src\vk_camera.cpp > CMakeFiles\vulkan_guide.dir\vk_camera.cpp.i

src/CMakeFiles/vulkan_guide.dir/vk_camera.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/vulkan_guide.dir/vk_camera.cpp.s"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S E:\VulkanEngine\src\vk_camera.cpp -o CMakeFiles\vulkan_guide.dir\vk_camera.cpp.s

src/CMakeFiles/vulkan_guide.dir/material_system.cpp.obj: src/CMakeFiles/vulkan_guide.dir/flags.make
src/CMakeFiles/vulkan_guide.dir/material_system.cpp.obj: src/CMakeFiles/vulkan_guide.dir/includes_CXX.rsp
src/CMakeFiles/vulkan_guide.dir/material_system.cpp.obj: src/material_system.cpp
src/CMakeFiles/vulkan_guide.dir/material_system.cpp.obj: src/CMakeFiles/vulkan_guide.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\VulkanEngine\CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object src/CMakeFiles/vulkan_guide.dir/material_system.cpp.obj"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/vulkan_guide.dir/material_system.cpp.obj -MF CMakeFiles\vulkan_guide.dir\material_system.cpp.obj.d -o CMakeFiles\vulkan_guide.dir\material_system.cpp.obj -c E:\VulkanEngine\src\material_system.cpp

src/CMakeFiles/vulkan_guide.dir/material_system.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/vulkan_guide.dir/material_system.cpp.i"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E E:\VulkanEngine\src\material_system.cpp > CMakeFiles\vulkan_guide.dir\material_system.cpp.i

src/CMakeFiles/vulkan_guide.dir/material_system.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/vulkan_guide.dir/material_system.cpp.s"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S E:\VulkanEngine\src\material_system.cpp -o CMakeFiles\vulkan_guide.dir\material_system.cpp.s

src/CMakeFiles/vulkan_guide.dir/vk_shaders.cpp.obj: src/CMakeFiles/vulkan_guide.dir/flags.make
src/CMakeFiles/vulkan_guide.dir/vk_shaders.cpp.obj: src/CMakeFiles/vulkan_guide.dir/includes_CXX.rsp
src/CMakeFiles/vulkan_guide.dir/vk_shaders.cpp.obj: src/vk_shaders.cpp
src/CMakeFiles/vulkan_guide.dir/vk_shaders.cpp.obj: src/CMakeFiles/vulkan_guide.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\VulkanEngine\CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object src/CMakeFiles/vulkan_guide.dir/vk_shaders.cpp.obj"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/vulkan_guide.dir/vk_shaders.cpp.obj -MF CMakeFiles\vulkan_guide.dir\vk_shaders.cpp.obj.d -o CMakeFiles\vulkan_guide.dir\vk_shaders.cpp.obj -c E:\VulkanEngine\src\vk_shaders.cpp

src/CMakeFiles/vulkan_guide.dir/vk_shaders.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/vulkan_guide.dir/vk_shaders.cpp.i"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E E:\VulkanEngine\src\vk_shaders.cpp > CMakeFiles\vulkan_guide.dir\vk_shaders.cpp.i

src/CMakeFiles/vulkan_guide.dir/vk_shaders.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/vulkan_guide.dir/vk_shaders.cpp.s"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S E:\VulkanEngine\src\vk_shaders.cpp -o CMakeFiles\vulkan_guide.dir\vk_shaders.cpp.s

src/CMakeFiles/vulkan_guide.dir/vk_renderpass.cpp.obj: src/CMakeFiles/vulkan_guide.dir/flags.make
src/CMakeFiles/vulkan_guide.dir/vk_renderpass.cpp.obj: src/CMakeFiles/vulkan_guide.dir/includes_CXX.rsp
src/CMakeFiles/vulkan_guide.dir/vk_renderpass.cpp.obj: src/vk_renderpass.cpp
src/CMakeFiles/vulkan_guide.dir/vk_renderpass.cpp.obj: src/CMakeFiles/vulkan_guide.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\VulkanEngine\CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building CXX object src/CMakeFiles/vulkan_guide.dir/vk_renderpass.cpp.obj"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/vulkan_guide.dir/vk_renderpass.cpp.obj -MF CMakeFiles\vulkan_guide.dir\vk_renderpass.cpp.obj.d -o CMakeFiles\vulkan_guide.dir\vk_renderpass.cpp.obj -c E:\VulkanEngine\src\vk_renderpass.cpp

src/CMakeFiles/vulkan_guide.dir/vk_renderpass.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/vulkan_guide.dir/vk_renderpass.cpp.i"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E E:\VulkanEngine\src\vk_renderpass.cpp > CMakeFiles\vulkan_guide.dir\vk_renderpass.cpp.i

src/CMakeFiles/vulkan_guide.dir/vk_renderpass.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/vulkan_guide.dir/vk_renderpass.cpp.s"
	cd /d E:\VulkanEngine\src && C:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S E:\VulkanEngine\src\vk_renderpass.cpp -o CMakeFiles\vulkan_guide.dir\vk_renderpass.cpp.s

# Object files for target vulkan_guide
vulkan_guide_OBJECTS = \
"CMakeFiles/vulkan_guide.dir/main.cpp.obj" \
"CMakeFiles/vulkan_guide.dir/vk_engine.cpp.obj" \
"CMakeFiles/vulkan_guide.dir/vk_initializers.cpp.obj" \
"CMakeFiles/vulkan_guide.dir/vk_mesh.cpp.obj" \
"CMakeFiles/vulkan_guide.dir/vk_textures.cpp.obj" \
"CMakeFiles/vulkan_guide.dir/vk_descriptors.cpp.obj" \
"CMakeFiles/vulkan_guide.dir/vk_camera.cpp.obj" \
"CMakeFiles/vulkan_guide.dir/material_system.cpp.obj" \
"CMakeFiles/vulkan_guide.dir/vk_shaders.cpp.obj" \
"CMakeFiles/vulkan_guide.dir/vk_renderpass.cpp.obj"

# External object files for target vulkan_guide
vulkan_guide_EXTERNAL_OBJECTS =

bin/vulkan_guide.exe: src/CMakeFiles/vulkan_guide.dir/main.cpp.obj
bin/vulkan_guide.exe: src/CMakeFiles/vulkan_guide.dir/vk_engine.cpp.obj
bin/vulkan_guide.exe: src/CMakeFiles/vulkan_guide.dir/vk_initializers.cpp.obj
bin/vulkan_guide.exe: src/CMakeFiles/vulkan_guide.dir/vk_mesh.cpp.obj
bin/vulkan_guide.exe: src/CMakeFiles/vulkan_guide.dir/vk_textures.cpp.obj
bin/vulkan_guide.exe: src/CMakeFiles/vulkan_guide.dir/vk_descriptors.cpp.obj
bin/vulkan_guide.exe: src/CMakeFiles/vulkan_guide.dir/vk_camera.cpp.obj
bin/vulkan_guide.exe: src/CMakeFiles/vulkan_guide.dir/material_system.cpp.obj
bin/vulkan_guide.exe: src/CMakeFiles/vulkan_guide.dir/vk_shaders.cpp.obj
bin/vulkan_guide.exe: src/CMakeFiles/vulkan_guide.dir/vk_renderpass.cpp.obj
bin/vulkan_guide.exe: src/CMakeFiles/vulkan_guide.dir/build.make
bin/vulkan_guide.exe: third_party/libvkbootstrap.a
bin/vulkan_guide.exe: third_party/libtinyobjloader.a
bin/vulkan_guide.exe: third_party/libimgui.a
bin/vulkan_guide.exe: third_party/libfmt_lib.a
bin/vulkan_guide.exe: third_party/libspirv_reflect.a
bin/vulkan_guide.exe: C:/VulkanSDK/1.3.216.0/Lib/vulkan-1.lib
bin/vulkan_guide.exe: assetlib/libassetlib.a
bin/vulkan_guide.exe: third_party/liblz4.a
bin/vulkan_guide.exe: src/CMakeFiles/vulkan_guide.dir/linklibs.rsp
bin/vulkan_guide.exe: src/CMakeFiles/vulkan_guide.dir/objects1.rsp
bin/vulkan_guide.exe: src/CMakeFiles/vulkan_guide.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=E:\VulkanEngine\CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Linking CXX executable ..\bin\vulkan_guide.exe"
	cd /d E:\VulkanEngine\src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\vulkan_guide.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/CMakeFiles/vulkan_guide.dir/build: bin/vulkan_guide.exe
.PHONY : src/CMakeFiles/vulkan_guide.dir/build

src/CMakeFiles/vulkan_guide.dir/clean:
	cd /d E:\VulkanEngine\src && $(CMAKE_COMMAND) -P CMakeFiles\vulkan_guide.dir\cmake_clean.cmake
.PHONY : src/CMakeFiles/vulkan_guide.dir/clean

src/CMakeFiles/vulkan_guide.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" E:\VulkanEngine E:\VulkanEngine\src E:\VulkanEngine E:\VulkanEngine\src E:\VulkanEngine\src\CMakeFiles\vulkan_guide.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/vulkan_guide.dir/depend

