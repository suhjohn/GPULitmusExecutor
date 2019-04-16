#ifndef ANDROID_OPENCLEXAMPLE_DROIDCL_H
#define ANDROID_OPENCLEXAMPLE_DROIDCL_H

#include <CL/cl.h>
#include <dlfcn.h>

using namespace std;

class DroidCL {
private:
    void *openCLHndl;

    cl_int (*DroidclGetPlatformIDs)(
            cl_uint, cl_platform_id *, cl_uint *);

    cl_int (*DroidclGetDeviceIds)(
            cl_platform_id, cl_device_type, cl_uint, cl_device_id *, cl_uint *);

    cl_int (*DroidclGetDeviceInfo)(
            cl_device_id, cl_device_info, size_t, void *, size_t *);

    cl_program (*DroidclCreateProgramWithSource)(
            cl_context, cl_uint, const char **, const size_t *, cl_int *);

    cl_int
    (*DroidclBuildProgram)(cl_program, cl_uint, const cl_device_id *, const char *,
                           void (*pfn_notify)(cl_program, void *user_data), void *);

    cl_int
    (*DroidclGetProgramBuildInfo)(
            cl_program, cl_device_id, cl_program_build_info, size_t, void *, size_t *);

    static string default_so_paths[7];
public:
    DroidCL() {
        for (const auto &path : default_so_paths) {
            openCLHndl = dlopen(path.c_str(), RTLD_NOW);
            if (openCLHndl != NULL) {
                *(void **) (&DroidclGetPlatformIDs) = dlsym(
                        openCLHndl, "clGetPlatformIDs");
                *(void **) (&DroidclGetDeviceIds) = dlsym(
                        openCLHndl, "clGetDeviceIDs");
                *(void **) (&DroidclGetDeviceInfo) = dlsym(
                        openCLHndl, "clGetDeviceInfo");
                *(void **) (&DroidclCreateProgramWithSource) = dlsym(
                        openCLHndl, "clCreateProgramWithSource");
                *(void **) (&DroidclBuildProgram) = dlsym(
                        openCLHndl, "clBuildProgram");
                *(void **) (&DroidclGetProgramBuildInfo) = dlsym(
                        openCLHndl, "clGetProgramBuildInfo");
            }
        }
    }

    cl_int clGetPlatformIDs(cl_uint num_entries,
                            cl_platform_id *platforms,
                            cl_uint *num_platforms) {
        return (*DroidclGetPlatformIDs)(100, platforms, num_platforms);
    }

    cl_int clGetDeviceIDs(cl_platform_id platform,
                          cl_device_type device_type,
                          cl_uint num_entries,
                          cl_device_id *devices,
                          cl_uint *num_devices) {
        return (*DroidclGetDeviceIds)(platform, device_type, num_entries, devices, num_devices);
    }

    cl_int clGetDeviceInfo(cl_device_id device,
                           cl_device_info param_name,
                           size_t param_value_size,
                           void *param_value,
                           size_t *param_value_size_ret) {
        return (*DroidclGetDeviceInfo)(
                device, param_name, param_value_size, param_value,
                param_value_size_ret);
    }

    cl_program clCreateProgramWithSource(cl_context context,
                                         cl_uint count,
                                         const char **strings,
                                         const size_t *lengths,
                                         cl_int *errcode_ret) {
        return (*DroidclCreateProgramWithSource)(context, count, strings, lengths, errcode_ret);
    }

    cl_int clBuildProgram(cl_program program,
                          cl_uint num_devices,
                          const cl_device_id *device_list,
                          const char *options,
                          void (*pfn_notify)(cl_program, void *user_data),
                          void *user_data) {
        return (*DroidclBuildProgram)(
                program, num_devices, device_list, options, pfn_notify, user_data);
    }

    cl_int clGetProgramBuildInfo(cl_program program,
                                 cl_device_id device,
                                 cl_program_build_info param_name,
                                 size_t param_value_size,
                                 void *param_value,
                                 size_t *param_value_size_ret) {
        return (*DroidclGetProgramBuildInfo)(
                program, device, param_name, param_value_size, param_value, param_value_size_ret);
    }

};

string DroidCL::default_so_paths[7] = {
        "/system/lib/libOpenCL.so",
        "/system/lib64/libOpenCL.so",
        "/system/vendor/lib/libOpenCL.so",
        "/system/vendor/lib64/libOpenCL.so",
        "/system/vendor/lib64/libGLES_mali.so",
        "/system/vendor/lib/libPVROCL.so",
        "/data/data/org.pocl.libs/files/lib/libpocl.so",
};

#endif //ANDROID_OPENCLEXAMPLE_DROIDCL_H
