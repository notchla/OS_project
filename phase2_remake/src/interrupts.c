#include "interrupts.h"
#include "system.h"
#include "scheduler.h"
#include "utils.h"
#include "asl.h"
#include "pcb.h"

// #include "termprint.h"

void interruptHandler() {
  cpu_time start_time = update_user_time(currentProcess);
  void* requested_handler = NULL;
  state_t* oldstatus = NULL;
  unsigned int cause;
  #if TARGET_UMPS
  oldstatus = (state_t*) IE_OLDAREA;
  cause = oldstatus-> cause;
  cause = (cause >> 8) & CAUSE_MASK;
  #elif TARGET_UARM
  oldstatus = (state_t*) INT_OLDAREA;
  cause = oldstatus-> CP15_Cause;
  cause = CAUSE_ALL_GET(cause);
  oldstatus->pc -= WORD_SIZE;
  #endif
  if (cause & INTERVALTIME_INT){
    requested_handler = &timeHandler;
  }
  else if(cause & TERM_INT){
    requested_handler = &termHandler;
  }
  else if(cause == 0){
    // term_puts("0");
    PANIC();
  }
  critical_wrapper(requested_handler, oldstatus, start_time, currentProcess);
  scheduler();
}

void exitInterrupt(state_t* oldstatus) {
  mymemcpy(&currentProcess->p_s, oldstatus, sizeof(*oldstatus));
  schedInsertProc(currentProcess);
}

int termHandler(state_t* oldstatus) {
  int deviceNumber = getDeviceNumber(INT_TERMINAL);
  if(deviceNumber == -1)
    HALT();
  devreg_t* deviceReg = (devreg_t*) DEV_REG_ADDR(INT_TERMINAL, deviceNumber);
  unsigned int line = 0;
  unsigned int * comReg = NULL;
  unsigned int status = 0;
  unsigned int recv_status = rx_status(&(deviceReg->term));
  unsigned int trans_status = tx_status(&(deviceReg->term));
  //TODO HANDLE TRASM AND RECEIVE CASE AND CHECK IF A PROCESS NEEDS TO BE WAKED IN THE SEM DEVICE QUEUE
  if((trans_status & TERM_STATUS_MASK) == ST_TRANSMITTED){
    comReg = &(deviceReg->term).transm_command;
    line = INT_TERMINAL + 1;
    status = trans_status;
    ACKDevice(comReg);
    verhogenDevice(line, status, deviceNumber);
  }
  if((recv_status & TERM_STATUS_MASK) == ST_RECEIVED){
    comReg = &(deviceReg->term).recv_command;
    line = INT_TERMINAL;
    status = recv_status;
    ACKDevice(comReg);
    verhogenDevice(line, status, deviceNumber);
  }
  exitInterrupt(oldstatus);
  return 0;
}

void ACKDevice(unsigned int* commandRegister) {
  if(commandRegister)
    *commandRegister = CMD_ACK;
}

void verhogenDevice(int line, unsigned int status, int deviceNumber) {
  if((line != 0) && (status != 0)) {
    int *s_key = &semdevices[(line - 3)*DEV_PER_INT + deviceNumber];
    ++(*s_key);
    if(*s_key <= 0){
      pcb_t* waiting_proc = removeBlocked(s_key);
      set_return(&waiting_proc->p_s, status);
      schedInsertProc(waiting_proc);
    }
  }
}

int timeHandler(state_t* oldstatus) {
  exitInterrupt(oldstatus);
  return 0;
}

int getDeviceNumber(int line){
  unsigned int* bitmap = (unsigned int *) CDEV_BITMAP_ADDR(line);
  unsigned int cause = *bitmap;
  //find interrupt line (exponent) of the lowest set bit. preserves priority order
  unsigned int devNum = log2(lowest_set(cause));

  if((devNum < 0) || (devNum > INT_TERMINAL)) {
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

unsigned int tx_status(termreg_t *tp)
{
    return ((tp -> transm_status));
}

unsigned int rx_status(termreg_t *tp){
  return ((tp->recv_status));
}
