cmake_minimum_required (VERSION 3.8)

project ("vulkan_rhi")

add_compile_definitions(VK_RHI_EXPORT)

set (LIBRARY_NAME vulkan_rhi)
set (ENGINE_SRC_PATH ${CMAKE_SOURCE_DIR})
set (THIRD_PARTY_PATH ${CMAKE_SOURCE_DIR})
set (BIN_PATH ${CMAKE_SOURCE_DIR})

string(REPLACE "/src/vulkan_rhi" "/src" ENGINE_SRC_PATH ${ENGINE_SRC_PATH})
string(REPLACE "/engine/src/vulkan_rhi" "/third_party" THIRD_PARTY_PATH ${THIRD_PARTY_PATH})
string(REPLACE "/engine/src/vulkan_rhi" "/bin" BIN_PATH ${BIN_PATH})

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BIN_PATH}")

file(GLOB VK_RHI_FILES
	"${CMAKE_SOURCE_DIR}/*.cpp"
	"${CMAKE_SOURCE_DIR}/*.h"
)

add_library(${LIBRARY_NAME} SHARED ${VK_RHI_FILES})
set_target_properties(${LIBRARY_NAME} PROPERTIES OUTPUT_NAME ${LIBRARY_NAME})

find_package(Vulkan REQUIRED)

add_library(vkbootstrap STATIC)
target_include_directories(vkbootstrap PUBLIC ${THIRD_PARTY_PATH}/vkbootstrap)
file(GLOB VKBOOTSTRAP_FILES "${THIRD_PARTY_PATH}/vkbootstrap/*")
target_sources(vkbootstrap PRIVATE ${VKBOOTSTRAP_FILES})

target_link_libraries(vkbootstrap PUBLIC Vulkan::Vulkan $<$<BOOL:UNIX>:${CMAKE_DL_LIBS}>)

##add_library(vma INTERFACE)
##target_include_directories(vma INTERFACE ${THIRD_PARTY_PATH}/vma)

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

add_library(sdl2 INTERFACE)
set(sdl2_DIR ${THIRD_PARTY_PATH}/SDL)

if (WIN32)
target_include_directories(sdl2 INTERFACE ${sdl2_DIR}/include ${sdl2_image_DIR}/include)
target_link_directories(sdl2 INTERFACE ${sdl2_DIR}/lib/x64 ${sdl2_image_DIR}/lib/x64)
elseif (UNIX)
target_include_directories(sdl2 INTERFACE ${sdl2_DIR}/include/SDL2 ${sdl2_image_DIR}/include/SDL2)
target_link_directories(sdl2 INTERFACE ${sdl2_DIR}/lib ${sdl2_image_DIR}/lib)
endif()

target_link_libraries(sdl2 INTERFACE SDL2main SDL2)

add_library(spirv_reflect STATIC)
target_include_directories(spirv_reflect PUBLIC ${THIRD_PARTY_PATH}/spirv_reflect)
target_sources(spirv_reflect PRIVATE
	${THIRD_PARTY_PATH}/spirv_reflect/spirv_reflect.h
	${THIRD_PARTY_PATH}/spirv_reflect/spirv_reflect.c
	)

include_directories(${ENGINE_SRC_PATH})
include_directories(${THIRD_PARTY_PATH}/vma)

target_link_libraries(vulkan_rhi vkbootstrap spirv_reflect fmt_lib Vulkan::Vulkan sdl2 file_system)

