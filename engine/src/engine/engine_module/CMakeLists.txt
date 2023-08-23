cmake_minimum_required (VERSION 3.8)

project ("engine")

add_compile_definitions(ENGINE_EXPORT)

set (LIBRARY_NAME engine)
set (ENGINE_SRC_PATH ${CMAKE_SOURCE_DIR})
set (PLUGINS_PATH ${CMAKE_SOURCE_DIR})
set (THIRD_PARTY_PATH ${CMAKE_SOURCE_DIR})
set (BIN_PATH ${CMAKE_SOURCE_DIR})

string(REPLACE "/src/engine/engine_module" "/src" ENGINE_SRC_PATH ${ENGINE_SRC_PATH})
string(REPLACE "/src/engine/engine_module" "/plugins" PLUGINS_PATH ${PLUGINS_PATH})
string(REPLACE "/engine/src/engine/engine_module" "/third_party" THIRD_PARTY_PATH ${THIRD_PARTY_PATH})
string(REPLACE "/engine/src/engine/engine_module" "/bin" BIN_PATH ${BIN_PATH})

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BIN_PATH}")

file(GLOB ENGINE_FILES
	"${CMAKE_SOURCE_DIR}/*.cpp"
	"${CMAKE_SOURCE_DIR}/*.h"
)

find_package(Vulkan REQUIRED) ## I have to remove it later after finishing changing engine architecture

add_library(${LIBRARY_NAME} SHARED ${ENGINE_FILES})
set_target_properties(${LIBRARY_NAME} PROPERTIES OUTPUT_NAME ${LIBRARY_NAME})

add_library(ecs SHARED IMPORTED)
set_target_properties(ecs PROPERTIES IMPORTED_IMPLIB ${BIN_PATH}/libecs.dll)
set_target_properties(ecs PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${PLUGINS_PATH}/ecs)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

add_library(core_libs STATIC)
target_link_libraries(core_libs ecs)
target_include_directories(core_libs PUBLIC
    ${ENGINE_SRC_PATH}/resource_manager
    ${ENGINE_SRC_PATH}/core
    ${THIRD_PARTY_PATH}/fmt_lib
    ${THIRD_PARTY_PATH}/inicpp
    ${THIRD_PARTY_PATH}/json
    ${THIRD_PARTY_PATH}/glm
    ${THIRD_PARTY_PATH}/lz4
    ${THIRD_PARTY_PATH}/cgltf
    ${THIRD_PARTY_PATH}/stb_image
    ${THIRD_PARTY_PATH}/reflection
    ${THIRD_PARTY_PATH}/tinyobjloader
)
file(GLOB ENGINE_LIBS_FILES
    ${ENGINE_SRC_PATH}/resource_manager/*
    ${ENGINE_SRC_PATH}/multithreading/*
    ${ENGINE_SRC_PATH}/engine_core/model/*
    ${ENGINE_SRC_PATH}/engine_core/project_settings/*
    ${ENGINE_SRC_PATH}/engine_core/texture/*
    ${ENGINE_SRC_PATH}/engine_core/material/*
    ${ENGINE_SRC_PATH}/engine_core/level/*
    ${ENGINE_SRC_PATH}/engine_core/object.*
    ${ENGINE_SRC_PATH}/engine_core/object_name.*
    ${ENGINE_SRC_PATH}/engine_core/uuid.*
    ${ENGINE_SRC_PATH}/engine_core/fwd.h
    ${ENGINE_SRC_PATH}/engine_core/base_settings.*
    ${ENGINE_SRC_PATH}/engine_core/basic_components.*
    ${ENGINE_SRC_PATH}/engine_core/contexts.*
    ${ENGINE_SRC_PATH}/engine_core/engine_objects_creator.*
    ${ENGINE_SRC_PATH}/engine_core/object.*
    ${ENGINE_SRC_PATH}/engine_core/object_name.*
    ${ENGINE_SRC_PATH}/engine_core/subsettings.*
    ${ENGINE_SRC_PATH}/engine_core/uuid.*
    ${ENGINE_SRC_PATH}/engine_core/world.*
    ${ENGINE_SRC_PATH}/core/*
    ${ENGINE_SRC_PATH}/file_system/utils.*
    ${ENGINE_SRC_PATH}/file_system/file.*
    ${ENGINE_SRC_PATH}/events/*
    ${THIRD_PARTY_PATH}/inicpp/*
    ${THIRD_PARTY_PATH}/json/*
    ${THIRD_PARTY_PATH}/glm/*
    ${THIRD_PARTY_PATH}/lz4/*
    ${THIRD_PARTY_PATH}/cgltf/*
    ${THIRD_PARTY_PATH}/stb_image/*
    ${THIRD_PARTY_PATH}/reflection/*
    ${THIRD_PARTY_PATH}/tinyobjloader/*
)
target_sources(core_libs PRIVATE ${ENGINE_LIBS_FILES})

# add_library(module_manager STATIC)
# target_include_directories(module_manager PUBLIC 
#     ${ENGINE_SRC_PATH}/core 
#     ${THIRD_PARTY_PATH}/fmt_lib
# )
# file(GLOB FILE_SYSTEM_FILES
# 	${ENGINE_SRC_PATH}/core/module_manager.h
# 	${ENGINE_SRC_PATH}/core/module_manager.cpp
# 	${THIRD_PARTY_PATH}/fmt_lib/fmt/*
# 	${THIRD_PARTY_PATH}/inicpp/*)
# target_sources(module_manager PRIVATE ${FILE_SYSTEM_FILES})

add_library(fmt_lib STATIC)
target_include_directories(fmt_lib PUBLIC ${THIRD_PARTY_PATH}/fmt_lib)
file(GLOB FMT_FILES ${THIRD_PARTY_PATH}/fmt_lib/fmt/*)
target_sources(fmt_lib PRIVATE ${FMT_FILES})

include_directories(${ENGINE_SRC_PATH})
include_directories(${THIRD_PARTY_PATH}/inicpp)
include_directories(${THIRD_PARTY_PATH}/vma)
include_directories(${THIRD_PARTY_PATH}/reflection)

target_link_libraries(engine Vulkan::Vulkan core_libs fmt_lib)
