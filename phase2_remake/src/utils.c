#include "utils.h"

void mymemcpy(void *dest, void *src, int n){
  char *csrc = (char*)src;
  char *cdest = (char*)dest;

  for (int i = 0;i<n;i++){
    cdest[i] = csrc[i];
  }
}

int critical_wrapper(int (*call)(), state_t* callerState, cpu_time start_time, pcb_t* currentProcess) {
  int call_sched = 0;
  if (call) {
    //call the requested syscall function
    call_sched = call(callerState);
  }
  cpu_time end_time = *((unsigned int *)BUS_REG_TOD_LO);
  if(currentProcess != NULL) {
    //save the time of the last context switch to user mode
    currentProcess->last_restart = end_time;
    //update the currentProcess kernel time
    currentProcess->kernel_timer = currentProcess->kernel_timer + end_time - start_time;
  }
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

void verhogenKill(pcb_t* process) {
  int * sem = process->p_semkey;
  if (sem) {
    //process was blocked on a semaphore
    //the s_key are in progressive, sequential memory areas by declaration
    if(sem >= getSemDev(LOWEST_LINE, 0, 0)->s_key && sem <= getSemDev(TERMINAL_LINE, DEV_PER_INT, 0)->s_key) {
      outBlocked(process);
      blockedCount--;
    } else {
      outBlocked(process);
    }
  }
}

void ACKDevice(unsigned int* commandRegister) {
  if(commandRegister)
    *commandRegister = CMD_ACK;
}

void verhogenDevice(int line, unsigned int status, int deviceNumber, int read) {
  if((line != 0) && (status != 0)) {
    int *s_key = (getSemDev(line, deviceNumber, read))->s_key;
    ++(*s_key);
    if(*s_key <= 0){
      pcb_t* waiting_proc = removeBlockedonDevice(s_key);
      set_return(&waiting_proc->p_s, status);
      blockedCount--;
      schedInsertProc(waiting_proc);
    }
  }
}

int pid_in_readyQ(pcb_t* pid) {
  pcb_t* ptr;
  list_for_each_entry(ptr, &readyQueue, p_next){
    if (ptr == pid)
      return 1;
  }
  return 0;
}

int set_register(unsigned int reg, unsigned int val) {
  if(reg) {
    *(unsigned int*) reg = val;
    return FALSE;
  } else {
    return TRUE;
  }
}


int getDeviceNumber(int line){
  unsigned int* bitmap = (unsigned int *) CDEV_BITMAP_ADDR(line);
  unsigned int cause = *bitmap;
  //find interrupt line (exponent) of the lowest set bit. preserves priority order
  unsigned int devNum = log2(lowest_set(cause));

  if((devNum < 0) || (devNum > TERMINAL_LINE)) {
    return -1;
  } else {
    return(devNum);
  }
}

int lowest_set(int number){
  return (number & -number);
}

int log2(unsigned int n){
  // very inefficient
  if(n > 1) {
    return(1 + log2(n/2));
  } else {
    return 0;
  }
}

unsigned int tx_status(termreg_t *tp) {
    return ((tp->transm_status));
}

unsigned int rx_status(termreg_t *tp) {
  return ((tp->recv_status));
}

void debug(int id, int val){
  return ;
}
