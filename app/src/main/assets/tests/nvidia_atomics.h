#pragma once

//A (very small) subset of the opencl 2.0  atomics for Nvidia. 
//Won't work for Fermi or earlier architectures.

//Known to be unsound on Kepler because of CORR behaviours. But 
//since our idioms don't use CORR it should be
//okay. Otherwise we need an expensive fence for load relaxed.

//Only one variant of each instruction is supported!! Please Check

typedef volatile int atomic_int;
typedef volatile uint atomic_uint;


// Added sub_group scope, but don't check for anywhere (relaxed accesses don't have fences anyways)
typedef enum {memory_scope_device, memory_scope_sub_group, memory_scope_work_group} memory_scope;
typedef enum {memory_order_relaxed, memory_order_release, memory_order_acquire, memory_order_acq_rel} memory_order;

//This is atomic_fetch_add_explicit(v,o, memory_order_acq_rel, memory_scope_device)
int atomic_fetch_add_explicit(__global volatile atomic_int* target, int operand, memory_order mo, memory_scope ms) {  
  int ret = 0;

  asm volatile ("membar.gl;\n");

  ret =  atomic_add(target, operand);
  
  asm volatile ("membar.gl;\n");

  return ret;  
}

//This is atomic_store_explicit(v,v, memory_order_release, memory_scope_device
void atomic_store_explicit(__global volatile atomic_int* target, int val, const memory_order mo, const memory_scope ms) {

    //asm volatile ("membar.gl;\n");
  
  *target = val;  
}

//This is atomic_load_explict(v, memory_order_relaxed, memory_scope_device)
int atomic_load_explicit(__global volatile atomic_int* target, const memory_order mo, const memory_scope ms) {
  int ret = 0;
  
  ret = *target;

  return ret;
}

//This is atomic_work_item_fence(memory_order_acquire, memory_scope_device)
void atomic_work_item_fence(const cl_mem_fence_flags flags, const memory_order mo, const memory_scope ms) {
  asm volatile ("membar.gl;\n");
}

//This is atomic_exchange_explicit(v, v, v, memory_order_acquire, memory_scope_device)
int atomic_exchange_explicit(__global volatile atomic_int* target, const int desired, const memory_order mo, const memory_scope ms) {
  int old = 0;

  old = atomic_xchg(target, desired);
    
  asm volatile ("membar.gl;\n");

  return old;
}
