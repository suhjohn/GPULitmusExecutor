
#ifdef _WIN32
#include "getopt_win.h"
#else
#include "getopt.h"
#endif
#include "droidcl.h"
#include <jni.h>
#include "OpenCLLitmus_v_2/litmus_driver.cpp"
// To compile: nvcc .\main.c -lOpenCL


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


extern "C"
JNIEXPORT jstring JNICALL
        Java_com_suhjohn_androidgpuconformancetester_TestFinishedActivity_executeLitmusTest(
                JNIEnv *env,
                jobject thiz,
                jstring config_string,
                jstring kernel_string,
                jint iteration) {
    int err = 0;
    std::string opts;
    std::string return_str;
    std::string test = jstringToString(env, kernel_string);
    std::string test_config = jstringToString(env, config_string);
    run_test(test, test_config, 1, 0, 0, "", return_str);
    return env->NewStringUTF(return_str.c_str());
}
