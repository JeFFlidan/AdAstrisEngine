add_library(logger INTERFACE)

# Add source to this project's executable.

file(GLOB ENGINE_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
 
add_executable(AdAstrisEngine ${ENGINE_FILES})
#set_target_properties(AdAstrisEngine PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

add_library(shaderc STATIC IMPORTED)

#if (WIN32)
#	set_target_properties(shaderc PROPERTIES IMPORTED_LOCATION_RELEASE E:/MyEngine/MyEngine/VulkanEngine/third_party/shaderc/lib/shaderc_combined.lib)
#	set_target_properties(shaderc PROPERTIES IMPORTED_LOCATION_MINSIZEREL E:/MyEngine/MyEngine/VulkanEngine/third_party/shaderc/lib/shaderc_combined.lib)
#	set_target_properties(shaderc PROPERTIES IMPORTED_LOCATION_RELWITHDEBINFO E:/MyEngine/MyEngine/VulkanEngine/third_party/shaderc/lib/shaderc_combined.lib)
#	set_target_properties(shaderc PROPERTIES IMPORTED_LOCATION_DEBUG E:/MyEngine/MyEngine/VulkanEngine/third_party/shaderc/lib/shaderc_combined.lib)
#endif()
set_target_properties(shaderc PROPERTIES IMPORTED_LOCATION E:/MyEngine/MyEngine/VulkanEngine/third_party/shaderc/lib/libshaderc_combined.a)

set_property(TARGET AdAstrisEngine PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:AdAstrisEngine>")

target_include_directories(logger INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_sources(logger PRIVATE profiler/logger.h)
target_include_directories(AdAstrisEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(AdAstrisEngine vkbootstrap spirv_reflect vma glm tinyobjloader imgui stb_image fmt_lib logger)

target_link_libraries(AdAstrisEngine Vulkan::Vulkan sdl2 assetlib shaderc)

add_dependencies(AdAstrisEngine Shaders)
