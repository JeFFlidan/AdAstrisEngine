add_compile_definitions(RENDER_CORE_EXPORT)

set (LIBRARY_NAME render_core)

file(GLOB RENDER_CORE_FILES
	${DIR_ENGINE_SRC}/render_core/*.cpp
	${DIR_ENGINE_SRC}/render_core/*.h
	${DIR_ENGINE_SRC}/render_core/shader_compiler/*.*
)

add_library(${LIBRARY_NAME} SHARED ${RENDER_CORE_FILES})
set_target_properties(${LIBRARY_NAME} PROPERTIES OUTPUT_NAME ${LIBRARY_NAME})

find_package(Vulkan REQUIRED)

target_link_libraries(${LIBRARY_NAME} Vulkan::Vulkan engine_core)
