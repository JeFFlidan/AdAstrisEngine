# CMakeList.txt : CMake project for AdAstrisEngine, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_COMPILER C:/msys64/mingw64/bin/g++.exe)
set(DIR_THIRD_PARTY ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
set(DIR_PLUGINS ${CMAKE_CURRENT_SOURCE_DIR}/engine/plugins)
set(DIR_BIN ${CMAKE_CURRENT_SOURCE_DIR}/bin)

project ("AdAstrisEngine")

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)

add_subdirectory(third_party)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

add_subdirectory(engine/src)
add_subdirectory(assetlib)
add_subdirectory(asset_baker)

find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin ${DIR_THIRD_PARTY}/vulkan_base/glslang)
    
link_directories("third_party/shaderc/lib")
link_directories("engine/plugins/ecs")
include_directories("third_party/shaderc/include")
