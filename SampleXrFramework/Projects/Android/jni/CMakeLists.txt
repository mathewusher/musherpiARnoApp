cmake_minimum_required(VERSION 3.18.1)
project(samplexrframework)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(
    -Wall
    -Wextra
    -Wshadow
    -Wno-unused-parameter
    -Wno-missing-field-initializers
)

file(GLOB_RECURSE FRAMEWORK_SRC
    ../../../Src/*.cpp
)

include_directories(
    ../../../Src
    ../../../../SampleCommon/Src
    ../../../../1stParty/OVR/Include
    ../../../../1stParty/utilities/include
    ../../../../3rdParty/stb/src
    ../../../../3rdParty/khronos/openxr/OpenXR-SDK/include
    ../../../../OpenXR/Include
    ${ANDROID_NDK}/sources/android/native_app_glue
)

add_library(samplexrframework STATIC ${FRAMEWORK_SRC})
if(TARGET android_native_app_glue)
    target_link_libraries(samplexrframework android_native_app_glue)
endif()

install(TARGETS samplexrframework
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)

install(DIRECTORY ../../../Src/
        DESTINATION include/samplexrframework
        FILES_MATCHING PATTERN "*.h")

