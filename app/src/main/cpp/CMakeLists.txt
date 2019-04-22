cmake_minimum_required(VERSION 3.10)


# Include OpenCL

set(jniLibsDir ${PROJECT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})

include_directories(includes)


find_library(log-lib log)

add_library(litmus-test
        SHARED
        main.cpp)

target_link_libraries(
        litmus-test
        android
        log)