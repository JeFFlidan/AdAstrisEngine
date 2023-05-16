cmake_minimum_required (VERSION 3.8)

project ("ecs")

set (CMAKE_CXX_STANDARD 17)
add_compile_definitions(ECS_EXPORT)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}")
set (LIBRARY_NAME ecs)
set (ENGINE_SRC_PATH ${CMAKE_SOURCE_DIR})
set (THIRD_PARTY_PATH ${CMAKE_SOURCE_DIR})

string(REPLACE "/plugins/ecs" "/src" ENGINE_SRC_PATH ${ENGINE_SRC_PATH})
string(REPLACE "/engine/plugins/ecs" "/third_party" THIRD_PARTY_PATH ${THIRD_PARTY_PATH})

file(GLOB ECS_FILES
	"${CMAKE_SOURCE_DIR}/private/*.cpp"
	"${CMAKE_SOURCE_DIR}/private/*.h"
	"${CMAKE_SOURCE_DIR}/public/*.h"
	"${CMAKE_SOURCE_DIR}/public/*.cpp"
)

add_library(${LIBRARY_NAME} SHARED ${ECS_FILES})

set_target_properties(${LIBRARY_NAME} PROPERTIES OUTPUT_NAME ${LIBRARY_NAME})

add_library(logger INTERFACE)
add_library(tuple INTERFACE)
add_library(uuid STATIC)
add_library(fmt_lib STATIC)

target_include_directories(logger INTERFACE ${ENGINE_SRC_PATH}/profiler)
target_include_directories(tuple INTERFACE ${ENGINE_SRC_PATH}/core)

target_include_directories(uuid PUBLIC ${ENGINE_SRC_PATH}/engine_core)
target_sources(uuid PRIVATE ${ENGINE_SRC_PATH}/engine_core/uuid.h ${ENGINE_SRC_PATH}/engine_core/uuid.cpp)

target_include_directories(fmt_lib PUBLIC ${THIRD_PARTY_PATH}/fmt_lib)
file(GLOB FMT_FILES "${THIRD_PARTY_PATH}/fmt_lib/fmt/*")
target_sources(fmt_lib PRIVATE ${FMT_FILES})

target_link_libraries(ecs uuid logger tuple fmt_lib)

include_directories(${CMAKE_SOURCE_DIR}/private)
include_directories(${CMAKE_SOURCE_DIR}/public)
include_directories(${ENGINE_SRC_PATH})