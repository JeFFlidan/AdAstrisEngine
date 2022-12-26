add_library(logger INTERFACE)

# Add source to this project's executable.
add_executable(vulkan_guide
    main.cpp
    vk_engine.cpp
    vk_engine.h
    vk_types.h
    vk_types.cpp
    vk_initializers.cpp
    vk_initializers.h
    vk_mesh.h
    vk_mesh.cpp
    vk_textures.h
    vk_textures.cpp
    vk_descriptors.h
    vk_descriptors.cpp
    vk_camera.h
    vk_camera.cpp
    material_system.h
    material_system.cpp
    vk_shaders.h
    vk_shaders.cpp
    vk_renderpass.h
    vk_renderpass.cpp
    vk_scene.h
    vk_scene.cpp
    vk_renderer.cpp)


set_property(TARGET vulkan_guide PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:vulkan_guide>")

target_include_directories(logger INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_sources(logger PRIVATE logger.h)
target_include_directories(vulkan_guide PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(vulkan_guide vkbootstrap vma glm tinyobjloader imgui stb_image fmt_lib logger spirv_reflect)

target_link_libraries(vulkan_guide Vulkan::Vulkan sdl2 assetlib)

add_dependencies(vulkan_guide Shaders)
