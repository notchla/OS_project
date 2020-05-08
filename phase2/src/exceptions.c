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
  kill(NULL, 0);
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
    case GETCPUTIME:
      Get_Cpu_Time(callerState, start_time);
    break;
    case CREATEPROCESS:
      create_process(callerState, start_time);
    break;
    case TERMINATEPROCESS:
      kill(callerState, start_time);
    break;
    //syscall not recognized
    default:
      PANIC();
    break;
  }
}
 // bidogna aggiungere il tempo passato a gestire gli interrupt?
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
  // mymemcpy(new_state, &(newProc->p_s), sizeof(*newProc)); //cosi è al contrario
  mymemcpy(&(newProc->p_s), new_state, sizeof(*new_state));
  processCount = processCount + 1;
  #if TARGET_UMPS
  callerState->reg_v0 = 0;
  #elif TARGET_UARM
  callerState->a1 = 0;
  #endif
  update_kernel_time(start_time); //deve essere chiamata prima di restituire il controllo

  //return to caller
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

void kill(state_t* callerState, unsigned int start_time) {
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
    LDST(callerState);
    #elif TARGET_UARM
    callerState->a1 = -1;
    LDST(callerState);
    #endif
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
  //call to scheduler to advance to the next process waiting
  update_kernel_time(start_time);

  #if TARGET_UMPS
  callerState->reg_v0 = 0;
  #elif TARGET_UARM
  callerState->a1 = 0;
  #endif
  //currentProcess is dead, and we killed it. scheduler gains control to advance execution
  if(currentProcess == kpid)
    scheduler();
  else
    LDST(callerState);
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

void verhogen(state_t* callerState) {
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
  LDST(callerState);
}

void passeren(state_t* callerState) {
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
    scheduler();
  }
  //return to caller
  LDST(callerState);
}

void get_pid_ppid(state_t* callerState, unsigned int start_time) {
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
  LDST(callerState);
}

void Do_IO(state_t* callerState, unsigned int start_time){
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
  LDST(callerState);
}

void passup_kill(state_t* callerState, unsigned int start_time){
  #if TARGET_UMPS
  callerState->reg_v0 = -1;
  #elif TARGET_UARM
  callerState->a1 = -1;
  #endif
  //todo update kernel time
  kill(NULL, start_time);
}

void Spec_Passup(state_t* callerState, unsigned int start_time){
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
        passup_kill(callerState, start_time);
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
        passup_kill(callerState, start_time);
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
        passup_kill(callerState, start_time);
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

  LDST(callerState);
}

void TLBManager() {
  state_t* callerState = NULL;
  callerState = (state_t*) TLB_OLDAREA;
  if(currentProcess->TlbNew != NULL){
    mymemcpy(currentProcess->TlbOld, callerState, sizeof(state_t));
    LDST(currentProcess->TlbNew);
  }
  //no hanlder defined
  kill(NULL, 0); //todo time
}
