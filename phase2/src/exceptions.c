#include "exceptions.h"
#include "scheduler.h"
#include "pcb.h"
#include "listx.h"
#include "system.h"
#include "types_bikaya.h"

void trapHandler() {
  //to be implemented
}

// unsigned int start_time = (unsigned int) BUS_REG_TOD_LO;

void update_kernel_time(unsigned int start_time) {
  unsigned int end_time = (unsigned int) BUS_REG_TOD_LO;
  currentProcess->last_restart = end_time;
  currentProcess->kernel_timer += end_time - start_time;
}

void syscallHandler() {
  state_t* callerState = NULL;
  int syscallRequest = 0;

  unsigned int start_time = (unsigned int) BUS_REG_TOD_LO;
  currentProcess->user_timer += (start_time - currentProcess->last_restart);

  // unsigned int start_time = (unsigned int) BUS_REG_TOD_LO; //for sys1
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
  // callerState-> pc = callerState-> pc + WORD_SIZE; //THIS IS NOT REQUESTED IN UARM
  #endif
  switch(syscallRequest) {
    case GET_CPU_TIME:
      Get_Cpu_Time(callerState, start_time);
    break;
    case CREATE_PROCESS:
      create_process(callerState, start_time);
    break;
    case TERMINATE_PROCESS:
      kill(start_time);
    break;
    //syscall not recognized
    default:
      PANIC();
    break;
  }
}

void Get_Cpu_Time(state_t* callerState, unsigned int start_time) {
  //code for umps
  unsigned int *time = (unsigned int*) BUS_REG_TOD_LO;
  #if TARGET_UMPS
  callerState->reg_a3 = *time - currentProcess->first_activation;
  callerState->reg_a2 = currentProcess->kernel_timer;
  callerState->reg_a1 = currentProcess->user_timer;
  #elif TARGET_UARM
  callerState->a4 = *time - currentProcess->first_activation;
  callerState->a3 = currentProcess->kernel_timer;
  callerState->a2 = currentProcess->user_timer;
  #endif

  update_kernel_time(start_time);
}


void create_process(state_t* callerState, unsigned int start_time) {
  pcb_t* newProc = allocPcb();
  if(newProc == NULL){
    #if TARGET_UMPS
    callerState->reg_v0 = -1;
    #elif TARGET_UARM
    callerState->a1 = -1;
    #endif
    //return to caller
    LDST(callerState);
  }
  int priority;
  state_t* new_state;
  #if TARGET_UMPS
  new_state = (state_t*) callerState->reg_a1;
  priority = callerState->reg_a2;
  //set the cpid to the pcb address
  callerState->reg_a3 = &newProc;
  #elif TARGET_UARM
  new_state = (state_t*) callerState->a2;
  priority = callerState->a3;
  //set the cpid to the pcb address
  callerState->a4 = &newProc;
  #endif
  newProc->priority = priority;
  //insert the new process as a child
  insertChild(currentProcess, newProc);
  //shedule the new process
  schedInsertProc(newProc);
  //load the passed state into the new process
  mymemcpy(new_state, &(newProc->p_s), sizeof(*newProc));
  processCount = processCount + 1;
  #if TARGET_UMPS
  callerState->reg_v0 = 0;
  #elif TARGET_UARM
  callerState->a1 = 0;
  #endif
  //return to caller
  LDST(callerState);

  update_kernel_time(start_time);
}

void kill(unsigned int start_time) {
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
  update_kernel_time(start_time);
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
