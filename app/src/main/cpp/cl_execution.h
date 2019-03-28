#pragma once
#define CL_HPP_TARGET_OPENCL_VERSION 200
#include <CL/cl.hpp>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <assert.h>
#include <limits.h>

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
  cl::Device exec_device;
  cl::Context exec_context;
  cl::CommandQueue exec_queue;
  cl::Program exec_program;
  std::map<std::string, cl::Kernel> exec_kernels;
  cl::Platform exec_platform;

private:
  // For recompiling the kernel with the correct subgroup size
  const char* cache_kernel_file;
  const char * cache_kernel_include;
  bool cache_slots;
  bool cache_discovery;
  bool cache_irgl_subgroups;
  int cache_sub_group_size;

public:
  
  //From IWOCL tutorial (needs attribution)
  static std::string getDeviceName(const cl::Device& device) {
    std::string name;
    cl_device_info info = CL_DEVICE_NAME;
    
    // Special case for AMD
#ifdef CL_DEVICE_BOARD_NAME_AMD
    device.getInfo(CL_DEVICE_VENDOR, &name);
    if (strstr(name.c_str(), "Advanced Micro Devices"))
      info = CL_DEVICE_BOARD_NAME_AMD;
#endif
    
    device.getInfo(info, &name);
    return name;
  }

  static std::string getDriverVersion(const cl::Device& device) {
    std::string version;
    cl_device_info info = CL_DRIVER_VERSION;
    device.getInfo(info, &version);
    return version;
  }
  
  std::string getExecDeviceName() {
    return getDeviceName(exec_device);
  }

  std::string getExecDriverVersion() {
    return getDriverVersion(exec_device);
  }
  
  int get_SMs() {
    cl_uint ret;
    cl_device_info info = CL_DEVICE_MAX_COMPUTE_UNITS;
    
    // Maybe special case it for Intel?
    //#ifdef CL_DEVICE_BOARD_NAME_AMD
    //device.getInfo(CL_DEVICE_VENDOR, &name);
    //if (strstr(name.c_str(), "Advanced Micro Devices"))
    //  info = CL_DEVICE_BOARD_NAME_AMD;
    //#endif
    
    exec_device.getInfo(info, &ret);
    return ret;
  }
  
  bool is_Nvidia() {
    std::string buffer;   
    cl_device_info info = CL_DEVICE_VENDOR;
    int err = 0;
    err = exec_device.getInfo(info, &buffer);
    check_ocl(err);
    if (buffer.find("NVIDIA Corporation") == std::string::npos) {
      return false;
    }
    return true;
  }

  bool is_Intel() {
	  std::string buffer;
	  cl_device_info info = CL_DEVICE_VENDOR;
	  int err = 0;
	  err = exec_device.getInfo(info, &buffer);
	  check_ocl(err);
	  if (buffer.find("Intel") == std::string::npos) {
		  return false;
	  }
	  return true;
  }

  bool is_AMD() {
	  std::string buffer;
	  cl_device_info info = CL_DEVICE_VENDOR;
	  int err = 0;
	  err = exec_device.getInfo(info, &buffer);
	  check_ocl(err);
	  if (buffer.find("Advanced Micro Devices") == std::string::npos) {
		  return false;
	  }
	  return true;
  }

  bool is_ARM() {
    // Tyler TODO
    std::string buffer;
    cl_device_info info = CL_DEVICE_VENDOR;
    int err = 0;
    err = exec_device.getInfo(info, &buffer);
    check_ocl(err);
    if (buffer.find("ARM") == std::string::npos) {
      return false;
    }
    return true;
  }
  
  bool is_ocl2() {
    std::string buffer;   
    cl_device_info info = CL_DEVICE_VERSION;
    int err = 0;
    err = exec_device.getInfo(info, &buffer);
    check_ocl(err);
    
    if (buffer.find("OpenCL 2.") == std::string::npos) {
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
  std::string loadProgram(const char* input) {
    std::ifstream stream(input);
    if (!stream.is_open()) {
      std::cout << "Cannot open file: " << input << std::endl;
#if defined(_WIN32) && !defined(__MINGW32__)
      system("pause");
#endif
      exit(1);
    }
    
    return std::string(
		       std::istreambuf_iterator<char>(stream),
		       (std::istreambuf_iterator<char>()));
  }
  
  
  //roughly from IWOCL tutorial (needs attribution)
  int compile_kernel(const char* kernel_file, const char * kernel_include) {

    cache_kernel_file = kernel_file;
    cache_kernel_include = kernel_include;

    int ret = CL_SUCCESS;
    exec_program = cl::Program(exec_context, loadProgram(kernel_file));
    
    std::stringstream options;
    options.setf(std::ios::fixed, std::ios::floatfield);
    
    //set compiler options here, example below 
    //options << " -cl-fast-relaxed-math";
    
    //Include the rt_device sources
    options << "-I" << kernel_include << " ";

        
    //Check to see if we're OpenCL 2.0
    options << check_ocl2x();

	  //Check to see if we're OpenCL 2.0
	  options << get_vendor_option();
    
    std::cout << "FLAGS: " << options.str() << std::endl;


    //build the program
    ret = exec_program.build(options.str().c_str());
    
    if (ret != CL_SUCCESS) {
      std::string log = exec_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(exec_device);
      std::cerr << log << std::endl;

      if(false) dump_program_binary(exec_program);
    }

    return ret;        
  }

  void dump_program_binary(cl::Program &exec_program) {
    cl_int ret;

    std::vector<char*> binary = exec_program.getInfo<CL_PROGRAM_BINARIES>(&ret);
    if(ret != CL_SUCCESS) {
      std::cerr << "ERROR:" << ret << ": when obtaining program source code";
    }
    std::vector<char*>::iterator i;
    for(i = binary.begin(); i != binary.end(); i++) {
      printf("%s\n", *i);
    }
  }

  bool intel_subgroups() {
    std::string buffer;
    cl_device_info info = CL_DEVICE_EXTENSIONS;
    int err = 0;
    err = exec_device.getInfo(info, &buffer);
    check_ocl(err);
    if (buffer.find("cl_intel_subgroups") == std::string::npos) {
      return false;
    }
    return true;
  }

  bool khr_subgroups() {
    std::string buffer;
    cl_device_info info = CL_DEVICE_EXTENSIONS;
    int err = 0;
    err = exec_device.getInfo(info, &buffer);
    check_ocl(err);
    if (buffer.find("cl_khr_subgroups") == std::string::npos) {
      return false;
    }
    return true;
  }

  bool at_least_cl21() {
    std::string buffer;
    cl_device_info info = CL_DEVICE_VERSION;
    int err = 0;
    err = exec_device.getInfo(info, &buffer);
    check_ocl(err);

    // Check if its not 1.0, 1.1, 1.2, 2.0
    if (buffer.find("OpenCL 1.0") != std::string::npos   ||
        buffer.find("OpenCL 1.1") != std::string::npos   ||
        buffer.find("OpenCL 1.2") != std::string::npos   ||
        buffer.find("OpenCL 2.0") != std::string::npos) {
      return false;
    }
    return true;
  }

  int get_compute_units(const char* kernel_name) {
    return get_SMs();
  }

  int my_max(int f, int s) {
    if (f > s)
      return f;
   return s;
  }

  size_t kernel_residency_approx(const char *kernel_name, const int block_size, const bool discovery, int given = 0) {
    /* not portable, not guaranteed to work */

    /* generous upper bound. Maybe some GPUs might be higher under some configs?*/
    if (discovery) {
      return 512;
    }
    if (given != 0) {
      printf("RESIDENCY: WARNING: using a given occupancy of %d. This will not be portable!", given);
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
      vendor_adjusted = (compute_units * 12) / 7;  // This is the ratio for R9, R7 allows more.
    }
    else if (is_Nvidia()) {
      vendor_adjusted = compute_units; // Maxwell does 2x, but Quadro kepler is 1x.
    }
    else if (is_Intel()) {
      vendor_adjusted = (compute_units + 1) / 8; // Works for iris and HD 5500
    }
    else if (is_ARM()) {
      vendor_adjusted = compute_units; // Works for t628 mali GPUs
    }
    else {
      printf("RESIDENCY: WARNING: unrecognized vendor, simply guessing residency is compute units\n");
      vendor_adjusted = compute_units;
    }

    printf("RESIDENCY: returning occupancy of %d\n", my_max(vendor_adjusted, 1) * factor);
    return my_max(vendor_adjusted,1) * factor;
  }

  void check_kernel_wg_sizes(cl::Kernel k, std::string name, int wg_size) {
    size_t ret_val = 0;
    clGetKernelWorkGroupInfo(k(), exec_device(), CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &ret_val, NULL);
    if (ret_val < wg_size) {
      std::cout << "WG_SIZE_ERROR: kernel " << name << " can only have a workgroup size up to " << ret_val << std::endl;
      std::cout << "bug a size of " << wg_size << " was requested" << std::endl;
    }
  }
};
