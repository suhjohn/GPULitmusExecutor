
#define CL_HPP_TARGET_OPENCL_VERSION 200
#include "stdio.h"
#include <vector>
#include <iostream>
#include <CL/cl.hpp>
#ifdef _WIN32
#include "getopt_win.h"
#endif
#include "getopt.h"
#include "cl_execution.h"
#include <chrono>
#include <fstream>
#include <algorithm>
#include <stdlib.h>

#define SIZE 1024
#define NANOSEC 1000000000LL

// To compile: nvcc .\main.c -lOpenCL

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

//This needs to be INPUT_FILE for Windows
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
  ChipConfig defaultChipConfig = { 256, 1, 32 };
  ChipConfig Nvidia940M = { 1024, 1, 16 };
  ChipConfig IntelHD5500 = { 256, 1, 16 };
  ChipConfig Inteli75600u = { 4, 1, 4 };

  ChipConfigMaps["default"] = defaultChipConfig;
  ChipConfigMaps["Intel(R) Core(TM) i7-5600U CPU @ 2.60GHz"] = Inteli75600u;
  ChipConfigMaps["Intel(R) HD Graphics 5500"] = IntelHD5500;
  ChipConfigMaps["GeForce 940M"] = Nvidia940M;
}

TestConfig parse_config_file(std::string config_file) {
  std::ifstream infile(config_file);
  TestConfig ret;
  std::string title, ignore;
  std::getline(infile, title);
  std::cout << "parsing config for test: " << title << std::endl;
  std::getline(infile, ignore); // "results"
  std::getline(infile, ignore);
  ret.hist_size = std::stoi(ignore);

  //infile >> ret.hist_size;
  for (int i = 0; i < ret.hist_size; i++) {
    std::string out_desc;
    std::getline(infile, out_desc);
    ret.hist_strings.push_back(out_desc);
    //std::cout << "parsed line: " << out_desc << std::endl;
  }
  ret.hist_strings.push_back("errors: ");
  ret.hist_size++;
  std::getline(infile, ignore); // "num outputs"
  std::getline(infile, ignore);
  ret.output_size = std::stoi(ignore);
  //std::cout << "output size: " << ret.output_size << std::endl;

  return ret;
}

//From IWOCL tutorial (needs attribution)
unsigned getDeviceList(std::vector<std::vector<cl::Device> >& devices)
{
  // Get list of platforms
  std::vector<cl::Platform> platforms;
  cl::Platform::get(&platforms);

  // Enumerate devices
  for (unsigned int i = 0; i < platforms.size(); i++)
    {
      std::vector<cl::Device> plat_devices;
      platforms[i].getDevices(CL_DEVICE_TYPE_ALL, &plat_devices);
      devices.push_back(plat_devices);
      //devices.insert(devices.end(), plat_devices.begin(), plat_devices.end());
    }

  return devices.size();
}

//From IWOCL tutorial (needs attribution)
void list_devices() {

  std::vector<std::vector<cl::Device> > devices;
  getDeviceList(devices);

  // Print device names
  if (devices.size() == 0) {
    std::cout << "No devices found." << std::endl;
  }
  else {
    std::cout << std::endl;
    std::cout << "Platform,Devices:" << std::endl;
    for (unsigned j = 0; j < devices.size(); j++) {
      for (unsigned i = 0; i < devices[j].size(); i++) {
	std::cout << j << ", " <<  i << ": " << CL_Execution::getDeviceName(devices[j][i]) << std::endl;
      }
    }
  }
}

void usage(int argc, char *argv[]) {
  fprintf(stderr, "usage: %s [-l] [-q] [-p platform_id] [-d device_id] [-o output-file] litmus-test-directory\n", argv[0]);
}

void parse_args(int argc, char *argv[])
{
  int c;
  const char *skel_opts = "p:d:i:clqo:";
  char *opts;
  int len = 0;
  char *end;

  len = strlen(skel_opts) + 1;
  opts = (char *) calloc(len, sizeof(char));
  if(!opts) {
    fprintf(stderr, "Unable to allocate memory\n");
    exit(EXIT_FAILURE);
  }

  strcat(opts, skel_opts);

  while((c = getopt(argc, argv, opts)) != -1) {
    switch(c)
      {
      case 'q':
	QUIET = 1;
	break;
      case 'c':
        USE_CHIP_CONFIG = 1;
        break;
      case 'o':
	OUTPUT = optarg; //TODO: copy?
	break;
      case 'i':
        errno = 0;
        ITERATIONS = strtol(optarg, &end, 10);
        if (errno != 0 || *end != '\0') {
          fprintf(stderr, "Invalid iterations '%s'. An integer must be specified.\n", optarg);
          exit(EXIT_FAILURE);
        }
        break;
      case 'l':
	LIST = 1; //TODO: copy?
	break;
      case 'p':
	errno = 0;
	PLATFORM_ID = strtol(optarg, &end, 10);
	if(errno != 0 || *end != '\0') {
	  fprintf(stderr, "Invalid platform id device '%s'. An integer must be specified.\n", optarg);
	  exit(EXIT_FAILURE);
	}
	break;
      case 'd':
	errno = 0;
	DEVICE_ID = strtol(optarg, &end, 10);
	if(errno != 0 || *end != '\0') {
	  fprintf(stderr, "Invalid device id device '%s'. An integer must be specified.\n", optarg);
	  exit(EXIT_FAILURE);
	}
	break;
      case '?':
	usage(argc, argv);
	exit(EXIT_FAILURE);
	break;

    }
  }

  if(LIST != 1) {
    if(optind < argc) {
      INPUT_FILE = argv[optind];
    }
    else {
      usage(argc, argv);
      exit(EXIT_FAILURE);
    }
  }
  free(opts);
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

int main(int argc, char *argv[]) {

  int err = 0;
  CL_Execution exec;


  if(argc == 1) {
    usage(argc, argv);
    exit(1);
  }

  parse_args(argc, argv);

  if (LIST == 1) {
    list_devices();
    exit(0);
  }

  std::vector<std::vector<cl::Device> > devices;
  getDeviceList(devices);

  if (PLATFORM_ID >= devices.size()) {
    printf("ERROR: Invalid platform id (%d). Please use the -l option to view platforms and device ids.\n", PLATFORM_ID);
    exit(1);
  }

  if (DEVICE_ID >= devices[PLATFORM_ID].size()) {
    printf("ERROR: Invalid device id (%d). Please use the -l option to view platforms and device ids.\n", DEVICE_ID);
    exit(1);
  }

  populate_ChipConfigMaps();
  exec.exec_device = devices[PLATFORM_ID][DEVICE_ID];

  printf("Using Device: %s\n", exec.getExecDeviceName().c_str());
  printf("Driver Version: %s\n", exec.getExecDriverVersion().c_str());
  ChipConfig cConfig;
  if (USE_CHIP_CONFIG) {
    printf("using chip config: %s\n", exec.getExecDeviceName().c_str());
    cConfig = ChipConfigMaps[exec.getExecDeviceName().c_str()];
    }
  else {
    printf("using chip config: default\n");
    cConfig = ChipConfigMaps["default"];
  }

  cl::Context context(exec.exec_device);
  exec.exec_context = context;
  cl::CommandQueue queue(exec.exec_context, exec.exec_device);
  exec.exec_queue = queue;

  std::string kernel_file = INPUT_FILE + "/kernel.cl";
  std::string test_config_file = INPUT_FILE + "/config.txt";

  TestConfig cfg = parse_config_file(test_config_file);

  printf("Kernel file: %s, Kernel Include path: %s\n", kernel_file.c_str(), kernel_include.c_str());
  err = exec.compile_kernel(kernel_file.c_str(), kernel_include.c_str());
  check_ocl(err);


  int occupancy = cConfig.occupancy_est;
  int max_local_size = cConfig.max_local_size;

  int max_global_size = max_local_size * occupancy;
  cl::NDRange sglobalND(1), slocalND(1);
  err = get_kernels(exec);
  check_ocl(err);

  // Actual real stuff starts

  cl::Buffer dga(exec.exec_context, CL_MEM_READ_WRITE, sizeof(cl_int) *(SIZE));
  cl::Buffer dgna(exec.exec_context, CL_MEM_READ_WRITE, sizeof(cl_int) *(SIZE));
  cl::Buffer doutput(exec.exec_context, CL_MEM_READ_WRITE, sizeof(cl_int) *(cfg.output_size));
  cl::Buffer dresult(exec.exec_context, CL_MEM_READ_WRITE, sizeof(cl_int) *(1));
  cl::Buffer dshuffled_ids(exec.exec_context, CL_MEM_READ_WRITE, sizeof(cl_int) *(max_global_size));

  cl_int hga[SIZE], hgna[SIZE], houtput[SIZE];
  cl_int hresult;
  cl_int *hshuffled_ids = (cl_int *)malloc(sizeof(cl_int)*max_global_size);

  int *histogram = (int *)malloc(sizeof(int) * cfg.hist_size);
  std::cout << "hist size: " << cfg.hist_size << std::endl;

  for (int i = 0; i < max_global_size; i++) {
    hshuffled_ids[i] = i;
  }

  for (int i = 0; i < cfg.hist_size; i++) {
    histogram[i] = 0;
  }


  for (int i = 0; i < SIZE; i++) {
    hga[i] = hgna[i] = houtput[i] = 0;
  }

  check_ocl(exec.exec_kernels["litmus_test"].setArg(0, dga));
  check_ocl(exec.exec_kernels["litmus_test"].setArg(1, dgna));
  check_ocl(exec.exec_kernels["litmus_test"].setArg(2, doutput));
  check_ocl(exec.exec_kernels["litmus_test"].setArg(3, dshuffled_ids));


  check_ocl(exec.exec_kernels["check_outputs"].setArg(0, doutput));
  check_ocl(exec.exec_kernels["check_outputs"].setArg(1, dresult));

  auto now = std::chrono::high_resolution_clock::now();
  unsigned long long begin_time, end_time, time;
  begin_time = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();

  for (int i = 0; i < ITERATIONS; i++) {

    // set up ids
    int local_size = rand() % max_local_size + cConfig.min_local_size;
    int global_size = occupancy * local_size;

    cl::NDRange globalND(global_size), localND(local_size);
    for (int i = 0; i < global_size; i++) {
      hshuffled_ids[i] = i;
    }

    std::random_shuffle(hshuffled_ids, &hshuffled_ids[global_size - 1]);
    err = exec.exec_queue.enqueueWriteBuffer(dshuffled_ids, CL_TRUE, 0, sizeof(cl_int) *(global_size), hshuffled_ids);
    check_ocl(err);
    err = exec.exec_queue.enqueueWriteBuffer(dga, CL_TRUE, 0, sizeof(cl_int) *(SIZE), hga);
    check_ocl(err);
    err = exec.exec_queue.enqueueWriteBuffer(dgna, CL_TRUE, 0, sizeof(cl_int) *(SIZE), hgna);
    check_ocl(err);
    err = exec.exec_queue.enqueueWriteBuffer(doutput, CL_TRUE, 0, sizeof(cl_int) *(cfg.output_size), houtput);
    check_ocl(err);
    err = exec.exec_queue.finish();
    check_ocl(err);
    err = exec.exec_queue.enqueueNDRangeKernel(exec.exec_kernels["litmus_test"], cl::NullRange, globalND, localND);
    check_ocl(err);
    err = exec.exec_queue.finish();
    check_ocl(err);
    err = exec.exec_queue.enqueueNDRangeKernel(exec.exec_kernels["check_outputs"], cl::NullRange, sglobalND, slocalND);
    check_ocl(err);
    err = exec.exec_queue.enqueueReadBuffer(dresult, CL_TRUE, 0, sizeof(cl_int) *(1), &hresult);
    check_ocl(err);
    histogram[hresult]++;
  }
  now = std::chrono::high_resolution_clock::now();
  end_time = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
  time = end_time - begin_time;
  float time_float = static_cast< float >(time) / static_cast< float >(NANOSEC);

  std::cout << std::endl << "RESULTS: " << std::endl;
  std::cout << "-------------------" << std::endl;
  for (int i = 0; i < cfg.hist_size; i++) {
    std::cout << cfg.hist_strings[i] << histogram[i] << std::endl;
  }
  std::cout << std::endl;
  std::cout << "RATES" << std::endl;
  std::cout << "-------------------" << std::endl;
  std::cout << "tests          : " << ITERATIONS << std::endl;
  std::cout << "time (seconds) : " << time_float << std::endl;
  std::cout << "tests/sec      : " << static_cast< float >(ITERATIONS)/ time_float << std::endl;
  std::cout << std::endl;

  //free(hga);
  //free(hgna);
  //free(houtput);

  fflush(stderr);
  return 0;
}




