#include "utils.h"

void mymemcpy(void *dest, void *src, int n){
  char *csrc = (char*)src;
  char *cdest = (char*)dest;

  for (int i = 0;i<n;i++){
    cdest[i] = csrc[i];
  }
}

/* critical_wrapper tracks kernel/user time in a tidy way.
*  inputs:
*   _ call: a pointer to the requested system call (null if updating from interrupt);
*   _ callerState: state vector of the caller process;
*   _ start_time: TOD saved at the beginning of the critical section;
*   _ currentProcess: process control block of the calling process.
*  outputs:
*   _ call_sched: forwards the bool returned by the calls.
*/

int critical_wrapper(int (*call)(), state_t* callerState, cpu_time start_time, pcb_t* currentProcess) {
  int call_sched = 0;
  if (call) {
    //call the requested syscall function
    call_sched = call(callerState);
  }
  cpu_time end_time = *((unsigned int *)BUS_REG_TOD_LO);
  //save the time of the last context switch to user mode
  currentProcess->last_restart = end_time;
  //update the currentProcess kernel time
  currentProcess->kernel_timer = currentProcess->kernel_timer + end_time - start_time;
  return call_sched;
}

cpu_time update_user_time(pcb_t* currentProcess) {
  cpu_time start_time = *((unsigned int *)BUS_REG_TOD_LO);;
  currentProcess->user_timer = currentProcess->user_timer + (start_time - currentProcess->last_restart);
  currentProcess->last_stop = start_time;
  return(start_time);
}

void set_return(state_t* caller, int status) {
  #if TARGET_UMPS
  caller->reg_v0 = status;
  #elif TARGET_UARM
  caller->a1 = status;
  #endif
}

unsigned int get_status(termreg_t* reg, int subdevice){
  if (subdevice)
    return reg->recv_status;
  else
    return reg->transm_status;
}

void debug(int id, int val){
  return ;
}
