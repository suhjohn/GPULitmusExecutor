cmake_minimum_required(VERSION 3.4.1)

set(jniLibsDir ${PROJECT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})
set(jniLibsCommonDir ${PROJECT_SOURCE_DIR}/../jniLibsCommon)

# Include headers for OpenCL
include_directories(./includes)


add_library(OpenCL
        SHARED
        IMPORTED)

# https://cmake.org/cmake/help/v3.7/manual/cmake-toolchains.7.html#cross-compiling-for-android-with-the-ndk
if (${ANDROID_ABI} STREQUAL "armeabi")
    set_target_properties(OpenCL
            PROPERTIES IMPORTED_LOCATION
            ${jniLibsDir}/libGLES_mali.so)
elseif (${ANDROID_ABI} STREQUAL "armeabi-v7a")
    set_target_properties(OpenCL
            PROPERTIES IMPORTED_LOCATION
            ${jniLibsDir}/libOpenCL.so)
elseif (${ANDROID_ABI} STREQUAL "arm64-v8a")
    set_target_properties(OpenCL
            PROPERTIES IMPORTED_LOCATION
            ${jniLibsDir}/libGLES_mali.so)
elseif (${ANDROID_ABI} STREQUAL "x86")
    set_target_properties(OpenCL
            PROPERTIES IMPORTED_LOCATION
            ${jniLibsDir}/libGLES_mali.so)
else ()
    set_target_properties(OpenCL
            PROPERTIES IMPORTED_LOCATION
            ${jniLibsDir}/libGLES_mali.so)
endif ()


# Log
find_library(log-lib log)

# JNI Library
add_library(opencl-example
        SHARED
        opencl-example.c)

# Include libraries needed for opencl-example lib
# Since OpenCL is a build-time dependency, should be included here
target_link_libraries(
        opencl-example
        OpenCL
        android
        log)
