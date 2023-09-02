cmake_minimum_required (VERSION 3.8)

project ("renderer")

add_compile_definitions(RENDERER_EXPORT)

set (LIBRARY_NAME renderer)
set (ENGINE_SRC_PATH ${CMAKE_SOURCE_DIR})
set (PLUGINS_PATH ${CMAKE_SOURCE_DIR})
set (THIRD_PARTY_PATH ${CMAKE_SOURCE_DIR})
set (BIN_PATH ${CMAKE_SOURCE_DIR})

string(REPLACE "/src/renderer" "/src" ENGINE_SRC_PATH ${ENGINE_SRC_PATH})
string(REPLACE "/src/renderer" "/plugins" PLUGINS_PATH ${PLUGINS_PATH})
string(REPLACE "/engine/src/renderer" "/third_party" THIRD_PARTY_PATH ${THIRD_PARTY_PATH})
string(REPLACE "/engine/src/renderer" "/bin" BIN_PATH ${BIN_PATH})

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BIN_PATH}")

file(GLOB RENDERER_FILES
	"${CMAKE_SOURCE_DIR}/*.cpp"
	"${CMAKE_SOURCE_DIR}/*.h"
)

find_package(Vulkan REQUIRED) ## I have to remove it later after finishing changing engine architecture

add_library(${LIBRARY_NAME} SHARED ${RENDERER_FILES})
set_target_properties(${LIBRARY_NAME} PROPERTIES OUTPUT_NAME ${LIBRARY_NAME})

add_library(ecs SHARED IMPORTED)
set_target_properties(ecs PROPERTIES IMPORTED_IMPLIB ${BIN_PATH}/libecs.dll)
set_target_properties(ecs PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${PLUGINS_PATH}/ecs)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

add_library(engine_libs STATIC)
target_link_libraries(engine_libs ecs)
target_include_directories(engine_libs PUBLIC
    ${ENGINE_SRC_PATH}/resource_manager
    ${ENGINE_SRC_PATH}/core
    ${THIRD_PARTY_PATH}/fmt_lib
    ${THIRD_PARTY_PATH}/inicpp
    ${THIRD_PARTY_PATH}/json
    ${THIRD_PARTY_PATH}/glm
    ${THIRD_PARTY_PATH}/lz4
    ${THIRD_PARTY_PATH}/cgltf
    ${THIRD_PARTY_PATH}/stb_image
    ${THIRD_PARTY_PATH}/tinyobjloader
)
file(GLOB ENGINE_LIBS_FILES
    ${ENGINE_SRC_PATH}/events/*
    ${ENGINE_SRC_PATH}/multithreading/*
    ${ENGINE_SRC_PATH}/resource_manager/*
    ${ENGINE_SRC_PATH}/engine_core/model/*
    ${ENGINE_SRC_PATH}/engine_core/texture/*
    ${ENGINE_SRC_PATH}/engine_core/material/*
    ${ENGINE_SRC_PATH}/engine_core/level/*
    ${ENGINE_SRC_PATH}/engine_core/object.*
    ${ENGINE_SRC_PATH}/engine_core/object_name.*
    ${ENGINE_SRC_PATH}/engine_core/uuid.*
    ${ENGINE_SRC_PATH}/engine_core/fwd.h
    ${ENGINE_SRC_PATH}/file_system/*
    ${ENGINE_SRC_PATH}/core/*
    #${THIRD_PARTY_PATH}/fmt_lib/fmt/*
    ${THIRD_PARTY_PATH}/inicpp/*
    ${THIRD_PARTY_PATH}/json/*
    ${THIRD_PARTY_PATH}/glm/*
    ${THIRD_PARTY_PATH}/lz4/*
    ${THIRD_PARTY_PATH}/cgltf/*
    ${THIRD_PARTY_PATH}/stb_image/*
    ${THIRD_PARTY_PATH}/tinyobjloader/*
    ${THIRD_PARTY_PATH}/reflection/*
)
target_sources(engine_libs PRIVATE ${ENGINE_LIBS_FILES})

add_library(fmt_lib STATIC)
target_include_directories(fmt_lib PUBLIC ${THIRD_PARTY_PATH}/fmt_lib)
file(GLOB FMT_FILES ${THIRD_PARTY_PATH}/fmt_lib/fmt/*)
target_sources(fmt_lib PRIVATE ${FMT_FILES})

include_directories(${ENGINE_SRC_PATH})
include_directories(${THIRD_PARTY_PATH}/inicpp)
include_directories(${THIRD_PARTY_PATH}/vma)
include_directories(${THIRD_PARTY_PATH}/reflection)

target_link_libraries(renderer Vulkan::Vulkan fmt_lib engine_libs)
