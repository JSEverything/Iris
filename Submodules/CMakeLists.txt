add_subdirectory(glfw)
add_subdirectory(glm)
add_subdirectory(Vulkan-Hpp)
add_subdirectory(fmt)
add_subdirectory(vk-bootstrap)
add_subdirectory(spdlog)
add_subdirectory(Glad)

# ImGui
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
file(GLOB_RECURSE IMGUI_SRC ${IMGUI_DIR}/**.cpp)
add_library(IMGUI STATIC)
target_sources(IMGUI PRIVATE ${IMGUI_SRC})

target_include_directories(IMGUI
        PUBLIC ${IMGUI_DIR}
        PUBLIC ${IMGUI_DIR}/backends
        PUBLIC ${SDL2_DIR}/include
        )
