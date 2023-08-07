cmake_minimum_required (VERSION 3.8)

project ("project_launcher")

add_compile_definitions(PROJECT_LAUNCHER_EXPORT)

set (LIBRARY_NAME project_launcher)
set (ENGINE_SRC_PATH ${CMAKE_SOURCE_DIR})
set (THIRD_PARTY_PATH ${CMAKE_SOURCE_DIR})
set (BIN_PATH ${CMAKE_SOURCE_DIR})
set (GLOBAL_BUILD_PATH ${CMAKE_SOURCE_DIR})

string(REPLACE "/devtools/project_launcher" "/src" ENGINE_SRC_PATH ${ENGINE_SRC_PATH})
string(REPLACE "/engine/devtools/project_launcher" "/third_party" THIRD_PARTY_PATH ${THIRD_PARTY_PATH})
string(REPLACE "/engine/devtools/project_launcher" "/bin" BIN_PATH ${BIN_PATH})
string(REPLACE "/engine/devtools/project_launcher" "/build" GLOBAL_BUILD_PATH ${GLOBAL_BUILD_PATH})

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BIN_PATH}")

file(GLOB PROJECT_LAUNCHER_FILES
    ${ENGINE_SRC_PATH}/ui_core/*    
	${CMAKE_SOURCE_DIR}/*.cpp
	${CMAKE_SOURCE_DIR}/*.h
    ${THIRD_PARTY_PATH}/glad/src/glad.c
    ${ENGINE_SRC_PATH}/application_core/glfw_window/glfw_window.*
)

add_library(${LIBRARY_NAME} SHARED ${PROJECT_LAUNCHER_FILES})
set_target_properties(${LIBRARY_NAME} PROPERTIES OUTPUT_NAME ${LIBRARY_NAME})

add_library(fmt_lib STATIC IMPORTED)
set_target_properties(fmt_lib PROPERTIES IMPORTED_LOCATION ${GLOBAL_BUILD_PATH}/third_party/libfmt_lib.a)

add_library(imgui STATIC IMPORTED)
set_target_properties(imgui PROPERTIES IMPORTED_LOCATION ${GLOBAL_BUILD_PATH}/third_party/libimgui.a)

add_library(core STATIC)
target_include_directories(core PUBLIC ${ENGINE_SRC_PATH}/core ${THIRD_PARTY_PATH}/fmt_lib)
file(GLOB CORE_FILES
	${ENGINE_SRC_PATH}/core/config_base.*
	${ENGINE_SRC_PATH}/core/module_manager.h
	${ENGINE_SRC_PATH}/core/module_manager.cpp
	${ENGINE_SRC_PATH}/file_system/utils.*
	${THIRD_PARTY_PATH}/fmt_lib/fmt/*
	${THIRD_PARTY_PATH}/inicpp/*)
target_sources(core PRIVATE ${CORE_FILES})

add_library(glfw STATIC IMPORTED)
set_target_properties(glfw PROPERTIES IMPORTED_LOCATION ${THIRD_PARTY_PATH}/glfw/libglfw3.a)

include_directories(${ENGINE_SRC_PATH})
include_directories(${THIRD_PARTY_PATH}/inicpp)
include_directories(${THIRD_PARTY_PATH}/glfw)
include_directories(${THIRD_PARTY_PATH}/glad)
include_directories(${THIRD_PARTY_PATH}/imgui)
include_directories(${THIRD_PARTY_PATH}/stb_image)

target_link_libraries(project_launcher fmt_lib core glfw imgui)
