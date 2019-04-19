
#define CL_HPP_TARGET_OPENCL_VERSION 200

#include <jni.h>
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
#include "droidcl.h"

using json = nlohmann::json;

#define SIZE 1024
#define NANOSEC 1000000000LL

#define handle_cl_error(env, err) handle_error(env, err, __FILE__, __LINE__)

jstring handle_error(JNIEnv *env, const int e, const char *file, const int line) {
    if (e < 0) {
        std::stringstream error_string;
        json error_response;
        error_string << file << ":" << line << ": error " << e << std::endl;
        error_response["error"] = error_string.str();
        return env->NewStringUTF(error_response.dump().c_str());
    }
    return env->NewStringUTF("");
}

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

// DEFAULT VALUES for testing
// This needs to be INPUT_FILE for Windows
char *OUTPUT = NULL;
int LIST = 0;
int PLATFORM_ID = 0;
int DEVICE_ID = 0;
int QUIET = 0;
int ITERATIONS = 1000;
int USE_CHIP_CONFIG = 0;
std::string INPUT_FILE;
std::string kernel_include = "./tests";
cl_platform_id *platforms;
std::map<std::string, ChipConfig> ChipConfigMaps;
CL_Execution exec;
ChipConfig cConfig;
DroidCL droidCL;

/**
 * OpenCL Dynamic Loading
 * */
//void *openCLHndl;
//std::string default_so_paths[7] = {
//        "/system/lib/libOpenCL.so",
//        "/system/lib64/libOpenCL.so",
//        "/system/vendor/lib/libOpenCL.so",
//        "/system/vendor/lib64/libOpenCL.so",
//        "/system/vendor/lib64/libGLES_mali.so",
//        "/system/vendor/lib/libPVROCL.so",
//        "/data/data/org.pocl.libs/files/lib/libpocl.so",
//};
//
//cl_int (*DroidclGetPlatformIDs)(
//        cl_uint, cl_platform_id *, cl_uint *);
//
//cl_int (*DroidclGetDeviceIds)(
//        cl_platform_id, cl_device_type, cl_uint, cl_device_id *, cl_uint *);
//
//cl_int (*DroidclGetDeviceInfo)(
//        cl_device_id, cl_device_info, size_t, void *, size_t *);
//
//cl_program (*DroidclCreateProgramWithSource)(
//        cl_context, cl_uint, const char **, const size_t *, cl_int *);
//
//cl_int (*DroidclBuildProgram)(cl_program, cl_uint, const cl_device_id *, const char *,
//                              void (*pfn_notify)(cl_program, void *user_data), void *);
//
//cl_int (*DroidclGetProgramBuildInfo)(
//        cl_program, cl_device_id, cl_program_build_info, size_t, void *, size_t *);
//
//cl_context (*DroidclCreateContext)(
//        cl_context_properties *, cl_uint, const cl_device_id *, void *,
//        void *, cl_int *);
//
//cl_command_queue (*DroidclCreateCommandQueue)(
//        cl_context, cl_device_id, cl_command_queue_properties, cl_int *
//);
//
//cl_kernel (*DroidclCreateKernel)(cl_program, const char *, cl_int *);
//
//cl_mem (*DroidclCreateBuffer)(cl_context, cl_mem_flags, size_t, void *, cl_int *);
//
//cl_int (*DroidclSetKernelArg)(cl_kernel, cl_uint, size_t, const void *);
//
//void initializeDroidCL() {
//    for (const auto &path : default_so_paths) {
//        openCLHndl = dlopen(path.c_str(), RTLD_NOW);
//        if (openCLHndl != NULL) {
//            *(void **) (&DroidclGetPlatformIDs) = dlsym(
//                    openCLHndl, "clGetPlatformIDs");
//            *(void **) (&DroidclGetDeviceIds) = dlsym(
//                    openCLHndl, "clGetDeviceIDs");
//            *(void **) (&DroidclGetDeviceInfo) = dlsym(
//                    openCLHndl, "clGetDeviceInfo");
//            *(void **) (&DroidclCreateProgramWithSource) = dlsym(
//                    openCLHndl, "clCreateProgramWithSource");
//            *(void **) (&DroidclBuildProgram) = dlsym(
//                    openCLHndl, "clBuildProgram");
//            *(void **) (&DroidclGetProgramBuildInfo) = dlsym(
//                    openCLHndl, "clGetProgramBuildInfo");
//            *(void **) (&DroidclCreateContext) = dlsym(
//                    openCLHndl, "clCreateContext");
//            *(void **) (&DroidclCreateCommandQueue) = dlsym(
//                    openCLHndl, "clCreateCommandQueue");
//            *(void **) (&DroidclCreateKernel) = dlsym(
//                    openCLHndl, "clCreateKernel");
//            *(void **) (&DroidclCreateBuffer) = dlsym(
//                    openCLHndl, "clCreateBuffer");
//        }
//    }
//}
/**
 *
 * */

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


TestConfig parse_config(const std::string &config_str) {
    std::stringstream config_stream(config_str);
    TestConfig ret;
    std::string title, ignore;

    std::getline(config_stream, title);

    std::getline(config_stream, ignore);
    std::getline(config_stream, ignore);
    ret.hist_size = std::stoi(ignore);

    //infile >> ret.hist_size;
    for (int i = 0; i < ret.hist_size; i++) {
        std::string out_desc;
        std::getline(config_stream, out_desc);
        ret.hist_strings.push_back(out_desc);
    }
    ret.hist_strings.push_back("errors: ");
    ret.hist_size++;
    std::getline(config_stream, ignore); // "num outputs"
    std::getline(config_stream, ignore);
    ret.output_size = std::stoi(ignore);
    return ret;
}

//From IWOCL tutorial (needs attribution)
unsigned long getDeviceList(std::vector<std::vector<cl_device_id>> &devices) {
    DroidCL droidCL;
    int err = 0;
    cl_uint num_plats = 0;
    err = droidCL.clGetPlatformIDs(0, NULL, &num_plats);
    check_ocl(err);
    platforms = (cl_platform_id *) malloc(sizeof(cl_platform_id) * num_plats);
    droidCL.clGetPlatformIDs(num_plats, platforms, NULL);
    check_ocl(err);

    // Enumerate devices
    for (unsigned int i = 0; i < num_plats; i++) {
        //std::vector<cl::Device> plat_devices;
        cl_uint num_devices = 0;
        err = droidCL.clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
        check_ocl(err);
        cl_device_id *plat_devices = (cl_device_id *) malloc(sizeof(cl_device_id) * num_devices);
        droidCL.clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, num_devices, plat_devices, NULL);


        //platforms[i].getDevices(CL_DEVICE_TYPE_ALL, &plat_devices);
        std::vector<cl_device_id> to_push;
        for (unsigned int j = 0; j < num_devices; j++) {
            to_push.push_back(plat_devices[j]);
        }
        devices.push_back(to_push);
        //devices.insert(devices.end(), plat_devices.begin(), plat_devices.end());
    }
    return devices.size();
}

cl_int get_kernels(CL_Execution &exec) {

    cl_int ret = CL_SUCCESS;
    DroidCL droidCL;
    exec.exec_kernels["litmus_test"] = droidCL.clCreateKernel(
            exec.exec_program, "litmus_test", &ret);
    check_ocl(ret);

    //Consider doing this for robustness
    //exec.check_kernel_wg_sizes(exec.exec_kernels["bfs_init"], "bfs_init", TB_SIZE);

    exec.exec_kernels["check_outputs"] = droidCL.clCreateKernel(
            exec.exec_program, "check_outputs", &ret);
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
Java_com_suhjohn_androidgpuconformancetester_TestFinishedActivity_executeLitmusTest(
        JNIEnv *env,
        jobject thiz,
        jstring config_string,
        jstring kernel_string,
        jint iteration) {

    json response;
    std::string opts;
    std::stringstream return_str;
    std::vector<std::vector<cl_device_id>> devices;
    int err = 0;

    /* Modified configuration */
    USE_CHIP_CONFIG = 0;
    OUTPUT = NULL;
    ITERATIONS = iteration;
    LIST = 0;
    PLATFORM_ID = 0;
    DEVICE_ID = 0;
    INPUT_FILE = "../../../cpp/tests/MP";

    getDeviceList(devices);
    if (PLATFORM_ID >= devices.size()) {
        return_str << "Invalid platform id " << PLATFORM_ID
                   << ". Please use the -l option to view platforms and device ids.\n";
        response["error"] = return_str.str();
        return (*env).NewStringUTF(response.dump().c_str());
    }

    if (DEVICE_ID >= devices[PLATFORM_ID].size()) {
        return_str << "ERROR: Invalid device id " << DEVICE_ID
                   << ". Please use the -l option to view platforms and device ids.\n";
        response["error"] = return_str.str();
        return (*env).NewStringUTF(return_str.str().c_str());
    }

    populate_ChipConfigMaps();
    exec.exec_device = devices[PLATFORM_ID][DEVICE_ID];
    exec.exec_platform = platforms[PLATFORM_ID];
    CL_Execution::getDeviceName(exec.exec_device);

    if (USE_CHIP_CONFIG) {
        if (ChipConfigMaps.find(exec.getExecDeviceName().c_str())
            == ChipConfigMaps.end()) {
            cConfig = ChipConfigMaps["default"];
        } else {
            cConfig = ChipConfigMaps[exec.getExecDeviceName().c_str()];
        }
    } else {
        cConfig = ChipConfigMaps["default"];
    }
    cl_context_properties props[3] = {
            CL_CONTEXT_PLATFORM, (cl_context_properties) exec.exec_platform, 0};
    exec.exec_context = droidCL.clCreateContext(
            props, 1, &(exec.exec_device), NULL, NULL, &err);
    handle_cl_error(env, err);

    exec.exec_queue = droidCL.clCreateCommandQueue(
            exec.exec_context, exec.exec_device, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err);
    handle_cl_error(env, err);

    std::string test_config_string = jstringToString(env, config_string);
    std::string test_kernel_string = jstringToString(env, kernel_string);
    TestConfig cfg = parse_config(test_config_string);

    err = exec.compile_kernel_string(test_kernel_string, kernel_include.c_str(), opts);
    if (err < 0) {
        char buffer[2048];
        droidCL.clGetProgramBuildInfo(
                exec.exec_program, exec.exec_device, CL_PROGRAM_BUILD_LOG, 2048, buffer, NULL);
        return_str << "[exec.compile_kernel_string] ERROR: " << err << std::endl;
        return_str << buffer << std::endl;
        response["error"] = return_str.str();
        return env->NewStringUTF(response.dump().c_str());
    }

    int occupancy = cConfig.occupancy_est;
    int max_local_size = cConfig.max_local_size;
    int max_global_size = max_local_size * occupancy;

    err = get_kernels(exec);
    handle_cl_error(env, err);

    // Actual real stuff starts
    cl_mem dga = droidCL.clCreateBuffer(exec.exec_context, CL_MEM_READ_WRITE,
                                        sizeof(cl_int) * (SIZE), NULL,
                                        &err);
    check_ocl(err);
    cl_mem dgna = droidCL.clCreateBuffer(exec.exec_context, CL_MEM_READ_WRITE,
                                         sizeof(cl_int) * (SIZE),
                                         NULL, &err);
    check_ocl(err);
    cl_mem doutput = droidCL.clCreateBuffer(exec.exec_context, CL_MEM_READ_WRITE,
                                            sizeof(cl_int) * ((cfg.output_size)), NULL, &err);
    check_ocl(err);
    cl_mem dresult = droidCL.clCreateBuffer(exec.exec_context, CL_MEM_READ_WRITE,
                                            sizeof(cl_int) * (1),
                                            NULL, &err);
    check_ocl(err);
    cl_mem dshuffled_ids = droidCL.clCreateBuffer(exec.exec_context, CL_MEM_READ_WRITE,
                                                  sizeof(cl_int) * (max_global_size), NULL, &err);
    check_ocl(err);
    cl_mem dscratchpad = droidCL.clCreateBuffer(exec.exec_context, CL_MEM_READ_WRITE,
                                                sizeof(cl_int) * (512), NULL, &err);
    check_ocl(err);
    cl_int dwarp_size = 32;

    cl_int hga[SIZE], hgna[SIZE], houtput[SIZE];
    cl_int hresult;
    cl_int *hshuffled_ids = (cl_int *) malloc(sizeof(cl_int) * max_global_size);

    std::cout << "hist size: " << cfg.hist_size << std::endl;

    for (int i = 0; i < max_global_size; i++) {
        hshuffled_ids[i] = i;
    }

    for (int i = 0; i < SIZE; i++) {
        hga[i] = hgna[i] = houtput[i] = 0;
    }

    // cl_int scratch_location = 128;
    // cl_int x_y_distance = 128;

    check_ocl(droidCL.clSetKernelArg(exec.exec_kernels["litmus_test"], 0, sizeof(cl_mem), &dga));
    check_ocl(droidCL.clSetKernelArg(exec.exec_kernels["litmus_test"], 1, sizeof(cl_mem), &dgna));
    check_ocl(
            droidCL.clSetKernelArg(exec.exec_kernels["litmus_test"], 2, sizeof(cl_mem), &doutput));
    check_ocl(droidCL.clSetKernelArg(exec.exec_kernels["litmus_test"], 3, sizeof(cl_mem),
                                     &dshuffled_ids));
    check_ocl(droidCL.clSetKernelArg(exec.exec_kernels["litmus_test"], 4, sizeof(cl_mem),
                                     &dscratchpad));
    check_ocl(droidCL.clSetKernelArg(exec.exec_kernels["litmus_test"], 7, sizeof(cl_int),
                                     &dwarp_size));

    check_ocl(droidCL.clSetKernelArg(exec.exec_kernels["check_outputs"], 0, sizeof(cl_mem),
                                     &doutput));
    check_ocl(droidCL.clSetKernelArg(exec.exec_kernels["check_outputs"], 1, sizeof(cl_mem),
                                     &dresult));

    auto now = std::chrono::high_resolution_clock::now();
    long long begin_time, end_time, time;
    begin_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()).count();

    int *shuffled_wg_order = (int *) malloc(occupancy * sizeof(int));
    int *temp_id_ordering = (int *) malloc(max_global_size * sizeof(int));
    int *shuffled_warp_order = (int *) malloc((max_local_size / dwarp_size) * sizeof(int));

    // For loop over x_y_dist between 2 and 512 incrementing by two
    // For loop over scrach_location between 2 and 512 incrementing by two
    for (int dist = 65; dist <= 512; dist += 32) {
        cl_int x_y_distance = dist;
        for (int location = 65; location <= 512; location += 32) {
            cl_int scratch_location = location;
            for (int offset = 0; offset < 2; offset++) {
                int *histogram = (int *) malloc(sizeof(int) * cfg.hist_size);
                for (int i = 0; i < cfg.hist_size; i++) {
                    histogram[i] = 0;
                }

                for (int i = 0; i < ITERATIONS; i++) {
                    check_ocl(droidCL.clSetKernelArg(exec.exec_kernels["litmus_test"], 5,
                                                     sizeof(cl_int),
                                                     &scratch_location));
                    cl_int to_pass = x_y_distance + offset;
                    check_ocl(droidCL.clSetKernelArg(exec.exec_kernels["litmus_test"], 6,
                                                     sizeof(cl_int),
                                                     &to_pass));

                    // set up ids
                    // mod by zero, if max local size is same and min
                    int local_size = (rand() % (max_local_size - cConfig.min_local_size)) +
                                     cConfig.min_local_size;
                    int global_size = occupancy * local_size;
                    int wg_count = global_size / local_size;
                    int warp_size = dwarp_size; // add to chip config before amd testing
                    int warps_per_wg = local_size / warp_size;
                    int remainder_threads = local_size % warp_size;

                    for (int j = 0; j < global_size; j++) {
                        hshuffled_ids[j] = i;
                    }

                    // id shuffle
                    for (int i = 0; i < wg_count; i++) {
                        shuffled_wg_order[i] = i;
                    }
                    std::random_shuffle(shuffled_wg_order,
                                        &shuffled_wg_order[wg_count]); // random_shuffle not end inclusive

                    for (int i = 0; i < global_size; i++) {
                        temp_id_ordering[i] = i;
                    }

                    for (int i = 0; i < wg_count; i++) {
                        for (int j = 0; j < warps_per_wg; j++) {
                            std::random_shuffle(
                                    &temp_id_ordering[i * local_size + j * warp_size],
                                    &temp_id_ordering[i * local_size +
                                                      (j + 1) * warp_size]);
                        }
                    }
                    for (int i = 0; i < wg_count; i++) {

                        for (int x = 0; x < warps_per_wg; x++) {
                            shuffled_warp_order[x] = x;
                        }
                        std::random_shuffle(shuffled_warp_order,
                                            &shuffled_warp_order[warps_per_wg]);

                        for (int j = 0; j < warps_per_wg; j++) {
                            for (int k = 0; k < warp_size; k++) {
                                hshuffled_ids[i * local_size + j * warp_size +
                                              k] = temp_id_ordering[
                                        shuffled_wg_order[i] * local_size +
                                        shuffled_warp_order[j] * warp_size + k];
                            }
                        }

                        if (remainder_threads != 0) {
                            for (int y = 0; y < remainder_threads; y++) {
                                hshuffled_ids[i * local_size + (warps_per_wg + 1) * warp_size +
                                              y] = temp_id_ordering[
                                        shuffled_wg_order[i] * local_size +
                                        (warps_per_wg + 1) * warp_size + y];
                            }
                        }
                    }

                    err = droidCL.clEnqueueWriteBuffer(exec.exec_queue, dshuffled_ids, CL_TRUE, 0,
                                                       sizeof(cl_int) * (global_size),
                                                       hshuffled_ids, 0,
                                                       NULL, NULL);
                    check_ocl(err);

                    err = droidCL.clEnqueueWriteBuffer(exec.exec_queue, dga, CL_TRUE, 0,
                                                       sizeof(cl_int) * (SIZE), hga, 0, NULL, NULL);
                    check_ocl(err);

                    err = droidCL.clEnqueueWriteBuffer(exec.exec_queue, dgna, CL_TRUE, 0,
                                                       sizeof(cl_int) * (SIZE), hgna, 0, NULL,
                                                       NULL);
                    check_ocl(err);

                    err = droidCL.clEnqueueWriteBuffer(exec.exec_queue, doutput, CL_TRUE, 0,
                                                       sizeof(cl_int) * (cfg.output_size), houtput,
                                                       0, NULL,
                                                       NULL);
                    check_ocl(err);

                    err = droidCL.clFinish(exec.exec_queue);
                    check_ocl(err);

                    const size_t global_size_t = global_size;
                    const size_t local_size_t = local_size;
                    err = droidCL.clEnqueueNDRangeKernel(exec.exec_queue,
                                                         exec.exec_kernels["litmus_test"],
                                                         1, NULL, &global_size_t, &local_size_t, 0,
                                                         NULL,
                                                         NULL);
                    check_ocl(err);

                    err = droidCL.clFinish(exec.exec_queue);
                    check_ocl(err);

                    const size_t global_size_t2 = 1;
                    const size_t local_size_t2 = 1;
                    err = droidCL.clEnqueueNDRangeKernel(exec.exec_queue,
                                                         exec.exec_kernels["check_outputs"], 1,
                                                         NULL,
                                                         &global_size_t2, &local_size_t2, 0, NULL,
                                                         NULL);
                    check_ocl(err);

                    err = droidCL.clEnqueueReadBuffer(exec.exec_queue, dresult, CL_TRUE, 0,
                                                      sizeof(cl_int) * 1, &hresult, 0, NULL, NULL);
                    check_ocl(err);

                    histogram[hresult]++;
                }
                now = std::chrono::high_resolution_clock::now();
                end_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
                        now.time_since_epoch()).count();
                time = end_time - begin_time;
                float time_float = static_cast<float>(time) / static_cast<float>(NANOSEC);

                json stress_test_obj;
                stress_test_obj["x_y_distance"] = x_y_distance;
                stress_test_obj["offset"] = offset;
                stress_test_obj["scratch_location"] = scratch_location;
                json stress_test_histogram;
                for (int i = 0; i < cfg.hist_size; i++) {
                    stress_test_histogram[cfg.hist_strings[i]] = histogram[i];
                }
                stress_test_obj["histogram"] = stress_test_histogram;
                stress_test_obj["time"] = time_float;
                stress_test_obj["test_count"] = ITERATIONS;
                stress_test_obj["test_time_ratio"] = static_cast< float >(ITERATIONS) / time_float;
                std::stringstream stress_test_obj_key;
                stress_test_obj_key << x_y_distance + offset << ":" << scratch_location;
                response[stress_test_obj_key.str()] = stress_test_obj;
            }
        }
    }
    free(shuffled_wg_order);
    free(temp_id_ordering);
    free(shuffled_warp_order);
    free(platforms);

    return env->NewStringUTF(response.dump().c_str());
//    return env->NewStringUTF(return_str.str().c_str());
}
