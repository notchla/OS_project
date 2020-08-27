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

void set_command(devreg_t* reg, unsigned int command, int subdevice){
  if(subdevice == -1)
    reg->dtp.command = command;
  else if(subdevice == 1)
    reg->term.recv_command = command;
  else
    reg->term.transm_command = command;
}

void get_line_dev(devreg_t* reg, int* line, int* dev) {
  unsigned int base_dev = DEV_REG_ADDR(LOWEST_LINE, 0);
  unsigned int offset = ((unsigned int) reg) - base_dev;
  unsigned int line_offset = (unsigned int) offset / DEV_REG_SIZE;
  *line = (int) line_offset / DEV_PER_INT + LOWEST_LINE;
  *dev = line_offset % DEV_PER_INT;
}


unsigned int get_status(devreg_t* reg, int subdevice){
  if (subdevice == -1)
    //-1 for generic device
    return reg->dtp.status;
  else if(subdevice == 1)
    return reg->term.recv_status;
  else
    return reg->term.transm_status;
}


semd_t *getSemDev(int line, int devnum, int read) {
  debug(3,line);
  debug(4,read);
  debug(5,devnum);
  switch (line) {
    case DISK_LINE:
      return &semDev.disk[devnum];
    break;
    case TAPE_LINE:
      return &semDev.tape[devnum];
    break;
    case NETWORK_LINE:
      return &semDev.network[devnum];
    break;
    case PRINTER_LINE:
      return &semDev.printer[devnum];
    break;
    case TERMINAL_LINE:
      if(read == 1)
        return &semDev.terminalR[devnum];
      else
        return &semDev.terminalT[devnum];
    break;
    default:
      return NULL;
    break;
  }
}

void debug(int id, int val){
  return ;
}
