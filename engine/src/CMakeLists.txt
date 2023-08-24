add_library(logger INTERFACE)

# Add source to this project's executable.

file(GLOB ENGINE_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/application/*.*"
    "${CMAKE_CURRENT_SOURCE_DIR}/application_core/*.*"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/*.*"
    "${CMAKE_CURRENT_SOURCE_DIR}/engine/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/engine/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/engine_core/*.*"
    "${CMAKE_CURRENT_SOURCE_DIR}/engine_core/components/*.*"
    "${CMAKE_CURRENT_SOURCE_DIR}/engine_core/editor_settings/*.*"
    "${CMAKE_CURRENT_SOURCE_DIR}/engine_core/project_settings/*.*"
    "${CMAKE_CURRENT_SOURCE_DIR}/engine_core/level/*.*"
    "${CMAKE_CURRENT_SOURCE_DIR}/engine_core/model/*.*"
    "${CMAKE_CURRENT_SOURCE_DIR}/engine_core/texture/*.*"
    "${CMAKE_CURRENT_SOURCE_DIR}/engine_core/material/*.*"
    "${CMAKE_CURRENT_SOURCE_DIR}/file_system/*.*"
    "${CMAKE_CURRENT_SOURCE_DIR}/profiler/*.*"
    "${CMAKE_CURRENT_SOURCE_DIR}/resource_manager/*.*"
    "${CMAKE_CURRENT_SOURCE_DIR}/rhi/*.*"
    "${CMAKE_CURRENT_SOURCE_DIR}/events/*.*"
    "${CMAKE_CURRENT_SOURCE_DIR}/multithreading/*.*"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.*"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.*")
 
add_executable(AdAstrisEngine ${ENGINE_FILES})
#set_target_properties(AdAstrisEngine PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

# add_library(shaderc STATIC IMPORTED)
# set_target_properties(shaderc PROPERTIES IMPORTED_LOCATION ${DIR_THIRD_PARTY}/shaderc/lib/libshaderc_combined.a)

add_library(ecs SHARED IMPORTED)
set_target_properties(ecs PROPERTIES IMPORTED_IMPLIB ${DIR_BIN}/libecs.dll)
set_target_properties(ecs PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${DIR_PLUGINS}/ecs)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

set_property(TARGET AdAstrisEngine PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:AdAstrisEngine>")

target_include_directories(logger INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_sources(logger PRIVATE profiler/logger.h)

target_include_directories(AdAstrisEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(AdAstrisEngine vkbootstrap spirv_reflect vma glm cgltf json lz4 tinyobjloader imgui stb_image fmt_lib logger)

target_link_libraries(AdAstrisEngine Vulkan::Vulkan sdl2 assetlib tinyobjloader inicpp ecs reflection)
