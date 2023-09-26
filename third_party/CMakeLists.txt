find_package(Vulkan REQUIRED)

add_library(vkbootstrap STATIC)
add_library(glm INTERFACE)
add_library(vma INTERFACE)
add_library(json INTERFACE)

add_library(stb_image INTERFACE)
add_library(cgltf INTERFACE)
add_library(reflection INTERFACE)

add_library(tinyobjloader STATIC)
add_library(lz4 STATIC)
add_library(fmt_lib STATIC)
add_library(spirv_reflect STATIC)
add_library(inicpp STATIC)

target_include_directories(vkbootstrap PUBLIC vkbootstrap)
file(GLOB VKBOOTSTRAP_FILES "vkbootstrap/*")
target_sources(vkbootstrap PRIVATE ${VKBOOTSTRAP_FILES})

target_link_libraries(vkbootstrap PUBLIC Vulkan::Vulkan $<$<BOOL:UNIX>:${CMAKE_DL_LIBS}>)

#both vma and glm and header only libs so we only need the include path
target_include_directories(vma INTERFACE vma)
target_include_directories(glm INTERFACE glm)
target_include_directories(json INTERFACE json)
target_include_directories(cgltf INTERFACE cgltf)
target_include_directories(reflection INTERFACE reflection)

target_sources(tinyobjloader PRIVATE 
tinyobjloader/tiny_obj_loader.h
tinyobjloader/tiny_obj_loader.cc
)

target_include_directories(tinyobjloader PUBLIC tinyobjloader)

target_sources(lz4 PRIVATE
    lz4/lz4.h
    lz4/lz4.c
    )

target_include_directories(lz4 PUBLIC lz4)

add_library(imgui STATIC)

target_include_directories(imgui PUBLIC imgui)
file(GLOB IMGUI_FILES "imgui/*")
target_sources(imgui PRIVATE ${IMGUI_FILES})

target_link_libraries(imgui PUBLIC Vulkan::Vulkan)

target_include_directories(stb_image INTERFACE stb_image)

target_include_directories(fmt_lib PUBLIC fmt_lib)
file(GLOB FMT_FILES "fmt_lib/fmt/*")
target_sources(fmt_lib PRIVATE ${FMT_FILES})

target_include_directories(spirv_reflect PUBLIC spirv_reflect)
target_sources(spirv_reflect PRIVATE
	spirv_reflect/spirv_reflect.h
	spirv_reflect/spirv_reflect.c
	)

target_include_directories(inicpp PUBLIC inicpp)
file(GLOB INICPP_FILES "inicpp/*")
target_sources(inicpp PRIVATE ${INICPP_FILES})

add_library(glad STATIC)
target_include_directories(glad PUBLIC glad)
target_sources(glad PRIVATE 
    glad/glad/glad.h
    glad/src/glad.c
    )
    