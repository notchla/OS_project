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
  //to be implemented
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

// void kill() {
//   if(!emptyChild(currentProcess)){
//     //process has child processes, killing the whole tree
//     recursive_kill(currentProcess);
//   }
//   else{
//     //process is single
//     outChild(currentProcess);
//     freePcb(currentProcess);
//     processCount = processCount - 1;
//   }
//   currentProcess = NULL;
//   //call to scheduler to advance to the next process waiting
//   scheduler();
// }

// void recursive_kill(pcb_t* process){
//   if(process == NULL)
//     return;
//   pcb_t* child;
//   list_for_each_entry(child, &process->p_child, p_sib){
//     recursive_kill(child);
//   }
//   if(outProcQ(&readyQueue, process)){
//     processCount = processCount - 1;
//     freePcb(process);
//   }
//   //current process case
//   else {
//     outChild(process);
//     freePcb(process);
//     currentProcess = currentProcess - 1;
//   }
// }

int get_cpu_time(state_t* callerState){
  cpu_time time = *(unsigned int*) BUS_REG_TOD_LO;
  #if TARGET_UMPS
  *(cpu_time*)callerState->reg_a3 = time - currentProcess->first_activation;
  //current kernel time, countinmg the time elapsed until the call
  *(cpu_time*)callerState->reg_a2 = currentProcess->kernel_timer + time - currentProcess->last_stop;
  *(cpu_time*)callerState->reg_a1 = currentProcess->user_timer;
  #elif TARGET_UARM
  *(cpu_time*)callerState->a4 = time - currentProcess->first_activation;
  *(cpu_time*)callerState->a3 = currentProcess->kernel_timer + time - currentProcess->last_stop;
  *(cpu_time*)callerState->a2 = currentProcess->user_timer;
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
  newProc->first_activation = 0;
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
int kill(state_t* state){};

int verhogen(state_t* callerState){
  int* sem = NULL;
  #if TARGET_UMPS
  sem = (int*) callerState->reg_a1;
  #elif TARGET_UARM
  sem = (int*)  callerState->a2;
  #endif
  (*sem)++;
  if((*sem) <= 0) {
    pcb_t* unblocked = removeBlocked(sem);
    debug(3, (int)unblocked);
    if(unblocked) {
      schedInsertProc(unblocked);
      processCount++;
    }
  }
  //return to caller
  return FALSE;
};

int passeren(state_t* callerState){
  int* sem = NULL;
  #if TARGET_UMPS
  sem = (int*) callerState->reg_a1;
  #elif TARGET_UARM
  sem = (int*)  callerState->a2;
  #endif
  (*sem)--;
  if((*sem) < 0) {
    mymemcpy(&(currentProcess->p_s), callerState, sizeof(state_t));
    insertBlocked(sem, currentProcess);
    debug(2, (int)currentProcess);
    //need to call the scheduler
    return TRUE;
  }
  //return to caller
  return FALSE;
};

void set_command(termreg_t* reg, unsigned int command, int subdevice){
  if(subdevice)
    reg->recv_command = command;
  else
    reg->transm_command = command;
}

void get_line_dev(termreg_t* reg, int* line, int* dev){
  for (int i = 0; i < N_EXT_IL; i++){
    for (int j = 0; j < N_DEV_PER_IL; j++){
      if( reg == (termreg_t*) DEV_REG_ADDR(i + 3, j)){
        *line = i + 3;
        *dev = j;
        return ;
      }
    }
  }
}

//TODO bloccare il processo che chiama la sys se il il device richiesto è in uso da un altro processo(controllare se un processo è bloccato nel semaforo).scrivere la getlinedev usando indirizzi di memoria facendo modulo e divisione
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

  int* s_key = &semdevices[(line-3)*DEV_PER_INT + dev];

  (*s_key)--;

  if((*s_key) < 0){
    mymemcpy(&(currentProcess->p_s), callerState, sizeof(*callerState));
    insertBlocked(s_key, currentProcess);
    return TRUE;
  }
  PANIC(); //S_KEY SHOULD NEVER BE >= 0
};

int spec_passup(state_t* state){};
int get_pid_ppid(state_t* state){};

void TLBManager() {
  //to be implemented
}
