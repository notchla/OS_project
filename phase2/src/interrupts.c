#include "interrupts.h"
#include "scheduler.h"

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
  } else if (cause & INT_DISK_BIT) {
    deviceHandler(INT_DISK);
  } else if (cause & INT_TAPE_BIT) {
    deviceHandler(INT_TAPE);
  } else if (cause & INT_UNUSED_BIT) {
    deviceHandler(INT_UNUSED);
  } else if (cause & INT_PRINTER_BIT) {
    deviceHandler(INT_PRINTER);
  } else if (cause & INT_TERMINAL_BIT) {
    deviceHandler(INT_TERMINAL);
  }
  else {
    //unknown interrupt
    PANIC();
  }
}

void deviceHandler(int line) {
  int deviceNumber = getDeviceNumber(line);
  if(deviceNumber == -1)
    //something went wrong somewhere
    PANIC();
  devreg_t* deviceRegister = getDeviceRegister(line, deviceNumber);

  unsigned int stat;
  if (line == INT_TERMINAL) {
    //terminal is different
  } else {
    //generic device
  }
}


int getDeviceNumber(int line) {
  /* because there are 5 bitmaps (from interrupt 3 to 7), each 1 word wide, the wanted bitmap
  line is located at (line - INTERNAL_INTS) words after the start address.*/
  unsigned int* bitmap = (unsigned int *) (BITMAPSTART + (line - INTERNAL_INTS) * WORDLEN);
  unsigned int cause = *bitmap;
  if (cause & INT_CPU_BIT) {
    return 0;
  }
  else if (cause & INT_T_SLICE_BIT) {
    return 1;
  }
  else if (cause & INT_TIMER_BIT){
    return 2;
  }
  else if (cause & INT_DISK_BIT) {
    return 3;
  }
  else if (cause & INT_TAPE_BIT) {
    return 4;
  }
  else if (cause & INT_UNUSED_BIT) {
    return 5;
  }
  else if (cause & INT_PRINTER_BIT) {
    return 6;
  }
  else if (cause & INT_TERMINAL_BIT) {
    return 7;
  }
  else {
    return -1;
  }
}


devreg_t* getDeviceRegister(int line, int deviceNumber) {
  /* starting from the device register beginning address, first reach the wanted line by multiplying
     it by the size of the total size of a device register line (DEVREGSIZE * DEVININT);
     then get to the wanted device by summing (deviceNumber * DEVREGSIZE) */
  return ((devreg_t*) (DEVREGSTART + (line - INTERNAL_INTS) * (DEVREGSIZE * DEV_PER_INT) + (deviceNumber * DEVREGSIZE)));
}
