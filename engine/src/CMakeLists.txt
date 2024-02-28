add_library(engine_core STATIC)
file(GLOB ENGINE_CORE_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/application/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/application_core/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/core/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/core/*/*
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
    ${DIR_ENGINE_SRC}/ui_core/*
    ${DIR_ENGINE_SRC}/ui_core/*/*)
target_sources(engine_core PRIVATE ${ENGINE_CORE_FILES})

add_library(nfd STATIC IMPORTED)
set_target_properties(nfd PROPERTIES IMPORTED_LOCATION ${DIR_THIRD_PARTY}/nfd/nfd.lib)

target_link_libraries(engine_core
    nfd
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
target_compile_options(engine_core PRIVATE "/Zc:preprocessor")

add_executable(AdAstrisEngine main.cpp)
target_include_directories(AdAstrisEngine PUBLIC ${DIR_ENGINE_SRC})
target_link_libraries(AdAstrisEngine engine_core)
