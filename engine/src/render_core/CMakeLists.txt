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

add_library(object_name STATIC)
target_include_directories(object_name PUBLIC ${ENGINE_SRC_PATH}/engine_core ${THIRD_PARTY_PATH}/fmt_lib)
file(GLOB OBJECT_NAME_FILES
	${ENGINE_SRC_PATH}/engine_core/object_name.h
	${ENGINE_SRC_PATH}/engine_core/object_name.cpp
	${THIRD_PARTY_PATH}/fmt_lib/fmt/*)
target_sources(object_name PRIVATE ${OBJECT_NAME_FILES})

add_library(fmt_lib STATIC)
target_include_directories(fmt_lib PUBLIC ${THIRD_PARTY_PATH}/fmt_lib)
file(GLOB FMT_FILES "${THIRD_PARTY_PATH}/fmt_lib/fmt/*")
target_sources(fmt_lib PRIVATE ${FMT_FILES})

add_library(file_system STATIC)
target_include_directories(file_system PUBLIC ${ENGINE_SRC_PATH}/file_system)
file(GLOB FILE_SYSTEM_FILES
	"${ENGINE_SRC_PATH}/file_system/utils.h"
	"${ENGINE_SRC_PATH}/file_system/utils.cpp")
target_sources(file_system PRIVATE ${FILE_SYSTEM_FILES})

add_library(module_manager STATIC)
target_include_directories(module_manager PUBLIC ${ENGINE_SRC_PATH}/core ${THIRD_PARTY_PATH}/fmt_lib)
file(GLOB FILE_SYSTEM_FILES
	"${ENGINE_SRC_PATH}/core/module_manager.h"
	"${ENGINE_SRC_PATH}/core/module_manager.cpp"
	"${THIRD_PARTY_PATH}/fmt_lib/fmt/*")
target_sources(module_manager PRIVATE ${FILE_SYSTEM_FILES})

add_library(shaderc STATIC IMPORTED)
set_target_properties(shaderc PROPERTIES IMPORTED_LOCATION ${THIRD_PARTY_PATH}/shaderc/lib/libshaderc_combined.a)

include_directories(${ENGINE_SRC_PATH})
include_directories(${THIRD_PARTY_PATH}/vma)
include_directories(${THIRD_PARTY_PATH}/glm)

target_link_libraries(render_core Vulkan::Vulkan fmt_lib module_manager file_system object_name shaderc)
