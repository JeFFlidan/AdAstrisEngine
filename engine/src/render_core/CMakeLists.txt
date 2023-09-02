cmake_minimum_required (VERSION 3.8)

project ("render_core")

add_compile_definitions(RENDER_CORE_EXPORT)

set (LIBRARY_NAME render_core)
set (ENGINE_SRC_PATH ${CMAKE_SOURCE_DIR})
set (THIRD_PARTY_PATH ${CMAKE_SOURCE_DIR})
set (BIN_PATH ${CMAKE_SOURCE_DIR})

string(REPLACE "/src/render_core" "/src" ENGINE_SRC_PATH ${ENGINE_SRC_PATH})
string(REPLACE "/engine/src/render_core" "/third_party" THIRD_PARTY_PATH ${THIRD_PARTY_PATH})
string(REPLACE "/engine/src/render_core" "/bin" BIN_PATH ${BIN_PATH})

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BIN_PATH}")

file(GLOB RENDER_CORE_FILES
	"${CMAKE_SOURCE_DIR}/*.cpp"
	"${CMAKE_SOURCE_DIR}/*.h"
)

add_library(${LIBRARY_NAME} SHARED ${RENDER_CORE_FILES})
set_target_properties(${LIBRARY_NAME} PROPERTIES OUTPUT_NAME ${LIBRARY_NAME})

find_package(Vulkan REQUIRED) ## TESTS

add_library(fmt_lib STATIC)
target_include_directories(fmt_lib PUBLIC ${THIRD_PARTY_PATH}/fmt_lib)
file(GLOB FMT_FILES "${THIRD_PARTY_PATH}/fmt_lib/fmt/*")
target_sources(fmt_lib PRIVATE ${FMT_FILES})

add_library(engine_libs STATIC)
target_include_directories(engine_libs PUBLIC
    ${THIRD_PARTY_PATH}/fmt_lib
    ${THIRD_PARTY_PATH}/inicpp
)
file(GLOB ENGINE_LIBS_FILES
    ${ENGINE_SRC_PATH}/events/*
    ${ENGINE_SRC_PATH}/multithreading/*
    ${ENGINE_SRC_PATH}/engine_core/object_name.*
    ${ENGINE_SRC_PATH}/file_system/utils.*
    ${ENGINE_SRC_PATH}/core/module_manager.*
	${ENGINE_SRC_PATH}/core/pool_allocator.*
    #${THIRD_PARTY_PATH}/fmt_lib/fmt/*
    ${THIRD_PARTY_PATH}/inicpp/*
)
target_sources(engine_libs PRIVATE ${ENGINE_LIBS_FILES})

add_library(shaderc STATIC IMPORTED)
set_target_properties(shaderc PROPERTIES IMPORTED_LOCATION ${THIRD_PARTY_PATH}/shaderc/lib/libshaderc_combined.a)

include_directories(${ENGINE_SRC_PATH})
include_directories(${THIRD_PARTY_PATH}/vma)
include_directories(${THIRD_PARTY_PATH}/glm)
include_directories(${THIRD_PARTY_PATH}/inicpp)

target_link_libraries(render_core Vulkan::Vulkan fmt_lib engine_libs shaderc)
