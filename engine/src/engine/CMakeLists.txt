add_compile_definitions(ENGINE_EXPORT)

set (LIBRARY_NAME engine)

file(GLOB ENGINE_FILES
	${DIR_ENGINE_SRC}/engine/*/*.cpp
	${DIR_ENGINE_SRC}/engine/*/*.h
	${DIR_ENGINE_SRC}/engine/*/*/*.cpp
    ${DIR_ENGINE_SRC}/engine/*/*/*.h
)

add_library(${LIBRARY_NAME} SHARED ${ENGINE_FILES})
set_target_properties(${LIBRARY_NAME} PROPERTIES OUTPUT_NAME ${LIBRARY_NAME})

target_link_libraries(${LIBRARY_NAME} engine_core)
target_compile_options(${LIBRARY_NAME} PRIVATE "/Zc:preprocessor")
