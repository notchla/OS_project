#include "exceptions.h"
#include "scheduler.h"
#include "pcb.h"
#include "listx.h"
#include "system.h"
#include "types_bikaya.h"

void trapHandler() {
  //to be implemented
}

void syscallHandler() {
  state_t* callerState = NULL;
  int syscallRequest = 0;
  #if TARGET_UMPS
  callerState = (state_t*) SYS_OLDAREA;
  //not in kernel mode
  if(callerState-> status & UMODE != ALLOFF) {
    PANIC();
  }
  syscallRequest = callerState-> reg_a0;
  /*need to increment program counter to next instruction, otherwise
  syscall loop (WORD_SIZE = 4) */
  callerState-> pc_epc = callerState-> pc_epc + WORD_SIZE;
  #elif TARGET_UARM
  callerState = (state_t*) SYSBK_OLDAREA;
  //not in kernel mode
  if((callerState-> cpsr & STATUS_SYS_MODE) == STATUS_NULL) {
    PANIC();
  }
  syscallRequest = callerState-> a1;
  callerState-> pc = callerState-> pc + WORD_SIZE;
  #endif
  switch(syscallRequest) {
    case TERMINATE_PROCESS:
      kill();
    break;
    //syscall not recognized
    default:
      PANIC();
    break;
  }
}

void kill() {
  if(!emptyChild(currentProcess)){
    //process has child processes, killing the whole tree
    recursive_kill(currentProcess);
  }
  else{
    //process is single
    outChild(currentProcess);
    freePcb(currentProcess);
    processCount = processCount - 1;
  }
  currentProcess = NULL;
  //call to scheduler to advance to the next process waiting
  scheduler();
}

void recursive_kill(pcb_t* process){
  if(process == NULL)
    return;
  pcb_t* child;
  list_for_each_entry(child, &process->p_child, p_sib){
    recursive_kill(child);
  }
  if(outProcQ(&readyQueue, process)){
    processCount = processCount - 1;
    freePcb(process);
  }
  //current process case
  else {
    outChild(process);
    freePcb(process);
    currentProcess = currentProcess - 1;
  }
}

void TLBManager() {
  //to be implemented
}
