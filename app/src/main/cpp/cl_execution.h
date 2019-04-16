#pragma once

#include <CL/cl.h>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <assert.h>
#include <limits.h>
#include "droidcl.h"

//From IWOCL tutorial (needs attribution)
#ifndef CL_DEVICE_BOARD_NAME_AMD
#define CL_DEVICE_BOARD_NAME_AMD 0x4038
#endif

#define check_ocl(err) check_ocl_error(err, __FILE__, __LINE__)

void check_ocl_error(const int e, const char *file, const int line) {
    if (e < 0) {
        printf("%s:%d: error (%d)\n", file, line, e);
        exit(1);
    }
}


class CL_Execution {
public:
    cl_device_id exec_device;
    cl_context exec_context;
    cl_command_queue exec_queue;
    cl_program exec_program;
    std::map<std::string, cl_kernel> exec_kernels;
    cl_platform_id exec_platform;

private:
    // For recompiling the kernel with the correct subgroup size
    const char *cache_kernel_file;
    const char *cache_kernel_include;
    bool cache_slots;
    bool cache_discovery;
    bool cache_irgl_subgroups;
    int cache_sub_group_size;

public:

    //From IWOCL tutorial (needs attribution)
    static std::string getDeviceName(const cl_device_id &device) {
        DroidCL droidCL;
        char name[256];
        cl_device_info info = CL_DEVICE_NAME;

        // Special case for AMD
        int err = 0;
        err = droidCL.clGetDeviceInfo(device, info, 256, name, NULL);
        check_ocl(err);
        std::string ret(name);
#ifdef CL_DEVICE_BOARD_NAME_AMD
        if (strstr(ret.c_str(), "Advanced Micro Devices"))
            info = CL_DEVICE_BOARD_NAME_AMD;
#endif
        return ret;
    }

    static std::string getDriverVersion(const cl_device_id &device) {
        DroidCL droidCL;
        char version[256];
        cl_device_info info = CL_DRIVER_VERSION;
        int err = 0;
        err = droidCL.clGetDeviceInfo(device, info, 256, version, NULL);
        check_ocl(err);
        return std::string(version);
    }

    std::string getExecDeviceName() {
        return getDeviceName(exec_device);
    }

    std::string getExecDriverVersion() {
        return getDriverVersion(exec_device);
    }

    int get_SMs() {
        DroidCL droidCL;
        cl_uint ret;
        cl_device_info info = CL_DEVICE_MAX_COMPUTE_UNITS;

        // Maybe special case it for Intel?
        //#ifdef CL_DEVICE_BOARD_NAME_AMD
        //device.getInfo(CL_DEVICE_VENDOR, &name);
        //if (strstr(name.c_str(), "Advanced Micro Devices"))
        //  info = CL_DEVICE_BOARD_NAME_AMD;
        //#endif
        droidCL.clGetDeviceInfo(exec_device, info, sizeof(cl_uint), &ret, NULL);
        //exec_device.getInfo(info, &ret);
        return ret;
    }

    bool is_Nvidia() {
        DroidCL droidCL;
        char buffer[256];
        cl_device_info info = CL_DEVICE_VENDOR;
        int err = 0;
        err = droidCL.clGetDeviceInfo(exec_device, info, 256, buffer, NULL);
        check_ocl(err);
        std::string to_search = buffer;
        if (to_search.find("NVIDIA Corporation") == std::string::npos) {
            return false;
        }
        return true;
    }

    bool is_Intel() {
        DroidCL droidCL;
        char buffer[256];
        cl_device_info info = CL_DEVICE_VENDOR;
        int err = 0;
        err = droidCL.clGetDeviceInfo(exec_device, info, 256, buffer, NULL);
        check_ocl(err);
        std::string to_search = buffer;
        if (to_search.find("Intel") == std::string::npos) {
            return false;
        }
        return true;
    }

    bool is_AMD() {
        DroidCL droidCL;
        char buffer[256];
        cl_device_info info = CL_DEVICE_VENDOR;
        int err = 0;
        err = droidCL.clGetDeviceInfo(exec_device, info, 256, buffer, NULL);
        check_ocl(err);
        std::string to_search = buffer;
        if (to_search.find("Advanced Micro Devices") == std::string::npos) {
            return false;
        }
        return true;
    }

    bool is_ARM() {
        DroidCL droidCL;
        // Tyler TODO
        char buffer[256];
        cl_device_info info = CL_DEVICE_VENDOR;
        int err = 0;
        err = droidCL.clGetDeviceInfo(exec_device, info, 256, buffer, NULL);
        check_ocl(err);
        std::string to_search = buffer;
        if (to_search.find("ARM") == std::string::npos) {
            return false;
        }
        return true;
    }

    bool is_ocl2() {
        DroidCL droidCL;
        char buffer[256];
        cl_device_info info = CL_DEVICE_VERSION;
        int err = 0;
        err = droidCL.clGetDeviceInfo(exec_device, info, 256, buffer, NULL);
        check_ocl(err);
        std::string to_search(buffer);
        if (to_search.find("OpenCL 2.") == std::string::npos) {
            return false;
        }
        return true;
    }

    std::string get_vendor_option() {
        if (is_Nvidia()) {
            return " -DNVIDIA ";
        }
        if (is_Intel()) {
            return " -DINTEL ";
        }
        if (is_AMD()) {
            return " -DAMD ";
        }
        if (is_ARM()) {
            return " -DARM ";
        }
        return "";
    }

    std::string check_ocl2x() {
        if (is_ocl2()) {
            return " -cl-std=CL2.0 ";
        }
        return "";
    }

    //From the IWOCL tutorial (needs attribution)
    std::string loadProgram(const char *input, size_t *len) {
        std::ifstream stream(input);
        if (!stream.is_open()) {
            std::cout << "Cannot open file: " << input << std::endl;
#if defined(_WIN32) && !defined(__MINGW32__)
            system("pause");
#endif
            exit(1);
        }

        std::string ret = std::string(
                std::istreambuf_iterator<char>(stream),
                (std::istreambuf_iterator<char>()));
        *len = ret.size();
        return ret;
    }

    int compile_kernel_string(
            std::string &kernel_string,
            const char *kernel_include,
            std::string kernel_defs) {
        DroidCL droidCL;
        int ret = CL_SUCCESS;
        size_t len = kernel_string.size();
        const char *source_c_str = kernel_string.c_str();
        exec_program = droidCL.clCreateProgramWithSource(
                exec_context, 1, (const char **) &source_c_str, &len, &ret);

        check_ocl(ret);

        std::stringstream options;
        options.setf(std::ios::fixed, std::ios::floatfield);

        // set compiler options here, example below
//        options << "-I" << kernel_include << " ";
//        options << kernel_defs;
        // Check if OpenCL 2.0
        options << check_ocl2x();
        options << get_vendor_option();

        //build the program
        ret = droidCL.clBuildProgram(
                exec_program, 1, &exec_device, options.str().c_str(), NULL, NULL);

        if (ret != CL_SUCCESS) {
            char buffer[2048];
            cl_program_build_info b_info = CL_PROGRAM_BUILD_LOG;
            droidCL.clGetProgramBuildInfo(exec_program, exec_device, b_info, 2048, buffer, NULL);
            return ret;
        }
        return ret;
    }

    //roughly from IWOCL tutorial (needs attribution)
    int
    compile_kernel(const char *kernel_file, const char *kernel_include, std::string kernel_defs) {

        int ret = CL_SUCCESS;
        //exec_program = cl::Program(cl::Context(exec_context), loadProgram(kernel_file), ret);
        size_t len;
        std::string source = (loadProgram(kernel_file, &len));
        const char *source_c_str = source.c_str();


        exec_program = clCreateProgramWithSource(exec_context, 1, (const char **) &source_c_str,
                                                 &len, &ret);

        check_ocl(ret);

        std::stringstream options;
        options.setf(std::ios::fixed, std::ios::floatfield);

        //set compiler options here, example below
        //options << " -cl-fast-relaxed-math";

        //Include the rt_device sources
        options << "-I" << kernel_include << " ";

        options << kernel_defs;


        //Check to see if we're OpenCL 2.0
        options << check_ocl2x();

        options << get_vendor_option();

        std::cout << "FLAGS: " << options.str() << std::endl;


        //build the program
        //ret = exec_program.build(options.str().c_str());
        ret = clBuildProgram(exec_program, 1, &exec_device, options.str().c_str(), NULL, NULL);
        //check_ocl(ret);

        if (ret != CL_SUCCESS) {
            char buffer[2048];
            cl_program_build_info b_info = CL_PROGRAM_BUILD_LOG;
            clGetProgramBuildInfo(exec_program, exec_device, b_info, 2048, buffer, NULL);
            std::string log(buffer);
            std::cerr << log << std::endl;
            //if(false) dump_program_binary(exec_program);
        }

        return ret;
    }

    void dump_program_binary(cl::Program &exec_program) {
        cl_int ret;

        std::vector<char *> binary = exec_program.getInfo<CL_PROGRAM_BINARIES>(&ret);
        if (ret != CL_SUCCESS) {
            std::cerr << "ERROR:" << ret << ": when obtaining program source code";
        }
        std::vector<char *>::iterator i;
        for (i = binary.begin(); i != binary.end(); i++) {
            printf("%s\n", *i);
        }
    }

    bool intel_subgroups() {
        char buffer[256];
        cl_device_info info = CL_DEVICE_EXTENSIONS;
        int err = 0;
        clGetDeviceInfo(exec_device, info, 256, buffer, NULL);
        //err = exec_device.getInfo(info, &buffer);
        check_ocl(err);
        std::string to_search(buffer);
        if (to_search.find("cl_intel_subgroups") == std::string::npos) {
            return false;
        }
        return true;
    }

    int get_compute_units(const char *kernel_name) {
        return get_SMs();
    }

    int my_max(int f, int s) {
        if (f > s)
            return f;
        return s;
    }

    size_t
    kernel_residency_approx(const char *kernel_name, const int block_size, const bool discovery,
                            int given = 0) {
        /* not portable, not guaranteed to work */

        /* generous upper bound. Maybe some GPUs might be higher under some configs?*/
        if (discovery) {
            return 512;
        }
        if (given != 0) {
            printf("RESIDENCY: WARNING: using a given occupancy of %d. This will not be portable!",
                   given);
            return given;
        }


        if (!discovery) {
            printf("RESIDENCY: WARNING: not using discovery, falling back to rough estimates based on compute units\n");
        }

        int compute_units = get_compute_units(kernel_name);
        int vendor_adjusted = 0;
        int factor = 256 / block_size;

        /* These are roughly based on the Table 3 in the paper:
           "Portable Inter-workgroup Barrier Synchronisation for GPUs"
           These are CONSERVATIVE and likely not tight. This will depend on local memory, register pressure, etc.
           If you can determine a better occupancy by hand, please pass it in as an arg.
        */
        if (is_AMD()) {
            vendor_adjusted =
                    (compute_units * 12) / 7;  // This is the ratio for R9, R7 allows more.
        } else if (is_Nvidia()) {
            vendor_adjusted = compute_units; // Maxwell does 2x, but Quadro kepler is 1x.
        } else if (is_Intel()) {
            vendor_adjusted = (compute_units + 1) / 8; // Works for iris and HD 5500
        } else if (is_ARM()) {
            vendor_adjusted = compute_units; // Works for t628 mali GPUs
        } else {
            printf("RESIDENCY: WARNING: unrecognized vendor, simply guessing residency is compute units\n");
            vendor_adjusted = compute_units;
        }

        printf("RESIDENCY: returning occupancy of %d\n", my_max(vendor_adjusted, 1) * factor);
        return my_max(vendor_adjusted, 1) * factor;
    }

};
