cmake_minimum_required(VERSION 3.20)
project("Iris")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Binaries")

find_package(Vulkan REQUIRED)

add_subdirectory(Submodules)
add_subdirectory(Shaders)
add_subdirectory(Iris)
add_subdirectory(Editor)