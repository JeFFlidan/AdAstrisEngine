add_compile_definitions(PROJECT_LAUNCHER_EXPORT)

set (LIBRARY_NAME project_launcher)

file(GLOB PROJECT_LAUNCHER_FILES
    ${DIR_ENGINE_SRC}/ui_core/*
    ${DIR_ENGINE_SRC}/ui_core/*/*
    ${DIR_DEVTOOLS}/project_launcher/*.cpp
    ${DIR_DEVTOOLS}/project_launcher/*.h
    ${DIR_ENGINE_SRC}/application_core/glfw_window/glfw_window.*
)

add_library(${LIBRARY_NAME} SHARED ${PROJECT_LAUNCHER_FILES})
set_target_properties(${LIBRARY_NAME} PROPERTIES OUTPUT_NAME ${LIBRARY_NAME})

add_library(core STATIC)
target_include_directories(core PUBLIC ${DIR_ENGINE_SRC}/core ${DIR_THIRD_PARTY}/fmt_lib)
file(GLOB CORE_FILES
	${DIR_ENGINE_SRC}/core/config_base.*
	${DIR_ENGINE_SRC}/core/module_manager.h
	${DIR_ENGINE_SRC}/core/module_manager.cpp
	${DIR_ENGINE_SRC}/file_system/utils.*
	${DIR_ENGINE_SRC}/file_system/file.*
	${DIR_THIRD_PARTY}/fmt_lib/fmt/*
	${DIR_THIRD_PARTY}/lz4/*
	${DIR_THIRD_PARTY}/inicpp/*)
target_sources(core PRIVATE ${CORE_FILES})

add_library(glfw STATIC IMPORTED)
set_target_properties(glfw PROPERTIES IMPORTED_LOCATION ${DIR_THIRD_PARTY}/glfw/glfw3.lib)

add_library(nfd STATIC IMPORTED)
set_target_properties(nfd PROPERTIES IMPORTED_LOCATION ${DIR_THIRD_PARTY}/nfd/nfd.lib)

include_directories(${DIR_ENGINE_SRC})
include_directories(${DIR_THIRD_PARTY})

target_link_libraries(project_launcher fmt_lib core glfw imgui nfd glad)
