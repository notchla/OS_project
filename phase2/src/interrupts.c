#include "interrupts.h"
#include "system.h"
#include "scheduler.h"
#include "utils.h"
#include "asl.h"
#include "pcb.h"

void interruptHandler() {
  cpu_time start_time = update_user_time(currentProcess);
  void* requested_handler = NULL;
  state_t* oldstatus = NULL;
  unsigned int cause;
  #if TARGET_UMPS
  oldstatus = (state_t*) IE_OLDAREA;
  cause = oldstatus-> cause;
  #elif TARGET_UARM
  oldstatus = (state_t*) INT_OLDAREA;
  cause = oldstatus-> CP15_Cause;
  oldstatus->pc -= WORD_SIZE;
  #endif
  //mask and shift unwanted bits
  cause = CAUSE_ALL_GET(cause);
  //order of ifs is important to preserve interrupt priority
  if(cause & PROC_INT) {
    //not handled!
    PANIC();
  } else if (cause & LOCALTIME_INT) {
    //unused
    //do nothing and pass
  } else if (cause & INTERVALTIME_INT) {
    requested_handler = &timerHandler;
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
  //return value is unimportant, always return to caller
  scheduler();
}


/* specific device handlers */

int timerHandler(state_t* oldstatus) {
  exitInterrupt(oldstatus);
  return 0;
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

void exitInterrupt(state_t* oldstatus) {
  mymemcpy(&currentProcess->p_s, oldstatus, sizeof(*oldstatus));
  schedInsertProc(currentProcess);
}
