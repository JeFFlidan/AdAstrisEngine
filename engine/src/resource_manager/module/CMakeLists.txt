add_compile_definitions(RESOURCE_MANAGER_API)

set(LIBRARY_NAME resource_manager)

file(GLOB RESOURCE_MANAGER_FILES
    ${DIR_ENGINE_SRC}/resource_manager/*.cpp
    ${DIR_ENGINE_SRC}/resource_manager/*.h
    ${DIR_ENGINE_SRC}/resource_manager/module/*.cpp
    ${DIR_ENGINE_SRC}/resource_manager/module/*.h
)

add_library(${LIBRARY_NAME} SHARED ${RESOURCE_MANAGER_FILES})
set_target_properties(${LIBRARY_NAME} PROPERTIES OUTPUT_NAME ${LIBRARY_NAME})

include_directories(${DIR_THIRD_PARTY})
include_directories(${DIR_ENGINE_SRC})
 
target_link_libraries(${LIBRARY_NAME}
    engine_core
    basisu)