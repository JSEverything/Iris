file(GLOB_RECURSE SRC_FILES *.cpp)

add_executable(Editor ${SRC_FILES})

target_include_directories(Editor PUBLIC "${PROJECT_SOURCE_DIR}/Editor/src")
target_include_directories(Editor PUBLIC "${PROJECT_SOURCE_DIR}/Iris/src")
target_link_libraries(Editor Iris -static)

target_precompile_headers(Editor PRIVATE src/pch.hpp)

set(CMAKE_EXE_LINKER_FLAGS "-Wl,--allow-multiple-definition")