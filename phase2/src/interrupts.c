#include "interrupts.h"
#include "scheduler.h"
#include "utils.h"
#include "asl.h"

extern int semDev[DEV_USED_INTS * DEV_PER_INT + 1];

void interruptHandler() {
  unsigned int start_time = update_user_time(currentProcess);

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
  /* the order of the cases is important, interrupts sorted by priority. */
  if (cause & INT_CPU_BIT) {
    PANIC();
  }
  else if (cause & INT_T_SLICE_BIT) {
    //TODO
  }
  else if (cause & INT_TIMER_BIT){
    mymemcpy(&currentProcess->p_s, old_status, sizeof(*old_status));
    schedInsertProc(currentProcess);
    scheduler();
  }
  else if (cause & INT_DISK_BIT) {
    deviceHandler(INT_DISK);
  }
  else if (cause & INT_TAPE_BIT) {
    deviceHandler(INT_TAPE);
  }
  else if (cause & INT_UNUSED_BIT) {
    deviceHandler(INT_UNUSED);
  }
  else if (cause & INT_PRINTER_BIT) {
    deviceHandler(INT_PRINTER);
  }
  else if (cause & INT_TERMINAL_BIT) {
    deviceHandler(INT_TERMINAL);
  }
  else {
    //unknown interrupt
    PANIC();
  }

  critical_wrapper(NULL, NULL, start_time, NULL);
  scheduler();
}

void deviceHandler(int line) {
  int deviceNumber = getDeviceNumber(line);
  if(deviceNumber == -1)
    //something went wrong somewhere
    PANIC();

  devreg_t* deviceRegister = (devreg_t*) GET_DEV_REG(line, deviceNumber);
  unsigned int stat;
  //mantaining the interrupt priority order, terminal is last
  if (line != INT_TERMINAL) {
    //generic device
    stat = (deviceRegister -> dtp).status;
    //send ack
    (deviceRegister -> dtp).command = CMD_ACK;
  } else {
    //terminal is different
    stat = tx_status(&(deviceRegister -> term));
  }
  /* index of the relative semaphore calculated by finding the current interrupt line + the device number*/
  int semIndex = (line - INTERNAL_INTS) * DEV_PER_INT + deviceNumber;
  //verhogen the relative semaphore
  semDev[semIndex]++;
  if(semDev[semIndex] <= 0) {
    pcb_t* unblocked = removeBlocked(&(semDev[semIndex]));
    if(unblocked) {
      set_return(&(unblocked -> p_s), stat);
      schedInsertProc(unblocked);
    }
  }
}

int getDeviceNumber(int line) {
  unsigned int* bitmap = (unsigned int *) GET_BITMAP(line);
  unsigned int cause = *bitmap;
  //find interrupt line (exponent) of the lowest set bit. preserves priority order
  unsigned int devNum = log2(cause * -cause);

  if((devNum == 0) || (devNum > INT_TERMINAL)) {
    return -1;
  } else {
    return(devNum);
  }
}

unsigned int tx_status(termreg_t *tp)
{
    return ((tp -> transm_status) & TERM_STATUS_MASK);
}
