 #include "testing_common.h"


__kernel void litmus_test(
  __global atomic_uint *ga /* global, atomic locations */,
  __global int *gn /* global, non-atomic locations */,
  __global int *out, /* output */
  __global int *shuffled_ids
) {
  int lid = get_local_id(0);
  int wgid = get_group_id(0);

  if (TEST_THREAD_0) {
    // Work-item 0 in workgroup 0:
    test_barrier(&(ga[1023]));
    out[0] = atomic_load_explicit(&ga[1], memory_order_relaxed, memory_scope_device);
    atomic_store_explicit(&ga[0], 1, memory_order_relaxed, memory_scope_device);
    
  } else if (TEST_THREAD_1) {
    // Work-item 0 in workgroup 1:               
    test_barrier(&(ga[1023]));
    out[1] = atomic_load_explicit(&ga[0], memory_order_relaxed, memory_scope_device);
    atomic_store_explicit(&ga[1], 1, memory_order_relaxed, memory_scope_device);
  }
}

__kernel void check_outputs(__global int *output, __global int *result) {
  
  if (get_global_id(0) == 0) {
    int r1 = output[0];
    int r2 = output[1];
    if (r1 == 0 && r2 == 0) {
      *result = 0;
    }
    else if (r1 == 0 && r2 == 0) {
      *result = 0;
    }
    else if (r1 == 1 && r2 == 0) {
      *result = 1;
    }
    else if (r1 == 0 && r2 == 1) {
      *result = 2;
    }
    else if (r1 == 1 && r2 == 1) {
      *result = 3;
    }
    else {
      *result = 4;
    }
  }
}
