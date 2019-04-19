#ifndef ANDROID_OPENCLEXAMPLE_DROIDCL_H
#define ANDROID_OPENCLEXAMPLE_DROIDCL_H

#include <CL/cl.h>
#include <string>
#include <dlfcn.h>

using namespace std;

class DroidCL {
private:
    void *openCLHndl;
    static string default_so_paths[7];

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

    cl_context (*DroidclCreateContext)(
            cl_context_properties *, cl_uint, const cl_device_id *, void *,
            void *, cl_int *);

    cl_command_queue (*DroidclCreateCommandQueue)(
            cl_context, cl_device_id, cl_command_queue_properties, cl_int *
    );

    cl_kernel (*DroidclCreateKernel)(cl_program, const char *, cl_int *);

    cl_mem (*DroidclCreateBuffer)(cl_context, cl_mem_flags, size_t, void *, cl_int *);

    cl_int (*DroidclSetKernelArg)(cl_kernel, cl_uint, size_t, const void *);

    cl_int (*DroidclEnqueueWriteBuffer)(
            cl_command_queue, cl_mem, cl_bool, size_t, size_t,
            const void *, cl_uint, const cl_event *, cl_event *);

    cl_int (*DroidclEnqueueNDRangeKernel)(
            cl_command_queue, cl_kernel, cl_uint, const size_t *, const size_t *, const size_t *,
            cl_uint, const cl_event *, cl_event *);

    cl_int (*DroidclEnqueueReadBuffer)(
            cl_command_queue, cl_mem, cl_bool, size_t, size_t, void *, cl_uint, const cl_event *,
            cl_event *);
    cl_int (*DroidclFinish)(cl_command_queue command_queue);

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
                *(void **) (&DroidclCreateContext) = dlsym(
                        openCLHndl, "clCreateContext");
                *(void **) (&DroidclCreateCommandQueue) = dlsym(
                        openCLHndl, "clCreateCommandQueue");
                *(void **) (&DroidclCreateKernel) = dlsym(
                        openCLHndl, "clCreateKernel");
                *(void **) (&DroidclCreateBuffer) = dlsym(
                        openCLHndl, "clCreateBuffer");
                *(void **) (&DroidclSetKernelArg) = dlsym(
                        openCLHndl, "clSetKernelArg");
                *(void **) (&DroidclEnqueueWriteBuffer) = dlsym(
                        openCLHndl, "clEnqueueWriteBuffer");
                *(void **) (&DroidclEnqueueNDRangeKernel) = dlsym(
                        openCLHndl, "clEnqueueNDRangeKernel");
                *(void **) (&DroidclEnqueueReadBuffer) = dlsym(
                        openCLHndl, "clEnqueueReadBuffer");
                *(void **) (&DroidclFinish) = dlsym(
                        openCLHndl, "clFinish");
            }
        }
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

    cl_mem clCreateBuffer(cl_context context,
                          cl_mem_flags flags,
                          size_t size,
                          void *host_ptr,
                          cl_int *errcode_ret) {
        return (*DroidclCreateBuffer)(context, flags, size, host_ptr, errcode_ret);
    }

    cl_context clCreateContext(cl_context_properties *properties,
                               cl_uint num_devices,
                               const cl_device_id *devices,
                               void *pfn_notify,
                               void *user_data,
                               cl_int *errcode_ret) {
        return (*DroidclCreateContext)(
                properties, num_devices, devices, pfn_notify, user_data, errcode_ret);
    }

    cl_command_queue clCreateCommandQueue(cl_context context,
                                          cl_device_id device,
                                          cl_command_queue_properties properties,
                                          cl_int *errcode_ret) {
        return (*DroidclCreateCommandQueue)(
                context, device, properties, errcode_ret);
    }


    cl_kernel clCreateKernel(cl_program program,
                             const char *kernel_name,
                             cl_int *errcode_ret) {
        return (*DroidclCreateKernel)(program, kernel_name, errcode_ret);
    }


    cl_program clCreateProgramWithSource(cl_context context,
                                         cl_uint count,
                                         const char **strings,
                                         const size_t *lengths,
                                         cl_int *errcode_ret) {
        return (*DroidclCreateProgramWithSource)(context, count, strings, lengths, errcode_ret);
    }

    cl_int clGetPlatformIDs(cl_uint num_entries,
                            cl_platform_id *platforms,
                            cl_uint *num_platforms) {
        return (*DroidclGetPlatformIDs)(num_entries, platforms, num_platforms);
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

    cl_int clGetProgramBuildInfo(cl_program program,
                                 cl_device_id device,
                                 cl_program_build_info param_name,
                                 size_t param_value_size,
                                 void *param_value,
                                 size_t *param_value_size_ret) {
        return (*DroidclGetProgramBuildInfo)(
                program, device, param_name, param_value_size, param_value, param_value_size_ret);
    }

    cl_int clSetKernelArg(cl_kernel kernel,
                          cl_uint arg_index,
                          size_t arg_size,
                          const void *arg_value) {
        return (*DroidclSetKernelArg)(kernel, arg_index, arg_size, arg_value);
    }

    cl_int clEnqueueWriteBuffer(cl_command_queue command_queue,
                                cl_mem buffer,
                                cl_bool blocking_write,
                                size_t offset,
                                size_t cb,
                                const void *ptr,
                                cl_uint num_events_in_wait_list,
                                const cl_event *event_wait_list,
                                cl_event *event) {
        return (*DroidclEnqueueWriteBuffer)(command_queue, buffer, blocking_write, offset, cb, ptr,
                                            num_events_in_wait_list, event_wait_list, event);
    }

    cl_int clEnqueueNDRangeKernel(cl_command_queue command_queue,
                                  cl_kernel kernel,
                                  cl_uint work_dim,
                                  const size_t *global_work_offset,
                                  const size_t *global_work_size,
                                  const size_t *local_work_size,
                                  cl_uint num_events_in_wait_list,
                                  const cl_event *event_wait_list,
                                  cl_event *event) {
        return (*DroidclEnqueueNDRangeKernel)(
                command_queue, kernel, work_dim, global_work_offset, global_work_size,
                local_work_size, num_events_in_wait_list, event_wait_list, event);
    }

    cl_int clEnqueueReadBuffer(cl_command_queue command_queue,
                               cl_mem buffer,
                               cl_bool blocking_read,
                               size_t offset,
                               size_t cb,
                               void *ptr,
                               cl_uint num_events_in_wait_list,
                               const cl_event *event_wait_list,
                               cl_event *event) {
        return (*DroidclEnqueueReadBuffer)(
                command_queue, buffer, blocking_read, offset, cb, ptr, num_events_in_wait_list,
                event_wait_list, event
        );
    }
    cl_int clFinish (	cl_command_queue command_queue){
        return (*DroidclFinish)(command_queue);
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
