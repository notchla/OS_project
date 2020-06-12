#include "interrupts.h"
#include "system.h"
#include "scheduler.h"
#include "utils.h"

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
  if (cause & INT_TIMER_BIT){
    mymemcpy(&currentProcess->p_s, old_status, sizeof(*old_status));
    schedInsertProc(currentProcess);
    scheduler();
  }
  else if(cause == 0){
    PANIC();
  }
}
