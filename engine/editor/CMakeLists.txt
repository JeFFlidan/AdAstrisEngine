add_compile_definitions(EDITOR_EXPORT)

set (LIBRARY_NAME editor)

file(GLOB EDITOR_FILES
	*.cpp
	*.h
	*/*.*
)

add_library(${LIBRARY_NAME} SHARED ${EDITOR_FILES})
set_target_properties(${LIBRARY_NAME} PROPERTIES OUTPUT_NAME ${LIBRARY_NAME})

include_directories(${DIR_THIRD_PARTY})

target_link_libraries(${LIBRARY_NAME} engine_core)
target_compile_options(${LIBRARY_NAME} PRIVATE "/Zc:preprocessor")
