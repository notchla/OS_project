#include "exceptions.h"
#include "scheduler.h"
#include "pcb.h"
#include "listx.h"
#include "system.h"
#include "types_bikaya.h"
#include "utils.h"
#include "termprint.h"
#include "asl.h"

//trap exceptions handler
int trapHelper(state_t* callerState) {
  if(currentProcess->pgtNew != NULL){
    mymemcpy(currentProcess->pgtOld, callerState, sizeof(state_t));
    return FALSE;//hanlder is definced
  }
  return TRUE;//hanlder is not defined
}

void trapHandler() {
  cpu_time start_time = updateUserTime(currentProcess);
  state_t* callerState = NULL;
  #if TARGET_UMPS
  callerState = (state_t*) TRAP_OLDAREA;
  #elif TARGET_UARM
  callerState = (state_t*) PGMTRAP_OLDAREA;
  #endif

  int call_sched = criticalWrapper(&trapHelper, callerState, start_time, currentProcess);

  if(call_sched) {//handler is not defined, process is killed
    recursiveKill(currentProcess);
    scheduler();
  }
  else
    LDST(currentProcess->pgtNew);//load the hanlder
}

/*--------------------------------------------------------------------------------------------------*/
//syscall exceptions handler
void syscallHandler() {
  cpu_time start_time = updateUserTime(currentProcess);

  int syscallRequest = 0;
  //custom syscall flag
  int custom = FALSE;
  state_t* callerState = NULL;

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
  #endif
  //pointer to the requested syscall function, to be evoked in the criticalWrapper
  void * requested_call = NULL;
  switch(syscallRequest) {
    case GETCPUTIME:
      requested_call = &getCPUTime;
    break;
    case CREATEPROCESS:
      requested_call = &createProcess;
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
      requested_call = &doIO;
    break;
    case SPECPASSUP:
      requested_call = &specPassup;
    break;
    case GETPID:
      requested_call = &getPIDPPID;
    break;
    //syscall not recognized, checking form custom
    default: ;
      requested_call = &customSyscall;
      custom = TRUE;
    break;
  }
  int call_sched = criticalWrapper(requested_call, callerState, start_time, currentProcess);
  if(call_sched) {
    if(custom)
      //custom syscall not initialized. kill the currentProcess, then call sched
      recursiveKill(currentProcess);
    scheduler();
  } else {
    if(custom)
      //load custom syscall
      LDST(currentProcess->sysNew);
    else
      LDST(callerState);
  }
}

/*--------------------------------------------------------------------------------------------------*/
//syscall function code

//SYS1
int getCPUTime(state_t* callerState){
  cpu_time time = *(unsigned int*) BUS_REG_TOD_LO;
  //precise current kernel time, counting the time elapsed until the call
  cpu_time currentKernelTime = currentProcess->kernel_timer + time - currentProcess->last_stop;
  #if TARGET_UMPS
  setRegister(callerState->reg_a3, time - currentProcess->first_activation);
  setRegister(callerState->reg_a2, currentKernelTime);
  setRegister(callerState->reg_a1, currentProcess->user_timer);
  #elif TARGET_UARM
  setRegister(callerState->a4, time - currentProcess->first_activation);
  setRegister(callerState->a3, currentKernelTime);
  setRegister(callerState->a2, currentProcess->user_timer);
  #endif
  return FALSE;
};

//SYS2
int createProcess(state_t* callerState){
  pcb_t* newProc = allocPcb();
  if(newProc == NULL){
    setReturn(callerState, -1);
    //return to caller
    return FALSE;
  }
  int priority;
  state_t* new_state;
  #if TARGET_UMPS
  new_state = (state_t*) callerState->reg_a1;
  priority = callerState->reg_a2;
  //set the cpid to the pcb address
  setRegister(callerState->reg_a3, (unsigned int) newProc);
  #elif TARGET_UARM
  new_state = (state_t*) callerState->a2;
  priority = callerState->a3;
  //set the cpid to the pcb address
  setRegister(callerState->a4, (unsigned int) newProc);
  #endif
  //check if the passed address is not null
  if(new_state == NULL) {
    setReturn(callerState, -1);
    return FALSE;
  }

  newProc->priority = priority;
  newProc->original_priority = priority;
  //insert the new process as a child
  insertChild(currentProcess, newProc);
  //shedule the new process
  schedInsertProc(newProc);
  //load the passed state into the new process
  mymemcpy(&(newProc->p_s), new_state, sizeof(*new_state));
  processCount = processCount + 1;
  setReturn(callerState, 0);
  //return to caller
  return FALSE;
};

//SYS3
int kill(state_t* callerState) {
  pcb_t* kpid = NULL;
  unsigned int killed_current = FALSE;
  #if TARGET_UMPS
  kpid = (pcb_t*) callerState->reg_a1;
  #elif TARGET_UARM
  kpid = (pcb_t*) callerState->a2;
  #endif
  if (kpid == NULL || (unsigned int) kpid == (unsigned int) currentProcess) {
    // currentProcess is dead, and we killed it.
    kpid = currentProcess;
    killed_current = TRUE;
  }
  // kpid is neither in readyqeue nor blocked on a semaphore; kpid is not a valid pcb
  else if(!PIDinReadyQ(kpid) && !isBlocked(kpid)){
    setReturn(callerState, -1);
    return FALSE;
  }
  if(!emptyChild(kpid)){
    //process has child processes, killing the whole tree
    recursiveKill(kpid);
  } else {
    //single process
    verhogenKill(kpid);
    // remove killed process from parent
    outChild(kpid);
    if(!killed_current) {
      //remove terminated process from readyQueue
      outProcQ(&readyQueue, kpid);
    }
    // back to free process list
    freePcb(kpid);
    processCount = processCount - 1;
  }
  setReturn(callerState, 0);
  // if the current process was terminated the scheduler gains control
  // the previous state is loaded otherwise
  return (killed_current);
}

// helper for the SYS3
void recursiveKill(pcb_t* process){
  if(process == NULL)
    return;
  while(!emptyChild(process)){
    pcb_t* child = container_of(process->p_child.next, pcb_t, p_sib);
    recursiveKill(child);
  }
  verhogenKill(process);
  outChild(process);
  if(process != currentProcess){
    outProcQ(&readyQueue, process);
  }
  freePcb(process);
  processCount = processCount - 1;
}

//SYS4
int verhogen(state_t* callerState){
  int* semkey = NULL;
  #if TARGET_UMPS
  semkey = (int*) callerState->reg_a1;
  #elif TARGET_UARM
  semkey = (int*)  callerState->a2;
  #endif
  //check if the passed address is not null
  if(semkey == NULL) {
    setReturn(callerState, -1);
    return FALSE;
  }
  semd_t* sem = getSemd(semkey);
  if(sem){
    pcb_t* unblocked = removeBlocked(semkey);
    if(unblocked) {
      schedInsertProc(unblocked);
    }
  }
  else{
   (*semkey)++;
  }

  return FALSE;
}

//SYS5
int passeren(state_t* callerState){
  int* semkey = NULL;
  #if TARGET_UMPS
  semkey = (int*) callerState->reg_a1;
  #elif TARGET_UARM
  semkey = (int*)  callerState->a2;
  #endif
  //check if the passed address is not null
  if(semkey == NULL) {
    setReturn(callerState, -1);
    return FALSE;
  }

  if(*semkey){
    (*semkey)--;
    return FALSE;
  }
  else{
    mymemcpy(&(currentProcess->p_s), callerState, sizeof(state_t));
    insertBlocked(semkey, currentProcess);
    //need to call the scheduler
    return TRUE;
  }
}

//SYS6
int doIO(state_t* callerState){
  unsigned int command;
  devreg_t *reg;
  int subdevice;

  #if TARGET_UMPS
  command = (unsigned int)callerState->reg_a1;
  reg = (devreg_t*)callerState->reg_a2;
  subdevice = (int)callerState->reg_a3;
  #elif TARGET_UARM
  command = (unsigned int)callerState->a2;
  reg = (devreg_t*)callerState->a3;
  subdevice = (int)callerState->a4;
  #endif
  //check if the passed address is not null
  if(reg == NULL) {
    setReturn(callerState, -1);
    return FALSE;
  }

  if((unsigned int) reg < DEV_REG_ADDR(TERMINAL_LINE, 0)) {
    //generic device
    subdevice = -1;
  }

  unsigned int stat = getStatus(reg, subdevice);
  if (stat != ST_READY && stat != ST_TRANSMITTED)
    //stop the doIO if the device is busy and return to caller
    return FALSE;

  setCommand(reg, command, subdevice);
  int line, dev;
  getLineDev(reg, &line, &dev);

  //P the sem of the requested device
  semd_t* sem = getSemDev(line, dev, subdevice);
  int * s_key = sem->s_key;
  (*s_key)--;
  if((*s_key) < 0){
    mymemcpy(&(currentProcess->p_s), callerState, sizeof(*callerState));
    insertSem(sem);
    insertBlocked(sem->s_key, currentProcess);
    blockedCount++;
    return TRUE;
  }
  PANIC(); //S_KEY SHOULD NEVER BE >= 0
  return FALSE;
};

//SYS7
int specPassup(state_t* callerState){
  int type;
  state_t* newState;
  state_t* oldState;

  #if TARGET_UMPS
  newState = (state_t*)callerState->reg_a3;
  oldState = (state_t*)callerState->reg_a2;
  type = callerState->reg_a1;
  #elif TARGET_UARM
  newState = (state_t*)callerState->a4;
  oldState = (state_t*)callerState->a3;
  type = callerState->a2;
  #endif
  //check if the passed addresses are not null
  if(oldState == NULL || newState == NULL) {
    setReturn(callerState, -1);
    return FALSE;
  }
  switch (type)
  {
    case 0: //sys/break
      if(currentProcess->sysNew != NULL){
        // passup kill
        recursiveKill(currentProcess);
        setReturn(callerState, -1);
        //back to scheduler
        return TRUE;
      }
      currentProcess->sysNew = newState;
      currentProcess->sysOld = oldState;

      break;
    case 1:
      if(currentProcess->TlbNew != NULL){
        // passup kill
        recursiveKill(currentProcess);
        setReturn(callerState, -1);
        //back to scheduler
        return TRUE;
      }
      currentProcess->TlbNew = newState;
      currentProcess->TlbOld = oldState;
      break;
    case 2:
      if(currentProcess->pgtNew != NULL){
        // passup kill
        recursiveKill(currentProcess);
        setReturn(callerState, -1);
        //back to scheduler
        return TRUE;
      }
      currentProcess->pgtNew = newState;
      currentProcess->pgtOld = oldState;
    break;
  }
  setReturn(callerState, 0);

  return FALSE;
};

//SYS8
int getPIDPPID(state_t* callerState){
  unsigned int pid = 0;
  unsigned int ppid = 0;
  pid = (unsigned int) currentProcess;
  ppid = (unsigned int) currentProcess->p_parent;
  // setRegister only sets the register if not NULL
  #if TARGET_UMPS
  setRegister(callerState->reg_a1, pid);
  setRegister(callerState->reg_a2, ppid);
  #elif TARGET_UARM
  setRegister(callerState->a2, pid);
  setRegister(callerState->a3, ppid);
  #endif
  return FALSE;
};

//custom SYSCALLS
int customSyscall(state_t* callerState) {
  if(currentProcess->sysNew != NULL){
    mymemcpy(currentProcess->sysOld, callerState, sizeof(state_t));
    return FALSE;
  }
  return TRUE;
}

/*--------------------------------------------------------------------------------------------------*/
//TLB exception handlers
int TLBHelper(state_t* callerState) {
  if(currentProcess->TlbNew != NULL){
    mymemcpy(currentProcess->TlbOld, callerState, sizeof(state_t));
    return FALSE;
  }
  return TRUE;
}

void TLBManager() {
  cpu_time start_time = updateUserTime(currentProcess);
  state_t* callerState = NULL;
  callerState = (state_t*) TLB_OLDAREA;
  int call_sched = criticalWrapper(&TLBHelper, callerState, start_time, currentProcess);
  if(call_sched) {
    recursiveKill(currentProcess);
    scheduler();
  }
  else
    LDST(currentProcess->TlbNew);
}
