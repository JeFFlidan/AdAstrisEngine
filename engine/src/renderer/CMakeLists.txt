add_compile_definitions(RENDERER_EXPORT)

set (LIBRARY_NAME renderer)

file(GLOB RENDERER_FILES
	${DIR_ENGINE_SRC}/renderer/*.cpp
	${DIR_ENGINE_SRC}/renderer/*.h
    ${DIR_ENGINE_SRC}/renderer/*/*.cpp
    ${DIR_ENGINE_SRC}/renderer/*/*.h
)

add_library(${LIBRARY_NAME} SHARED ${RENDERER_FILES})
set_target_properties(${LIBRARY_NAME} PROPERTIES OUTPUT_NAME ${LIBRARY_NAME})

target_link_libraries(${LIBRARY_NAME} engine_core imgui)
target_compile_options(${LIBRARY_NAME} PRIVATE "/Zc:preprocessor")