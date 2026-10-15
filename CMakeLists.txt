cmake_minimum_required(VERSION 3.24)

project(VulkanSpheres LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan 1.4 REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm CONFIG REQUIRED)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(VulkanSpheres ${SOURCES})

target_compile_features(VulkanSpheres PRIVATE cxx_std_20)

target_include_directories(VulkanSpheres PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/external/stb"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/tinyobjloader"
)
target_sources(VulkanSpheres PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/external/stb/stb_image.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/tinyobjloader/tiny_obj_loader.cpp"
)

set_target_properties(VulkanSpheres PROPERTIES
        CXX_EXTENSIONS OFF
)

target_link_libraries(VulkanSpheres PRIVATE
        Vulkan::Vulkan
        glfw
        glm::glm
        ${CMAKE_DL_LIBS}
)

target_compile_definitions(VulkanSpheres PRIVATE
        VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

        GLM_FORCE_RADIANS
        GLM_FORCE_DEPTH_ZERO_TO_ONE

        $<$<CONFIG:Debug>:DEBUG>
        $<$<CONFIG:Release>:RELEASE>
)