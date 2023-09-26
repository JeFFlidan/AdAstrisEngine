add_compile_definitions(RENDERER_EXPORT)

set (LIBRARY_NAME renderer)

file(GLOB RENDERER_FILES
	${DIR_ENGINE_SRC}/renderer/*.cpp
	${DIR_ENGINE_SRC}/renderer/*.h
    ${DIR_ENGINE_SRC}/renderer/*/*.cpp
    ${DIR_ENGINE_SRC}/renderer/*/*.h
)

include(CMakePrintHelpers)
cmake_print_variables(RENDERER_FILES)

add_library(${LIBRARY_NAME} SHARED ${RENDERER_FILES})
set_target_properties(${LIBRARY_NAME} PROPERTIES OUTPUT_NAME ${LIBRARY_NAME})

target_link_libraries(${LIBRARY_NAME} engine_core imgui shader_interop)
target_compile_options(${LIBRARY_NAME} PRIVATE "/Zc:preprocessor")
