#include "exceptions.h"
#include "scheduler.h"
#include "pcb.h"
#include "asl.h"
#include "listx.h"
#include "system.h"
#include "types_bikaya.h"
#include "utils.h"

void trapHandler() {
  state_t* callerState = NULL;
  #if TARGET_UMPS
  callerState = (state_t*) TRAP_OLDAREA;
  #elif TARGET_UARM
  callerState = (state_t*) PGMTRAP_OLDAREA;
  #endif

  if(currentProcess->pgtNew != NULL){
    mymemcpy(currentProcess->pgtOld, callerState, sizeof(state_t));
    LDST(currentProcess->pgtNew);
  }
  //TODO: time handling
  kill(NULL);
}

void syscallHandler() {
  //save time at the beginning of the kernel code
  unsigned int start_time = update_user_time(currentProcess);

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
  // callerState-> pc = callerState-> pc + WORD_SIZE; //THIS IS NOT REQUESTED IN UARM
  #endif
  void * requested_call = NULL;
  switch(syscallRequest) {
    case GETCPUTIME:
      requested_call = &get_cpu_time;
    break;
    case CREATEPROCESS:
      requested_call = &create_process;
    break;
    case TERMINATEPROCESS:
      requested_call = &kill;
    break;
    case VERHOGEN:
      requested_call = &verhogen;
    break;
    case PASSEREN:
      requested_call = &passeren;
    break;
    case WAITIO:
      requested_call = &do_IO;
    break;
    case SPECPASSUP:
      requested_call = &spec_passup;
    break;
    case GETPID:
      requested_call = &get_pid_ppid;
    break;
    //syscall not recognized
    default:
      PANIC();
    break;
  }
  int call_sched = critical_wrapper(requested_call, callerState, start_time, currentProcess);
  if(call_sched)
    //nothing to do
    scheduler();
  else
    LDST(callerState);
}

/*
SYSTEM CALL STRUCTURE:
  input:
   _ caller state vector
  output:
   _ int as bool representing the need to call the scueduler (current process
     is kill, or blocked by semaphore).
*/

 // bidogna aggiungere il tempo passato a gestire gli interrupt?
int get_cpu_time(state_t* callerState) {
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
  return FALSE;
}


int create_process(state_t* callerState) {
  pcb_t* newProc = allocPcb();
  if(newProc == NULL){
    #if TARGET_UMPS
    callerState->reg_v0 = -1;
    #elif TARGET_UARM
    callerState->a1 = -1;
    #endif
    //return to caller
    return FALSE;
  }
  int priority;
  state_t* new_state;
  #if TARGET_UMPS
  new_state = (state_t*) callerState->reg_a1;
  priority = callerState->reg_a2;
  //set the cpid to the pcb address
  if(!callerState->reg_a3)
    callerState->reg_a3 = (unsigned int) newProc; //non sono sicuro del cast
  #elif TARGET_UARM
  new_state = (state_t*) callerState->a2;
  priority = callerState->a3;
  //set the cpid to the pcb address
  if(!callerState->a4)
    callerState->a4 = (unsigned int) newProc;
  #endif
  newProc->priority = priority;
  newProc->original_priority = priority;
  //insert the new process as a child
  insertChild(currentProcess, newProc);
  //shedule the new process
  schedInsertProc(newProc);
  //load the passed state into the new process
  mymemcpy(&(newProc->p_s), new_state, sizeof(*new_state));
  processCount = processCount + 1;
  #if TARGET_UMPS
  callerState->reg_v0 = 0;
  #elif TARGET_UARM
  callerState->a1 = 0;
  #endif
  //return to caller
  return FALSE;

}
int pid_in_readyQ(pcb_t* pid){
  pcb_t* ptr;
  list_for_each_entry(ptr, &readyQueue, p_next){
    if (ptr == pid)
      return 1;
  }
  return 0;
}

int kill(state_t* callerState) {
  pcb_t* kpid = NULL;
  if(callerState == NULL){ //used in specpassup kill
    kpid = currentProcess;
    callerState = &currentProcess->p_s;
  }
  else{
    #if TARGET_UMPS
    kpid = (pcb_t*) callerState->reg_a1;
    #elif TARGET_UARM
    kpid = (pcb_t*)  callerState->a2;
    #endif
  //TODO: gestire successo o errore
  }
  if (kpid == NULL)
    kpid = currentProcess;
  if(!pid_in_readyQ(kpid)){
    #if TARGET_UMPS
    callerState->reg_v0 = -1;
    #elif TARGET_UARM
    callerState->a1 = -1;
    #endif
    return FALSE;
  }

  if(!emptyChild(kpid)){
    //process has child processes, killing the whole tree
    recursive_kill(kpid);
  }
  else{
    //process is single
    outChild(kpid);
    freePcb(kpid);
    processCount = processCount - 1;
  }

  #if TARGET_UMPS
  callerState->reg_v0 = 0;
  #elif TARGET_UARM
  callerState->a1 = 0;
  #endif
  //currentProcess is dead, and we killed it. scheduler gains control to advance execution
  return (kpid == currentProcess);
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
    processCount = processCount - 1;
  }
}

int verhogen(state_t* callerState) {
  int* sem = NULL;
  #if TARGET_UMPS
  sem = (int*) callerState->reg_a1;
  #elif TARGET_UARM
  sem = (int*)  callerState->a2;
  #endif
  (*sem)++;
  if((*sem) <= 0) {
    pcb_t* unblocked = removeBlocked(sem);
    if(unblocked) {
      schedInsertProc(unblocked);
    }
  }
  //return to caller
  return FALSE;
}

int passeren(state_t* callerState) {
  int* sem = NULL;
  #if TARGET_UMPS
  sem = (int*) callerState->reg_a1;
  #elif TARGET_UARM
  sem = (int*)  callerState->a2;
  #endif
  (*sem)--;
  if((*sem) < 0) {
    //capire cosa fare con mymemcpoy
    insertBlocked(sem, currentProcess);
    //need to call the scheduler
    return TRUE;
  }
  //return to caller
  return FALSE;
}

int get_pid_ppid(state_t* callerState) {
  pcb_t* pid = NULL;
  #if TARGET_UMPS
  pid = (pcb_t*) callerState->reg_a1;
  #elif TARGET_UARM
  pid = (pcb_t*)  callerState->a2;
  #endif
  if(pid != NULL) {
    pcb_t* ppid = pid->p_parent;
    #if TARGET_UMPS
    if(!callerState->reg_a2)
      callerState->reg_a2 = (unsigned int) ppid;
    #elif TARGET_UARM
    if(!callerState->a3)
      callerState->a3 = (unsigned int) ppid;
    #endif
  }
  return FALSE;
}

int do_IO(state_t* callerState){
  unsigned int command;
  unsigned int *reg;
  int subdevice;
  unsigned int status;
  #if TARGET_UMPS
  command = (unsigned int)callerState->reg_a1;
  reg = (unsigned int*)callerState->reg_a2;
  subdevice = (int)callerState->reg_a3;
  #elif TARGET_UARM
  command = (unsigned int)callerState->a2;
  reg = (unsigned int*)callerState->a3;
  subdevice = (int)callerState->a4;
  #endif
  if(subdevice == 1) //for terminal
    reg++;

  *(reg + 0x4) = command;
  while((status = (*reg & 0xFF)) == 3)
    ;
  *(reg + 0x4) = 1; // ack

  if(status != 1){
  #if TARGET_UMPS
     callerState->reg_v0 = -1;
  #elif TARGET_UARM
    callerState->a1 = -1;
  #endif
  }
  else{
    #if TARGET_UMPS
     callerState->reg_v0 = *reg;
  #elif TARGET_UARM
    callerState->a1 = *reg;
  #endif
  }
  return FALSE;
}

void passup_kill(state_t* callerState){
  #if TARGET_UMPS
  callerState->reg_v0 = -1;
  #elif TARGET_UARM
  callerState->a1 = -1;
  #endif
  //todo update kernel time
  kill(NULL);
}

int spec_passup(state_t* callerState){
  int type;
  #if TARGET_UMPS
  type = callerState->reg_a1;
  #elif TARGET_UARM
  type = callerState->a2;
  #endif
  switch (type)
  {
    case 0: //sys/break
      if(currentProcess->sysNew != NULL){
        //todo time
        passup_kill(callerState);
      }
      #if TARGET_UMPS
      currentProcess->sysNew = (state_t*)callerState->reg_a3;
      currentProcess->sysOld = (state_t*)callerState->reg_a2;
      #elif TARGET_UARM
      currentProcess->sysNew = (state_t*)callerState->a4;
      currentProcess->sysOld = (state_t*)callerState->a3;
      #endif

      break;
    case 1:
      if(currentProcess->TlbNew != NULL){
        passup_kill(callerState);
      }
      #if TARGET_UMPS
      currentProcess->TlbNew =(state_t*)callerState->reg_a3;
      currentProcess->TlbOld = (state_t*)callerState->reg_a2;
      #elif TARGET_UARM
      currentProcess->TlbNew = (state_t*)callerState->a4;
      currentProcess->TlbOld = (state_t*)callerState->a3;
      #endif
      break;
    case 2:
      if(currentProcess->pgtNew != NULL){
        passup_kill(callerState);
      }
      #if TARGET_UMPS
      currentProcess->pgtNew = (state_t*)callerState->reg_a3;
      currentProcess->pgtOld = (state_t*)callerState->reg_a2;
      #elif TARGET_UARM
      currentProcess->pgtNew = (state_t*)callerState->a4;
      currentProcess->pgtOld = (state_t*)callerState->a3;
      #endif
    break;
  }
  #if TARGET_UMPS
  callerState->reg_v0 = 0;
  #elif TARGET_UARM
  callerState->a1 = 0;
  #endif

  return FALSE;
}

void TLBManager() {
  state_t* callerState = NULL;
  callerState = (state_t*) TLB_OLDAREA;
  if(currentProcess->TlbNew != NULL){
    mymemcpy(currentProcess->TlbOld, callerState, sizeof(state_t));
    LDST(currentProcess->TlbNew);
  }
  //no hanlder defined
  kill(NULL); //todo time
}
