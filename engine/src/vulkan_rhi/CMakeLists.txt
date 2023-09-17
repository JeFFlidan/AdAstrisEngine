add_compile_definitions(VK_RHI_EXPORT)

set (LIBRARY_NAME vulkan_rhi)

file(GLOB VK_RHI_FILES
	${DIR_ENGINE_SRC}/vulkan_rhi/*.cpp
	${DIR_ENGINE_SRC}/vulkan_rhi/*.h
)

add_library(${LIBRARY_NAME} SHARED ${VK_RHI_FILES})
set_target_properties(${LIBRARY_NAME} PROPERTIES OUTPUT_NAME ${LIBRARY_NAME})

add_library(file_system STATIC)
target_include_directories(file_system PUBLIC ${DIR_THIRD_PARTY}/fmt_lib)
file(GLOB FILE_SYSTEM_FILES
	${DIR_THIRD_PARTY}/fmt_lib/fmt/*
	${DIR_ENGINE_SRC}/file_system/utils.h
	${DIR_ENGINE_SRC}/file_system/utils.cpp
	${DIR_ENGINE_SRC}/file_system/file.*
	${DIR_THIRD_PARTY}/lz4/*)
target_sources(file_system PRIVATE ${FILE_SYSTEM_FILES})

include_directories(${DIR_THIRD_PARTY})
include_directories(${DIR_ENGINE_SRC})

target_link_libraries(${LIBRARY_NAME}
	vkbootstrap 
	spirv_reflect 
	fmt_lib 
	file_system)
