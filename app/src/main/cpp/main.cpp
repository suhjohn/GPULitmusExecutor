
#ifdef _WIN32
#include "getopt_win.h"
#else

#include "getopt.h"

#endif

#include "droidcl.h"
#include <jni.h>
#include "OpenCLLitmus_v_2/litmus_driver.cpp"


std::string jstringToString(JNIEnv *env, jstring jStr) {
    if (!jStr)
        return "";
    const jclass stringClass = env->GetObjectClass(jStr);
    const jmethodID getBytes = env->GetMethodID(
            stringClass, "getBytes", "(Ljava/lang/String;)[B");
    const jbyteArray stringJbytes = (jbyteArray) env->CallObjectMethod(
            jStr, getBytes, env->NewStringUTF("UTF-8"));

    size_t length = (size_t) env->GetArrayLength(stringJbytes);
    jbyte *pBytes = env->GetByteArrayElements(stringJbytes, NULL);

    std::string ret = std::string((char *) pBytes, length);
    env->ReleaseByteArrayElements(stringJbytes, pBytes, JNI_ABORT);

    env->DeleteLocalRef(stringJbytes);
    env->DeleteLocalRef(stringClass);
    return ret;
}

int jintToInt(JNIEnv *env, jint value) {
    int val = (int32_t) value;
    return val;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_suhjohn_androidgpuconformancetester_TestFinishedActivity_executeLitmusTest(
        JNIEnv *env,
        jobject thiz,
        jstring config_string,
        jstring kernel_string,
        jint iterations,
        jstring options,
        jint x_y_stride
) {
    // Barrier as a separate buffer?
    std::string opts;
    std::string return_str;
    std::string test = jstringToString(env, kernel_string);
    std::string test_config = jstringToString(env, config_string);
    std::string options_cstr = jstringToString(env, options);
    int iterations_cint = jintToInt(env, iterations);
    int x_y_stride_cint = jintToInt(env, x_y_stride);
    // TODO change function definition to pass x_y_stride as argument

    run_test(test, test_config, iterations_cint, x_y_stride_cint, 0, 0, options_cstr, return_str);
    return env->NewStringUTF(return_str.c_str());
}

// For FullTestActivity.java
extern "C"
JNIEXPORT jstring JNICALL
Java_com_suhjohn_androidgpuconformancetester_FullTestActivity_executeLitmusTest(
        JNIEnv *env,
        jobject thiz,
        jstring config_string,
        jstring kernel_string,
        jint iterations,
        jstring options,
        jint x_y_stride
) {
    // Barrier as a separate buffer?
    std::string opts;
    std::string return_str;
    std::string test = jstringToString(env, kernel_string);
    std::string test_config = jstringToString(env, config_string);
    std::string options_cstr = jstringToString(env, options);
    int iterations_cint = jintToInt(env, iterations);
    int x_y_stride_cint = jintToInt(env, x_y_stride);
    // TODO change function definition to pass x_y_stride as argument

    run_test(test, test_config, iterations_cint, x_y_stride_cint, 0, 0, options_cstr, return_str);
    return env->NewStringUTF(return_str.c_str());
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_suhjohn_androidgpuconformancetester_utils_LitmusTest_executeLitmusTest(
        JNIEnv *env,
        jobject thiz,
        jstring config_string,
        jstring kernel_string,
        jint iterations,
        jstring options,
        jint x_y_stride
) {
    // Barrier as a separate buffer?
    std::string opts;
    std::string return_str;
    std::string test = jstringToString(env, kernel_string);
    std::string test_config = jstringToString(env, config_string);
    std::string options_cstr = jstringToString(env, options);
    int iterations_cint = jintToInt(env, iterations);
    int x_y_stride_cint = jintToInt(env, x_y_stride);
    // TODO change function definition to pass x_y_stride as argument

    run_test(test, test_config, iterations_cint, x_y_stride_cint, 0, 0, options_cstr, return_str);
    return env->NewStringUTF(return_str.c_str());
}
