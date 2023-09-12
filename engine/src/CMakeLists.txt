add_library(engine_core STATIC)
file(GLOB ENGINE_CORE_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/application/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/application_core/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/core/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/engine_core/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/engine_core/components/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/engine_core/editor_settings/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/engine_core/project_settings/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/engine_core/level/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/engine_core/model/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/engine_core/texture/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/engine_core/material/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/ecs/*/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/ecs/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/file_system/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/profiler/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/resource_manager/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/rhi/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/events/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/multithreading/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.*)
target_sources(engine_core PRIVATE ${ENGINE_CORE_FILES})

target_link_libraries(engine_core
    tinyobjloader 
    inicpp 
    reflection
    glm
    cgltf
    json
    lz4
    tinyobjloader 
    imgui 
    stb_image
    fmt_lib)
target_include_directories(engine_core PUBLIC ${DIR_ENGINE_SRC})
target_include_directories(engine_core PUBLIC ${DIR_THIRD_PARTY})

add_executable(AdAstrisEngine main.cpp)
target_include_directories(AdAstrisEngine PUBLIC ${DIR_ENGINE_SRC})
target_link_libraries(AdAstrisEngine engine_core)
