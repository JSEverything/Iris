file(GLOB_RECURSE SRC_FILES src/*.c)
add_library(GLAD STATIC)
target_sources(GLAD PRIVATE ${SRC_FILES})
target_include_directories(GLAD
        PUBLIC src
        PUBLIC include
        )
