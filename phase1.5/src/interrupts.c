#include "interrupts.h"
#include "system.h"
#include "scheduler.h"
#include "p1.5test_bikaya_v0.c"

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
  currentProcess-> p_s.pc = currentProcess-> p_s.pc - WORD_SIZE;
  cause = (cause >> 8) & CAUSE_MASK;
  #endif
  termprint("\n -- test interrupts out --\n");
  if (cause == INTERVALTIME_INT) {
    termprint("test interrupts");
    scheduler();
  }
}
