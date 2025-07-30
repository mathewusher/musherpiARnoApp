cmake_minimum_required(VERSION 3.18.1)
project(samplecommon)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(
    -Wall
    -Wextra
    -Wshadow
    -Wno-unused-parameter
    -Wno-missing-field-initializers
)

# Source and include paths
file(GLOB_RECURSE COMMON_SRC
    ../../../Src/*.cpp
)

include_directories(
    ../../../Src
)

# Define static library
add_library(samplecommon STATIC ${COMMON_SRC})

# Install compiled binary
install(TARGETS samplecommon
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)

# Install public headers
install(DIRECTORY ../../../Src/
        DESTINATION include/samplecommon
        FILES_MATCHING PATTERN "*.h")

