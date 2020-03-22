#include "exceptions.h"
#include "consts_bikaya.h"
#include "scheduler.h"
#include "pcb.h"
#include "listx.h"
#include "system.h"

void trapHandler() {
  //stub function, to be implemented
}

void syscallHandler() {
  state_t* callerState = (state_t*) getCAUSE(); // = (state_t) SYS_NEWAREA (or SYSBK_NEWAREA)
  int syscallRequest = 0;
  /*need to increment program counter to next instruction, otherwise
    syscall loop (4 is word size) */
  #if TARGET_UMPS
  //not in kernel mode
  if(callerState-> status & UMODE != ALLOFF) {
    PANIC();
  }
  syscallRequest = callerState-> reg_a0;
  callerState-> pc_epc = callerState-> pc_epc + 4;
  #elif TARGET_UARM
  if(callerState-> cpsr & STATUS_SYS_MODE == STATUS_NULL) {
    PANIC();
  }
  syscallRequest = callerState-> CP15_Cause;
  callerState-> pc = callerState-> pc + 4;
  #endif
  switch(syscallRequest) {
    case CREATE_PROCESS:  //to be implemented
    break;
    case TERMINATE_PROCESS:
      kill();
    break;
    case VERHOGEN:  //to be implemented
    break;
    case PASSEREN:  //to be implemented
    break;
    case EXCEPTION_STATE_VECTOR:  //to be implemented
    break;
    case GET_CPU_TIME:  //to be implemented
    break;
    case WAIT_CLOCK:  //to be implemented
    break;
    case WAIT_IO_DEVICE:  //to be implemented
    break;
    //syscall not recognized
    default:
      PANIC();
    break;
  }
}

void kill() {
  if(!emptyChild(currentProcess)) {
    pcb_t* child;
    list_for_each_entry(child, &currentProcess->p_child, p_sib){
      if(outProcQ(&readyQueue, child)) { //if actually deleting something
        processCount = processCount - 1;
        freePcb(child);
      }
    }
    outChild(currentProcess);
  } else {
    outChild(currentProcess);
    freePcb(currentProcess);
    processCount = processCount - 1;
  }
  currentProcess = NULL;
  //return control to scheduler
  scheduler();
}

void TLBManager() {
  //stub function, to be implemented
}
