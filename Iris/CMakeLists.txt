find_package(Vulkan REQUIRED)
find_package(Threads REQUIRED)

file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.hpp)

add_library(Iris STATIC ${SRC_FILES})

set_target_properties(Iris PROPERTIES LINK_SEARCH_START_STATIC 1)
set_target_properties(Iris PROPERTIES LINK_SEARCH_END_STATIC 1)

target_include_directories(Iris PUBLIC
        "${PROJECT_SOURCE_DIR}/Iris/src"
        "${PROJECT_SOURCE_DIR}/Submodules/stb"
        )
target_link_libraries(Iris PUBLIC
        Threads::Threads
        Vulkan::Vulkan
        glfw
        glm::glm
        fmt::fmt
        spdlog::spdlog
        vk-bootstrap::vk-bootstrap
        GLAD
        tinyobjloader
        imgui
        ImGuizmo
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