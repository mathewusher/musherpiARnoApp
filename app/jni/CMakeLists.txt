cmake_minimum_required(VERSION 3.18.1)
project(xrpassthrough)

set(CMAKE_CXX_STANDARD 17)

# Compiler warnings
add_compile_options(
    -Wall
    -Wextra
    -Wshadow
    -Wno-unused-parameter
    -Wno-missing-field-initializers
    -fvisibility=hidden
)

# Enable PIE
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Main source files
file(GLOB_RECURSE XR_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/main/cpp/*.cpp
)

# Include paths
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/main/cpp
    ../../1stParty/OVR/Include
    ../../1stParty/utilities/include
    ../../3rdParty/stb/src
    ../../3rdParty/minizip/src
    ../../3rdParty/khronos/openxr/OpenXR-SDK/include
    ../../OpenXR/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/External/curl/include
    ../../3rdParty/ktx/include
)

# External native modules with explicit binary dirs
add_subdirectory(${CMAKE_SOURCE_DIR}/../../SampleCommon/Projects/Android/jni ${CMAKE_BINARY_DIR}/_samplecommon)
add_subdirectory(${CMAKE_SOURCE_DIR}/../../SampleXrFramework/Projects/Android/jni ${CMAKE_BINARY_DIR}/_samplexrframework)
add_subdirectory(${CMAKE_SOURCE_DIR}/../../3rdParty/stb/build/android/jni ${CMAKE_BINARY_DIR}/_stb)
add_subdirectory(${CMAKE_SOURCE_DIR}/../../3rdParty/minizip/build/android/jni ${CMAKE_BINARY_DIR}/_minizip)

# Android native app glue
add_library(android_native_app_glue STATIC
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
)
target_include_directories(android_native_app_glue INTERFACE
    ${ANDROID_NDK}/sources/android/native_app_glue
)

# Prebuilt curl
add_library(curl STATIC IMPORTED)
set_target_properties(curl PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/External/curl/libcurl.a)
target_include_directories(curl INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/External/curl/include)
# Prebuilt ktx
add_library(ktx STATIC IMPORTED)
set_target_properties(ktx PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/../../3rdParty/ktx/lib/arm64-v8a/libktx.a)
target_include_directories(ktx INTERFACE
    ${CMAKE_SOURCE_DIR}/../../3rdParty/ktx/include)

# Define shared library
add_library(xrpassthrough SHARED ${XR_SRC})

# Shared OpenXR loader
add_library(openxr_loader SHARED IMPORTED)
set_target_properties(openxr_loader PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../../OpenXR/Libs/Android/arm64-v8a/Debug/libopenxr_loader.so)
target_include_directories(openxr_loader INTERFACE
    ${CMAKE_SOURCE_DIR}/../../OpenXR/Include)


# Link dependencies
target_link_libraries(xrpassthrough
    log
    android
    GLESv3
    EGL
    z
    m
    openxr_loader
    samplecommon
    samplexrframework
    stb
    minizip
    curl
    ktx
    android_native_app_glue
)

# OpenGL ES 3 Debug toggle
option(ENABLE_GLES3_DEBUG "Enable OpenGL ES 3 KHR_debug support" ON)

if(ENABLE_GLES3_DEBUG)
    add_definitions(-DENABLE_GLES3_DEBUG=1)
else()
    add_definitions(-DENABLE_GLES3_DEBUG=0)
endif()

