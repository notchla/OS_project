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
*   _ call: a pointer to the requested system call;
*   _ callerState: state vector of the caller process;
*   _ start_time: TOD saved at the beginning of the critical section;
*   _ currentProcess: process control block of the calling process.
*  outputs:
*   _ call_sched: forwards the bool returned by the calls.
*/

int critical_wrapper(int (*call)(), state_t* callerState, unsigned int start_time, pcb_t* currentProcess) {
  //call the requested syscall function
  int call_sched = call(callerState);
  unsigned int end_time = (unsigned int) BUS_REG_TOD_LO;
  //save the time of the last context switch to user mode
  currentProcess->last_restart = end_time;
  //update the currentProcess kernel time
  currentProcess->kernel_timer += end_time - start_time;

  return call_sched;
}

unsigned int update_user_time(pcb_t* currentProcess) {
  unsigned int start_time = (unsigned int) BUS_REG_TOD_LO;
  currentProcess->user_timer += (start_time - currentProcess->last_restart);
  return(start_time);
}
