add_compile_definitions(RENDER_CORE_EXPORT)

set (LIBRARY_NAME render_core)

file(GLOB RENDER_CORE_FILES
	${DIR_ENGINE_SRC}/render_core/*.cpp
	${DIR_ENGINE_SRC}/render_core/*.h
)

add_library(${LIBRARY_NAME} SHARED ${RENDER_CORE_FILES})
set_target_properties(${LIBRARY_NAME} PROPERTIES OUTPUT_NAME ${LIBRARY_NAME})

find_package(Vulkan REQUIRED)

add_library(shaderc STATIC IMPORTED)
set_target_properties(shaderc PROPERTIES IMPORTED_LOCATION ${DIR_THIRD_PARTY}/shaderc/lib/libshaderc_combined.a)

target_link_libraries(${LIBRARY_NAME} Vulkan::Vulkan shaderc engine_core)
