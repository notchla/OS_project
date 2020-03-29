#include "interrupts.h"
#include "system.h"
#include "scheduler.h"
#include "p1.5test_bikaya_v0.c"

void mymemcpy(void *dest, void *src, int n){
  char *csrc = (char*)src;
  char *cdest = (char*)dest;

  for (int i = 0;i<n;i++){
    cdest[i] = csrc[i];
  }
}

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
  // currentProcess-> p_s.pc = currentProcess-> p_s.pc - WORD_SIZE;
  old_status->pc -= WORD_SIZE;
  // cause = (cause >> 8) & CAUSE_MASK;
  #endif
  // if (cause == LOCALTIME_INT) {
  //   mymemcpy(&currentProcess->p_s, old_status, sizeof(*old_status));
  //   schedInsertProc(currentProcess);
  //   scheduler();
  // }
  // else if (cause == TERM_INT){
  //   unsigned int stat = tx_status(term0_reg);
  //   while ((stat = tx_status(term0_reg)) == ST_BUSY)
  //     ;
  //   term0_reg->transm_command = CMD_ACK;
  //   LDST(old_status);
  // }
  mymemcpy(&currentProcess->p_s, old_status, sizeof(*old_status));
  schedInsertProc(currentProcess);
  scheduler();
}
