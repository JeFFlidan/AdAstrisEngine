add_compile_definitions(PROJECT_LAUNCHER_EXPORT)

set (LIBRARY_NAME project_launcher)

file(GLOB PROJECT_LAUNCHER_FILES
    ${DIR_DEVTOOLS}/project_launcher/*.cpp
    ${DIR_DEVTOOLS}/project_launcher/*.h
    ${DIR_ENGINE_SRC}/application_core/glfw_window/glfw_window.*
)

add_library(${LIBRARY_NAME} SHARED ${PROJECT_LAUNCHER_FILES})
set_target_properties(${LIBRARY_NAME} PROPERTIES OUTPUT_NAME ${LIBRARY_NAME})

add_library(glfw STATIC IMPORTED)
set_target_properties(glfw PROPERTIES IMPORTED_LOCATION ${DIR_THIRD_PARTY}/glfw/glfw3.lib)

include_directories(${DIR_ENGINE_SRC})
include_directories(${DIR_THIRD_PARTY})

target_link_libraries(project_launcher engine_core glfw imgui glad)
