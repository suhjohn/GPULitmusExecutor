#if !(__OPENCL_C_VERSION__ >= 200) && defined(NVIDIA)
#include "nvidia_atomics.h"
#elif !(__OPENCL_C_VERSION__ >= 200) && !(defined(NVIDIA))
#include "custom_atomics/cl_1x_atomics.cl"
#endif

#define BARRIER
#define ID_SHUFFLE

#ifdef BARRIER
void test_barrier(__global atomic_int *x) {
int max_iters = 1000;
int iters = 0;
int val = atomic_fetch_add_explicit(x, 1, memory_order_relaxed, memory_scope_device);
while (iters < max_iters && val != 2) {
val = atomic_load_explicit(x, memory_order_relaxed, memory_scope_device);
iters += 1;
}
return;
}
#else
void test_barrier(__global atomic_int *x) {
    return;
  }
#endif

#ifdef ID_SHUFFLE
#define TEST_THREAD_0 shuffled_ids[get_global_id(0)] == get_local_size(0) * 0
#define TEST_THREAD_1 shuffled_ids[get_global_id(0)] == get_local_size(0) * 1
#define TEST_THREAD_2 shuffled_ids[get_global_id(0)] == get_local_size(0) * 2
#define TEST_THREAD_3 shuffled_ids[get_global_id(0)] == get_local_size(0) * 3
#define TESTING_WARP ((shuffled_ids[get_global_id(0)] > get_local_size(0) * 0) && (shuffled_ids[get_global_id(0)] < get_local_size(0) * 0 + dwarp_size)) || ((shuffled_ids[get_global_id(0)] > get_local_size(0) * 1) && (shuffled_ids[get_global_id(0)] < get_local_size(0) * 1 + dwarp_size))
#else
#define TEST_THREAD_0 lid == 0 && wgid == 0
#define TEST_THREAD_1 lid == 0 && wgid == 1
#define TEST_THREAD_2 lid == 0 && wgid == 2
#define TEST_THREAD_3 lid == 0 && wgid == 3
#endif

#ifndef MEM_STRESS
#define MEM_STRESS 1
#endif

#ifndef STRESS_ITERATIONS
#define STRESS_ITERATIONS 512
#endif

#ifndef STRESS_PATTERN
#define STRESS_PATTERN 0
#endif
