
#define CL_HPP_TARGET_OPENCL_VERSION 200

#include <jni.h>
#include <CL/cl.hpp>
#include <android/log.h>
#include <vector>
#include <iostream>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <list>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <random>
#include <nlohmann/json.hpp>
#include "cl_execution.h"

#define SIZE 1024
#define NANOSEC 1000000000LL

// for convenience
using json = nlohmann::json;

/*cl_device_id device_id = NULL;
cl_context context = NULL;
cl_command_queue command_queue = NULL;
cl_platform_id platform_id = NULL;
cl_uint ret_num_devices;
cl_uint ret_num_platforms;
cl_int ret;

#define SIZE 1000

void set_up_opencl_junk() {
  ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
  ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
  context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
  command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
}*/

// DEFAULT VALUES for testing
// This needs to be INPUT_FILE for Windows
char *OUTPUT = NULL;
std::string INPUT_FILE;
std::string kernel_include = "./tests";
int LIST = 0;
int PLATFORM_ID = 0;
int DEVICE_ID = 0;
int QUIET = 0;
int ITERATIONS = 1000;
int USE_CHIP_CONFIG = 0;

struct TestConfig {
    int hist_size;
    std::vector<std::string> hist_strings;
    int output_size;
};

struct ChipConfig {
    int max_local_size;
    int min_local_size;
    int occupancy_est;
    //int SIMD_width; //will be needed for intra-wg interactions
};

std::map<std::string, ChipConfig> ChipConfigMaps;

void populate_ChipConfigMaps() {
    ChipConfig defaultChipConfig = {256, 1, 18};
    ChipConfig Nvidia940M = {1024, 1, 16};
    ChipConfig IntelHD5500 = {256, 1, 16};
    ChipConfig Inteli75600u = {4, 1, 4};

    ChipConfigMaps["default"] = defaultChipConfig;
    ChipConfigMaps["Intel(R) Core(TM) i7-5600U CPU @ 2.60GHz"] = Inteli75600u;
    ChipConfigMaps["Intel(R) HD Graphics 5500"] = IntelHD5500;
    ChipConfigMaps["GeForce 940M"] = Nvidia940M;
}


TestConfig parse_config(const std::string &config_str, std::stringstream &return_str) {
    std::stringstream config_stream(config_str);
    TestConfig ret;
    std::string title, ignore;

    std::getline(config_stream, title);
    return_str << "parsing config for test: " << title << std::endl;
    std::getline(config_stream, ignore);
    std::getline(config_stream, ignore);

    ret.hist_size = std::stoi(ignore);

    //infile >> ret.hist_size;
    for (int i = 0; i < ret.hist_size; i++) {
        std::string out_desc;
        std::getline(config_stream, out_desc);
        ret.hist_strings.push_back(out_desc);
        return_str << "parsed line: " << out_desc << std::endl;
    }
    ret.hist_strings.push_back("errors: ");
    ret.hist_size++;
    std::getline(config_stream, ignore); // "num outputs"
    std::getline(config_stream, ignore);
    ret.output_size = std::stoi(ignore);
    return_str << "output size: " << ret.output_size << std::endl;
    return ret;
}

//From IWOCL tutorial (needs attribution)
unsigned long getDeviceList(std::vector<std::vector<cl::Device>> &devices) {
// Get list of platforms
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

// Enumerate devices
    for (unsigned int i = 0; i < platforms.size(); i++) {
        std::vector<cl::Device> plat_devices;
        platforms[i].getDevices(CL_DEVICE_TYPE_ALL, &plat_devices);
        devices.push_back(plat_devices);
        //devices.insert(devices.end(), plat_devices.begin(), plat_devices.end());
    }

    return devices.size();
}

cl_int get_kernels(CL_Execution &exec) {
    cl_int ret = CL_SUCCESS;
    exec.exec_kernels["litmus_test"] = cl::Kernel(exec.exec_program, "litmus_test", &ret);
    check_ocl(ret);

    //Consider doing this for robustness
    //exec.check_kernel_wg_sizes(exec.exec_kernels["bfs_init"], "bfs_init", TB_SIZE);

    exec.exec_kernels["check_outputs"] = cl::Kernel(exec.exec_program, "check_outputs", &ret);
    check_ocl(ret);

    //Consider doing this for robustness
    //exec.check_kernel_wg_sizes(exec.exec_kernels["bfs_kernel"], "bfs_kernel", TB_SIZE);
    return ret;
}

/*
 * https://stackoverflow.com/questions/41820039/jstringjni-to-stdstringc-with-utf8-characters
 * */
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
Java_com_example_openclexample_TestFinishedActivity_executeLitmusTest(JNIEnv *env,
                                                                      jobject thiz,
                                                                      jstring config_string,
                                                                      jstring kernel_string,
                                                                      jint iteration) {
    json j;
    int err = 0;
    CL_Execution exec;

    std::stringstream return_str;

    /* Modified configuration */
    USE_CHIP_CONFIG = 0;
    OUTPUT = NULL;
    ITERATIONS = iteration;
    LIST = 0;
    PLATFORM_ID = 0;
    DEVICE_ID = 0;
    INPUT_FILE = "../../../cpp/tests/MP";

    void *openCLHndl;
    const char *default_so_paths[] = {
            // Android
            "/system/lib/libOpenCL.so",
            "/system/lib64/libOpenCL.so",
            "/system/vendor/lib/libOpenCL.so",
            "/system/vendor/lib64/libOpenCL.so",
            "/system/vendor/lib64/libGLES_mali.so",
            "/system/vendor/lib/libPVROCL.so",
            "/data/data/org.pocl.libs/files/lib/libpocl.so",
    };
    for (const auto &path : default_so_paths) {
        openCLHndl = dlopen(path, RTLD_NOW);
        if (openCLHndl != NULL) break;
    }
    if (openCLHndl == NULL) {
        return_str << "ERROR: failed to link OpenCL Library.\n";
        return (*env).NewStringUTF(return_str.str().c_str());
    }
    typedef void (*func_ptr_t)(void);

    int (*myclGetPlatformIDs)(cl_uint, cl_platform_id *, cl_uint *);

    *(void **) (&myclGetPlatformIDs) = dlsym(openCLHndl, "clGetPlatformIDs");
    cl_platform_id my_platforms[100];
    cl_uint my_platforms_n = 0;
    (*myclGetPlatformIDs)(100, my_platforms, &my_platforms_n);
    return_str << "=== " << my_platforms_n << "OpenCL platform(s) found ===";

    std::vector<std::vector<cl::Device>> devices;
    getDeviceList(devices);

    if (PLATFORM_ID >= devices.size()) {
        return_str << "ERROR: Invalid platform id " << PLATFORM_ID
                   << ". Please use the -l option to view platforms and device ids.\n";
        return (*env).NewStringUTF(return_str.str().c_str());
    }

    if (DEVICE_ID >= devices[PLATFORM_ID].size()) {
        return_str << "ERROR: Invalid device id " << DEVICE_ID
                   << ". Please use the -l option to view platforms and device ids.\n";
        return (*env).NewStringUTF(return_str.str().c_str());
    }

    populate_ChipConfigMaps();
    exec.exec_device = devices[PLATFORM_ID][DEVICE_ID];

    return_str << "Using Device: " << exec.getExecDeviceName().c_str() << "\n";
    return_str << "Driver Version: " << exec.getExecDriverVersion().c_str() << "\n";

    ChipConfig cConfig;
    if (USE_CHIP_CONFIG) {
        return_str << "using chip config:" << exec.getExecDeviceName().c_str() << "\n";
        cConfig = ChipConfigMaps[exec.getExecDeviceName().c_str()];
    } else {
        return_str << "using chip config: default\n";
        cConfig = ChipConfigMaps["default"];
    }

    cl::Context context(exec.exec_device);
    exec.exec_context = context;
    cl::CommandQueue queue(exec.exec_context, exec.exec_device);
    exec.exec_queue = queue;

    std::string test_config_string = jstringToString(env, config_string);
    std::string test_kernel_String = jstringToString(env, kernel_string);

    TestConfig cfg = parse_config(test_config_string, return_str);

    err = exec.compile_kernel_string(test_kernel_String, kernel_include.c_str(), return_str);
    log_cl_err(err);

    // Original Kernel loading code
//    return_str << "Kernel file: " << kernel_file.c_str() << ", Kernel Include path: "
//               << kernel_include.c_str() << "\n";
//    err = exec.compile_kernel(kernel_file.c_str(), kernel_include.c_str());
//    check_ocl(err);

    int occupancy = cConfig.occupancy_est;
    int max_local_size = cConfig.max_local_size;

    int max_global_size = max_local_size * occupancy;
    cl::NDRange sglobalND(1), slocalND(1);

    err = get_kernels(exec);
    log_cl_err(err);

    // Actual real stuff starts

    cl::Buffer dga(exec.exec_context, CL_MEM_READ_WRITE, sizeof(cl_int) * (SIZE));
    cl::Buffer dgna(exec.exec_context, CL_MEM_READ_WRITE, sizeof(cl_int) * (SIZE));
    cl::Buffer doutput(exec.exec_context, CL_MEM_READ_WRITE, sizeof(cl_int) * (cfg.output_size));
    cl::Buffer dresult(exec.exec_context, CL_MEM_READ_WRITE, sizeof(cl_int) * (1));
    cl::Buffer dshuffled_ids(exec.exec_context, CL_MEM_READ_WRITE,
                             sizeof(cl_int) * (max_global_size));

    cl_int hga[SIZE], hgna[SIZE], houtput[SIZE];
    cl_int hresult;
    cl_int *hshuffled_ids = (cl_int *) malloc(sizeof(cl_int) * max_global_size);

    int *histogram = (int *) malloc(sizeof(int) * cfg.hist_size);
    return_str << "hist size: " << cfg.hist_size << std::endl;

    for (int i = 0; i < max_global_size; i++) hshuffled_ids[i] = i;
    for (int i = 0; i < cfg.hist_size; i++) histogram[i] = 0;
    for (int i = 0; i < SIZE; i++) hga[i] = hgna[i] = houtput[i] = 0;

    log_cl_err(exec.exec_kernels["litmus_test"].setArg(0, dga));
    log_cl_err(exec.exec_kernels["litmus_test"].setArg(1, dgna));
    log_cl_err(exec.exec_kernels["litmus_test"].setArg(2, doutput));
    log_cl_err(exec.exec_kernels["litmus_test"].setArg(3, dshuffled_ids));
    log_cl_err(exec.exec_kernels["check_outputs"].setArg(0, doutput));
    log_cl_err(exec.exec_kernels["check_outputs"].setArg(1, dresult));

    auto now = std::chrono::high_resolution_clock::now();
    long long begin_time, end_time, time;
    begin_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()).count();

    for (int i = 0; i < ITERATIONS; i++) {
        // set up ids
        size_t local_size = 256; //(size_t) rand() % max_local_size + cConfig.min_local_size;
        size_t global_size = occupancy * local_size;

        cl::NDRange globalND(global_size), localND(local_size);
        for (int j = 0; j < global_size; j++) {
            hshuffled_ids[j] = j;
        }

        // Random Shuffle
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(hshuffled_ids, &hshuffled_ids[global_size - 1], g);
        err = exec.exec_queue.enqueueWriteBuffer(dshuffled_ids, CL_TRUE, 0,
                                                 sizeof(cl_int) * (global_size), hshuffled_ids);
        log_cl_err(err);
        err = exec.exec_queue.enqueueWriteBuffer(dga, CL_TRUE, 0, sizeof(cl_int) * (SIZE), hga);
        log_cl_err(err);
        err = exec.exec_queue.enqueueWriteBuffer(dgna, CL_TRUE, 0, sizeof(cl_int) * (SIZE), hgna);
        log_cl_err(err);
        err = exec.exec_queue.enqueueWriteBuffer(doutput, CL_TRUE, 0,
                                                 sizeof(cl_int) * (cfg.output_size), houtput);
        log_cl_err(err);
        err = exec.exec_queue.finish();
        log_cl_err(err);
        err = exec.exec_queue.enqueueNDRangeKernel(exec.exec_kernels["litmus_test"], cl::NullRange,
                                                   globalND, localND);
        log_cl_err(err);
        err = exec.exec_queue.finish();
        log_cl_err(err);
        err = exec.exec_queue.enqueueNDRangeKernel(exec.exec_kernels["check_outputs"],
                                                   cl::NullRange, sglobalND, slocalND);
        log_cl_err(err);
        err = exec.exec_queue.finish();
        log_cl_err(err);
        err = exec.exec_queue.enqueueReadBuffer(dresult, CL_TRUE, 0, sizeof(cl_int) * (1),
                                                &hresult);
        log_cl_err(err);
        histogram[hresult]++;
    }
    now = std::chrono::high_resolution_clock::now();
    end_time = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    time = end_time - begin_time;
    float time_float = static_cast< float >(time) / static_cast< float >(NANOSEC);

    return_str << std::endl << "RESULTS: " << std::endl;
    return_str << "-------------------" << std::endl;
    for (int i = 0; i < cfg.hist_size; i++) {
        return_str << cfg.hist_strings[i] << histogram[i] << std::endl;
    }
    return_str << std::endl;
    return_str << "RATES" << std::endl;
    return_str << "-------------------" << std::endl;
    return_str << "tests          : " << ITERATIONS << std::endl;
    return_str << "time (seconds) : " << time_float << std::endl;
    return_str << "tests/sec      : " << static_cast< float >(ITERATIONS) / time_float << std::endl;
    return_str << std::endl;

    //free(hga);
    //free(hgna);
    //free(houtput);
    j["device_name"] = nullptr;
    j["kernel"] = nullptr;
    j["test_name"] = nullptr;

    return env->NewStringUTF(return_str.str().c_str());
}




