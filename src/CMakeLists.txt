add_library(logger INTERFACE)

# Add source to this project's executable.

file(GLOB ENGINE_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ui/*.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/ui/*.h")
 

add_executable(vulkan_guide ${ENGINE_FILES})

set_property(TARGET vulkan_guide PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:vulkan_guide>")

target_include_directories(logger INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_sources(logger PRIVATE logger.h)
target_include_directories(vulkan_guide PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(vulkan_guide vkbootstrap vma glm tinyobjloader imgui stb_image fmt_lib logger spirv_reflect)

target_link_libraries(vulkan_guide Vulkan::Vulkan sdl2 assetlib)

add_dependencies(vulkan_guide Shaders)
