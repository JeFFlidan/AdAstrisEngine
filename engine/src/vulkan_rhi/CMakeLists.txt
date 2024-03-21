add_compile_definitions(VK_RHI_EXPORT)

set (LIBRARY_NAME vulkan_rhi)

file(GLOB VK_RHI_FILES
	${DIR_ENGINE_SRC}/vulkan_rhi/*/*.cpp
	${DIR_ENGINE_SRC}/vulkan_rhi/*/*.h
	${DIR_ENGINE_SRC}/vulkan_rhi/*/*/*.cpp
    ${DIR_ENGINE_SRC}/vulkan_rhi/*/*/*.h
)

add_library(${LIBRARY_NAME} SHARED ${VK_RHI_FILES})
set_target_properties(${LIBRARY_NAME} PROPERTIES OUTPUT_NAME ${LIBRARY_NAME})

include_directories(${DIR_THIRD_PARTY})
include_directories(${DIR_ENGINE_SRC})

target_link_libraries(${LIBRARY_NAME}
	volk
	spirv_reflect 
	engine_core)

target_include_directories(${LIBRARY_NAME} PUBLIC ${DIR_THIRD_PARTY}/vulkan_base)
