cmake_minimum_required(VERSION 3.4.1)

set(jniLibsDir ${PROJECT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})
set(jniLibsCommonDir ${PROJECT_SOURCE_DIR}/../jniLibsCommon)

# Include headers for OpenCL
include_directories(/Users/mac/AndroidStudioProjects/HelloJNI3/app/src/main/cpp/includes)


#add_library(foo
#        SHARED
#        IMPORTED)
#add_library(log
#        SHARED
#        IMPORTED)
#add_library(nativewindow
#        SHARED
#        IMPORTED)
#add_library(z
#        SHARED
#        IMPORTED)
##add_library(cpp
##        SHARED
##        IMPORTED)
#add_library(utils
#        SHARED
#        IMPORTED)
#add_library(cutils
#        SHARED
#        IMPORTED)
#add_library(m
#        SHARED
#        IMPORTED)
#add_library(c
#        SHARED
#        IMPORTED)
#add_library(dl
#        SHARED
#        IMPORTED)
#
#set_target_properties(foo
#        PROPERTIES IMPORTED_LOCATION
#        ${jniLibsDir}/libfoo.so
#        )
#set_target_properties(log
#        PROPERTIES IMPORTED_LOCATION
#        ${jniLibsDir}/liblog.so
#        )
#set_target_properties(nativewindow
#        PROPERTIES IMPORTED_LOCATION
#        ${jniLibsDir}/libnativewindow.so
#        )
#set_target_properties(z
#        PROPERTIES IMPORTED_LOCATION
#        ${jniLibsDir}/libz.so
#        )
##set_target_properties(cpp
##        PROPERTIES IMPORTED_LOCATION
##        ${jniLibsDir}/libc++.so
##        )
#set_target_properties(utils
#        PROPERTIES IMPORTED_LOCATION
#        ${jniLibsDir}/libutils.so
#        )
#set_target_properties(cutils
#        PROPERTIES IMPORTED_LOCATION
#        ${jniLibsDir}/libcutils.so
#        )
#set_target_properties(m
#        PROPERTIES IMPORTED_LOCATION
#        ${jniLibsDir}/libm.so
#        )
#set_target_properties(c
#        PROPERTIES IMPORTED_LOCATION
#        ${jniLibsDir}/libc.so
#        )
#set_target_properties(dl
#        PROPERTIES IMPORTED_LOCATION
#        ${jniLibsDir}/libdl.so
#        )

# OpenCL

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
add_library(hello-jni
        SHARED
        hello-jni.c)

# Include libraries needed for hello-jni lib
# Since OpenCL is a build-time dependency, should be included here
target_link_libraries(
        hello-jni
        OpenCL
#        foo
#        log
#        nativewindow
#        z
#        cpp
#        utils
#        cutils
#        m
#        c
#        dl
        android
        log)
