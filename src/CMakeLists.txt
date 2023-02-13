add_library(logger INTERFACE)

# Add source to this project's executable.

file(GLOB ENGINE_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
 
add_executable(AdAstrisEngine ${ENGINE_FILES})

set_property(TARGET AdAstrisEngine PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:AdAstrisEngine>")

target_include_directories(logger INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_sources(logger PRIVATE profiler/logger.h)
target_include_directories(AdAstrisEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(AdAstrisEngine vkbootstrap vma glm tinyobjloader imgui stb_image fmt_lib logger spirv_reflect)

target_link_libraries(AdAstrisEngine Vulkan::Vulkan sdl2 assetlib)

add_dependencies(AdAstrisEngine Shaders)
