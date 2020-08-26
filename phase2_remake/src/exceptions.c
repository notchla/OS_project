#include "exceptions.h"
#include "scheduler.h"
#include "pcb.h"
#include "listx.h"
#include "system.h"
#include "types_bikaya.h"
#include "utils.h"
#include "termprint.h"
#include "asl.h"

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
  kill_current();
}

void syscallHandler() {
  cpu_time start_time = update_user_time(currentProcess);

  int syscallRequest = 0;

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
  // callerState-> pc = callerState-> pc + WORD_SIZE; NOT NEEDED IN UARM
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
    default: ;
      state_t* callerState = NULL;
      #if TARGET_UMPS
      callerState = (state_t*) SYS_OLDAREA;
      #elif TARGET_UARM
      callerState = (state_t*) SYSBK_OLDAREA;
      #endif

      if(currentProcess->sysNew != NULL){
        mymemcpy(currentProcess->sysOld, callerState, sizeof(state_t));
        LDST(currentProcess->sysNew);
      }
      kill_current();
    break;
  }
  int call_sched = critical_wrapper(requested_call, callerState, start_time, currentProcess);
  if(call_sched)
    //nothing to do
    scheduler();
  else
    LDST(callerState);
}
int pid_in_readyQ(pcb_t* pid){
  pcb_t* ptr;
  list_for_each_entry(ptr, &readyQueue, p_next){
    if (ptr == pid)
      return 1;
  }
  return 0;
}

void set_register(unsigned int reg, unsigned int val) {
  *(unsigned int*) reg = val;
}

void get_params(state_t* callerState, unsigned int* p1, unsigned int* p2, unsigned int* p3) {
  #if TARGET_UMPS
  if(p1 != NULL)
    p1 = (unsigned int*) callerState->reg_a1;
  if(p2 != NULL)
    p2 = (unsigned int*) callerState->reg_a2;
  if(p3 != NULL)
    p3 = (unsigned int*) callerState->reg_a3;
  #elif TARGET_UARM
  if(p1 != NULL)
    p1 = (unsigned int*) callerState->a2;
  if(p2 != NULL)
    p2 = (unsigned int*) callerState->a3;
  if(p3 != NULL)
    p3 = (unsigned int*) callerState->a4;
  #endif
}

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
  else if(!pid_in_readyQ(kpid) && !isBlocked(kpid)){//si potrebbe fare che isblocked ritorni il semaforo in cui kpid è bloccato
    set_return(callerState, -1);
    return FALSE;
  }
  if(!emptyChild(kpid)){
    //process has child processes, killing the whole tree
    recursive_kill(kpid);
  } else {
    //single process
    if (kpid->p_semkey) {
      //process was blocked on a semaphore
      outBlocked(kpid);
    }
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
  set_return(callerState, 0);
  // if the current process was terminated the scheduler gains control
  // the previous state is loaded otherwise
  return (killed_current);
}

// helper for the terminateprocess syscall
void recursive_kill(pcb_t* process){
  if(process == NULL)
    return;
  pcb_t* child;
  // list_for_each_entry(child, &process->p_child, p_sib){
  //   // call on every branch
  //   recursive_kill(child);
  // }
  while(!list_empty(&process->p_child)){
    pcb_t* child = container_of(process->p_child.next, pcb_t, p_sib);
    recursive_kill(child);
  }
  if(process->p_semkey != NULL) {
    outBlocked(process);
  }
  outChild(process);
  if(process != currentProcess){
    outProcQ(&readyQueue, process);
    // outChild(process);
  }
  // else {
  //   //current process cases
  //   outChild(currentProcess);
  // }
  freePcb(process);
  processCount = processCount - 1;
}

int get_cpu_time(state_t* callerState){
  cpu_time time = *(unsigned int*) BUS_REG_TOD_LO;
  #if TARGET_UMPS
  set_register(callerState->reg_a3, time - currentProcess->first_activation);
  //current kernel time, countinmg the time elapsed until the call
  set_register(callerState->reg_a2, currentProcess->kernel_timer + time - currentProcess->last_stop);
  set_register(callerState->reg_a1, currentProcess->user_timer);
  #elif TARGET_UARM
  set_register(callerState->a4, time - currentProcess->first_activation);
  set_register(callerState->a3, currentProcess->kernel_timer + time - currentProcess->last_stop);
  set_register(callerState->a2, currentProcess->user_timer);
  #endif
  return FALSE;
};


int create_process(state_t* callerState){
    pcb_t* newProc = allocPcb();
  if(newProc == NULL){
    set_return(callerState, -1);
    //return to caller
    return FALSE;
  }
  int priority;
  state_t* new_state;
  #if TARGET_UMPS
  new_state = (state_t*) callerState->reg_a1;
  priority = callerState->reg_a2;
  //set the cpid to the pcb address
  if(callerState->reg_a3 != 0)
    set_register(callerState->reg_a3, (unsigned int) newProc);
  #elif TARGET_UARM
  new_state = (state_t*) callerState->a2;
  priority = callerState->a3;
  //set the cpid to the pcb address
  if(callerState->a4 != 0)
    set_register(callerState->a4, (unsigned int) newProc);
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
  set_return(callerState, 0);
  //return to caller
  return FALSE;
};

// int verhogen(state_t* callerState){
//   int* sem = NULL;
//   #if TARGET_UMPS
//   sem = (int*) callerState->reg_a1;
//   #elif TARGET_UARM
//   sem = (int*)  callerState->a2;
//   #endif
//   (*sem)++;
//   if((*sem) <= 0) {
//     pcb_t* unblocked = removeBlocked(sem);
//     if(unblocked) {
//       schedInsertProc(unblocked);
//       processCount++;
//     }
//   }
//   //return to caller
//   return FALSE;
// };

int verhogen(state_t* callerState){
  int* semkey = NULL;
  #if TARGET_UMPS
  semkey = (int*) callerState->reg_a1;
  #elif TARGET_UARM
  semkey = (int*)  callerState->a2;
  #endif

  semd_t* sem = getSemd(semkey);
  if(sem){
    pcb_t* unblocked = removeBlocked(semkey);
    blockedCount--;
    if(unblocked) {
      schedInsertProc(unblocked);
    }
  }
  else{
   (*semkey)++;
  }

  return FALSE;
}

// int passeren(state_t* callerState){
//   int* sem = NULL;
//   #if TARGET_UMPS
//   sem = (int*) callerState->reg_a1;
//   #elif TARGET_UARM
//   sem = (int*)  callerState->a2;
//   #endif
//   (*sem)--;
//   if((*sem) < 0) {
//     mymemcpy(&(currentProcess->p_s), callerState, sizeof(state_t));
//     insertBlocked(sem, currentProcess);
//     //need to call the scheduler
//     return TRUE;
//   }
//   //return to caller
//   return FALSE;
// };

int passeren(state_t* callerState){
  int* sem = NULL;
  #if TARGET_UMPS
  sem = (int*) callerState->reg_a1;
  #elif TARGET_UARM
  sem = (int*)  callerState->a2;
  #endif

  if(*sem){
    (*sem)--;
    return FALSE;
  }
  else{
    mymemcpy(&(currentProcess->p_s), callerState, sizeof(state_t));
    insertBlocked(sem, currentProcess);
    blockedCount++;
    //need to call the scheduler
    return TRUE;
  }
}

void set_command(termreg_t* reg, unsigned int command, int subdevice){
  if(subdevice)
    reg->recv_command = command;
  else
    reg->transm_command = command;
}

void get_line_dev(termreg_t* reg, int* line, int* dev){
  unsigned int base_dev = DEV_REG_ADDR(DEV_UNUSED_INTS, 0);
  unsigned int offset = ((unsigned int) reg) - base_dev;
  unsigned int line_offset = (unsigned int) offset / DEV_REG_SIZE;
  *line = (int) line_offset / DEV_PER_INT + DEV_UNUSED_INTS;
  *dev = line_offset % DEV_PER_INT;
}

int do_IO(state_t* callerState){
  unsigned int command;
  termreg_t *reg;
  int subdevice;

  #if TARGET_UMPS
  command = (unsigned int)callerState->reg_a1;
  reg = (termreg_t*)callerState->reg_a2;
  subdevice = (int)callerState->reg_a3;
  #elif TARGET_UARM
  command = (unsigned int)callerState->a2;
  reg = (termreg_t*)callerState->a3;
  subdevice = (int)callerState->a4;
  #endif

  unsigned int stat = get_status(reg, subdevice);

  if (stat != 1 && stat != 5)
    term_puts("device not ready");

  set_command(reg, command, subdevice);

  int line, dev;
  get_line_dev(reg, &line, &dev);

  if(!subdevice) //if transmission
    line += 1;

  int* s_key = &semdevices[(line-DEV_UNUSED_INTS)*DEV_PER_INT + dev];

  (*s_key)--;

  if((*s_key) < 0){
    mymemcpy(&(currentProcess->p_s), callerState, sizeof(*callerState));
    insertBlocked(s_key, currentProcess);
    blockedCount++;
    return TRUE;
  }
  PANIC(); //S_KEY SHOULD NEVER BE >= 0
};

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
        return passup_kill(callerState);
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
        return passup_kill(callerState);
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
        return passup_kill(callerState);
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
  set_return(callerState, 0);

  return FALSE;
};

int get_pid_ppid(state_t* callerState){
  unsigned int pid = 0;
  unsigned int ppid = 0;
  #if TARGET_UMPS
  pid = callerState->reg_a1;
  ppid = callerState->reg_a2;
  #elif TARGET_UARM
  pid = callerState->a2;
  ppid = callerState->a3;
  #endif
  if(pid != 0) {
    pid = (unsigned int) currentProcess;
    #if TARGET_UMPS
    set_register(callerState->reg_a1, pid);
    #elif TARGET_UARM
    set_register(callerState->a2, pid);
    #endif
  }
  if(ppid != 0) {
    ppid = (unsigned int) currentProcess->p_parent;
    #if TARGET_UMPS
    set_register(callerState->reg_a2, ppid);
    #elif TARGET_UARM
    set_register(callerState->a3, ppid);
    #endif
  }
  return FALSE;
};

int passup_kill(state_t* callerState){
  set_return(callerState, -1);
  pcb_t* kpid = currentProcess;
  if(!emptyChild(kpid)){
    //process has child processes, killing the whole tree
    recursive_kill(kpid);
  } else {
    //single process
    if (kpid->p_semkey) {
      //process was blocked on a semaphore
      outBlocked(kpid);
      if(kpid->p_semkey <= &(semdevices[0]) && kpid->p_semkey >= &(semdevices[(1 + DEV_USED_INTS) * DEV_PER_INT - 1])) {
        // not blocked on device
        *(kpid->p_semkey)++;
      }
    }
    // remove killed process from parent
    outChild(kpid);
    // back to free process list
    freePcb(kpid);
    processCount = processCount - 1;
  }
  //the current process was terminated the scheduler gains control
  return TRUE;
}

void kill_current(){
  pcb_t* kpid = currentProcess;
  if(!emptyChild(kpid)){
    //process has child processes, killing the whole tree
    recursive_kill(kpid);
  } else {
    //single process
    if (kpid->p_semkey) {
      //process was blocked on a semaphore
      outBlocked(kpid);
    }
    // remove killed process from parent
    outChild(kpid);
    // back to free process list
    freePcb(kpid);
    processCount = processCount - 1;
  }
  //the current process was terminated the scheduler gains control
  scheduler();
}

void TLBManager() {
  state_t* callerState = NULL;
  callerState = (state_t*) TLB_OLDAREA;
  if(currentProcess->TlbNew != NULL){
    mymemcpy(currentProcess->TlbOld, callerState, sizeof(state_t));
    LDST(currentProcess->TlbNew);
  }
  kill_current();
}
