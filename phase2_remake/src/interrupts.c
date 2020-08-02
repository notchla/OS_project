#include "interrupts.h"
#include "system.h"
#include "scheduler.h"
#include "utils.h"
#include "asl.h"
#include "pcb.h"

// #include "termprint.h"

void interruptHandler() {
  state_t* old_status;
  unsigned int cause;
  #if TARGET_UMPS
  old_status = (state_t*) IE_OLDAREA;
  cause = old_status-> cause;
  cause = (cause >> 8) & CAUSE_MASK;
  #elif TARGET_UARM
  old_status = (state_t*) INT_OLDAREA;
  cause = old_status-> CP15_Cause;
  cause = CAUSE_ALL_GET(cause);
  old_status->pc -= WORD_SIZE;
  #endif
  if (cause & INTERVALTIME_INT){
    mymemcpy(&currentProcess->p_s, old_status, sizeof(*old_status));
    schedInsertProc(currentProcess);
    scheduler();
  }
  else if(cause & TERM_INT){
    int deviceNumber = getDeviceNumber(INT_TERMINAL);
    if(deviceNumber == -1)
      HALT();
    devreg_t* deviceReg = (devreg_t*) DEV_REG_ADDR(INT_TERMINAL, deviceNumber);
    unsigned int trans_status = tx_status(&(deviceReg->term));
    unsigned int recv_status = rx_status(&(deviceReg->term));
    //TODO HANDLE TRASM AND RECEIVE CASE AND CHECK IF A PROCESS NEEDS TO BE WAKED IN THE SEM DEVICE QUEUE
    if((trans_status & TERM_STATUS_MASK) == ST_TRANSMITTED){
      int index = INT_TERMINAL + 1;
      (deviceReg->term).transm_command = CMD_ACK;
      int* s_key = &semdevices[(index - 3)*DEV_PER_INT + deviceNumber];
      ++(*s_key);
      if(*s_key <= 0){
        pcb_t* waiting_proc = removeBlocked(s_key);
        set_return(&waiting_proc->p_s, trans_status);
        schedInsertProc(waiting_proc);
      }
    }
    if((recv_status & TERM_STATUS_MASK) == ST_RECEIVED){
      int index = INT_TERMINAL;
      (deviceReg->term).recv_command = CMD_ACK;
      int *s_key = &semdevices[(index - 3)*DEV_PER_INT + deviceNumber];
      ++(*s_key);
      if(*s_key <= 0){
        pcb_t* waiting_proc = removeBlocked(s_key);
        set_return(&waiting_proc->p_s, trans_status);
        schedInsertProc(waiting_proc);
      }
    }
    mymemcpy(&currentProcess->p_s, old_status, sizeof(*old_status));
    schedInsertProc(currentProcess);
    scheduler();
  }
  else if(cause == 0){
    // term_puts("0");
    // PANIC();
  }
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