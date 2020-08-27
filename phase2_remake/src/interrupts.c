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
  //order of ifs is important to preserve interrupt priority
  if(cause & PROC_INT) {
    //not handled!
    PANIC();
  } else if (cause & LOCALTIME_INT) {
    //unused
    //do nothing and pass
  } else if (cause & INTERVALTIME_INT) {
    requested_handler = &timeHandler;
  } else if(cause & DISK_INT) {
    requested_handler = &diskHandler;
  } else if (cause & TAPE_INT) {
    requested_handler = &tapeHandler;
  } else if (cause & NETWORK_INT) {
    requested_handler = &netHandler;
  } else if (cause & PRINTER_INT) {
    requested_handler = &printerHandler;
  } else if(cause & TERM_INT) {
    requested_handler = &termHandler;
  } else {
    //unknown cause
    PANIC();
  }
  critical_wrapper(requested_handler, oldstatus, start_time, currentProcess);
  scheduler();
}


/* specific device handlers */

void exitInterrupt(state_t* oldstatus) {
  mymemcpy(&currentProcess->p_s, oldstatus, sizeof(*oldstatus));
  schedInsertProc(currentProcess);
}

int diskHandler(state_t* oldstatus) {
  devHandler(DISK_LINE, oldstatus);
  return 0;
}

int tapeHandler(state_t* oldstatus) {
  devHandler(TAPE_LINE, oldstatus);
  return 0;
}

int netHandler(state_t* oldstatus) {
  devHandler(NETWORK_LINE, oldstatus);
  return 0;
}

int printerHandler(state_t* oldstatus) {
  devHandler(PRINTER_LINE, oldstatus);
  return 0;
}

int termHandler(state_t* oldstatus) {
  devHandler(TERMINAL_LINE, oldstatus);
  return 0;
}

/* helper methods */
void devHandler(int line, state_t* oldstatus) {
  int deviceNumber = getDeviceNumber(line);
  if(deviceNumber == -1)
    HALT();
  devreg_t* deviceReg = (devreg_t*) DEV_REG_ADDR(line, deviceNumber);
  unsigned int * comReg = NULL;
  unsigned int status = 0;
  int read = 0;
  if(line == TERMINAL_LINE) {
    //device is terminal
    unsigned int recv_status = rx_status(&(deviceReg->term));
    unsigned int trans_status = tx_status(&(deviceReg->term));
    if((trans_status & TERM_STATUS_MASK) == ST_TRANSMITTED){
      comReg = &(deviceReg->term).transm_command;
      status = trans_status;
      ACKDevice(comReg);
      verhogenDevice(line, status, deviceNumber, read);
    }
    if((recv_status & TERM_STATUS_MASK) == ST_RECEIVED){
      comReg = &(deviceReg->term).recv_command;
      read = 1;
      status = recv_status;
      ACKDevice(comReg);
      verhogenDevice(line, status, deviceNumber, read);
    }
  } else {
    //generic device
    status = (deviceReg->dtp).status;
    comReg = &(deviceReg->dtp).command;
    ACKDevice(comReg);
    verhogenDevice(line, status, deviceNumber, read);
  }
  exitInterrupt(oldstatus);
}

void ACKDevice(unsigned int* commandRegister) {
  if(commandRegister)
    *commandRegister = CMD_ACK;
}

void verhogenDevice(int line, unsigned int status, int deviceNumber, int read) {
  if((line != 0) && (status != 0)) {
    int *s_key = (getSemDev(line, deviceNumber, read))->s_key;
    ++(*s_key);
    debug(1, (int)s_key);
    if(*s_key <= 0){
      pcb_t* waiting_proc = removeBlockedonDevice(s_key);
      set_return(&waiting_proc->p_s, status);
      blockedCount--;
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

unsigned int tx_status(termreg_t *tp)
{
    return ((tp->transm_status));
}

unsigned int rx_status(termreg_t *tp){
  return ((tp->recv_status));
}
