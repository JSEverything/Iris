add_subdirectory(glfw)
add_subdirectory(glm)
add_subdirectory(Vulkan-Hpp)
add_subdirectory(fmt)
add_subdirectory(vk-bootstrap)
add_subdirectory(spdlog)
add_subdirectory(Glad)
add_subdirectory(tinyobjloader)

# ImGui
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
#file(GLOB_RECURSE IMGUI_SRC ${IMGUI_DIR}/**.cpp)
add_library(imgui STATIC)
#target_sources(imgui PRIVATE ${IMGUI_SRC})
target_sources(imgui PRIVATE
        "${IMGUI_DIR}/imgui.cpp"
        "${IMGUI_DIR}/imgui_demo.cpp"
        "${IMGUI_DIR}/imgui_draw.cpp"
        "${IMGUI_DIR}/imgui_widgets.cpp"
        "${IMGUI_DIR}/imgui_tables.cpp"
        "${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp"
        "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
        )

target_include_directories(imgui
        PUBLIC ${IMGUI_DIR}
        PUBLIC ${IMGUI_DIR}/backends
        )

target_link_libraries(imgui PRIVATE
        Vulkan::Vulkan
        glfw
        )

# ImGuizmo
set(IMGUIZMO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo)
#file(GLOB_RECURSE IMGUI_SRC ${IMGUI_DIR}/**.cpp)
add_library(ImGuizmo STATIC)
#target_sources(imgui PRIVATE ${IMGUI_SRC})
target_sources(ImGuizmo PRIVATE
        "${IMGUIZMO_DIR}/ImGuizmo.cpp"
        )

target_include_directories(ImGuizmo
        PUBLIC ${IMGUIZMO_DIR}
        )

target_link_libraries(ImGuizmo PRIVATE
        imgui
        )
