find_package(Vulkan REQUIRED)

file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.hpp)

add_library(Iris STATIC ${SRC_FILES})

target_include_directories(Iris PUBLIC "${PROJECT_SOURCE_DIR}/Iris/src")
target_link_libraries(Iris PUBLIC
        Vulkan::Vulkan
        glfw
        glm::glm
        fmt::fmt
        spdlog::spdlog
        vk-bootstrap::vk-bootstrap
        GLAD
        )
target_precompile_headers(Iris PUBLIC src/pch.hpp)

target_compile_definitions(Iris PUBLIC
        $<$<CONFIG:Debug>:IRIS_DEBUG>
        $<$<CONFIG:Release>:IRIS_RELEASE>
        )

if (WIN32)
    target_compile_definitions(Iris PUBLIC IRIS_PLATFORM_WINDOWS)
endif (WIN32)

if (UNIX)
    target_compile_definitions(Iris PUBLIC IRIS_PLATFORM_LINUX)
endif (UNIX)

add_dependencies(Iris Shaders)