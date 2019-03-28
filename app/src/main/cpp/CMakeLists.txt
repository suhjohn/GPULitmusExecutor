cmake_minimum_required(VERSION 3.4.1)


# Include OpenCL
set(jniLibsDir ${PROJECT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})

include_directories(includes)

add_library(OpenCL
        SHARED
        IMPORTED)

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

find_library(log-lib log)

add_library(opencl-example
        SHARED
        opencl-example.c)

target_link_libraries(
        opencl-example
        OpenCL
        android
        log)

add_library(litmus-test
        SHARED
        main.cpp)

target_link_libraries(
        litmus-test
        OpenCL
        android
        log)
