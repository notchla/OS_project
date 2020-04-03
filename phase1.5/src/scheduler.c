#include "scheduler.h"
#include "pcb.h"
#include "system.h"
#include "listx.h"
#include "p1.5test_bikaya_v0.c"


struct list_head readyQueue = LIST_HEAD_INIT(readyQueue);
pcb_t* currentProcess = NULL;
int processCount = 0;
int softBlockCount = 0;

void incProcCount(){
    ++processCount;
}

void schedInsertProc(pcb_t* process){
    if(process->original_priority != 0)
      process->priority = process->original_priority;
    insertProcQ(&readyQueue, process);
}

int getProcCount(){
    return processCount;
}

void aging(){
  pcb_t* ptr;
  list_for_each_entry(ptr, &readyQueue, p_next){
    if(ptr->original_priority == 0)
      ptr->original_priority = ptr->priority;
    ptr->priority += 1;
  }
}

void scheduler(){
  if(emptyProcQ(&readyQueue)) {
    currentProcess = NULL;
    // termprint("empty");
    //work done
    if (processCount == 0) {
      HALT();
    }
    //deadlock
    if ((processCount > 0) && (softBlockCount == 0)) {
      PANIC();
    }
    //nothing to do
    if ((processCount > 0) && (softBlockCount > 0)) {
      #if TARGET_UMPS
      setSTATUS(getSTATUS() | IEON | IECON & ~(IEMASK) & ~(UMODE)); //TODO: chiedere
      #elif TARGET_UARM
      setSTATUS(STATUS_ENABLE_INT(
        STATUS_ALL_INT_ENABLE(getSTATUS() | STATUS_SYS_MODE)));
        #endif
      WAIT();
    }
  }
  else {
    currentProcess = removeProcQ(&readyQueue);
    //order of these operations is important
    aging();
    // setTIMER(TIME_SLICE*TIME_SCALE);
    unsigned int *timer = (unsigned int*) BUS_REG_TIMER;
    *timer = (TIME_SLICE * TIME_SCALE);
    //load currentProcess CPU status
    LDST((&(currentProcess-> p_s)));
  }
}
