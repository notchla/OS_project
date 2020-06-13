#include "scheduler.h"
#include "pcb.h"
#include "system.h"
#include "listx.h"
#include "p2test_bikaya_v0.2.h"
#include "utils.h"

struct list_head readyQueue = LIST_HEAD_INIT(readyQueue);
pcb_t* currentProcess = NULL;
int processCount = 0;

void incProcCount(){
    ++processCount;
}

void schedInsertProc(pcb_t* process){
    process->priority = process->original_priority;
    insertProcQ(&readyQueue, process);
}

int getProcCount(){
    return processCount;
}

void aging(){
  pcb_t* ptr;
  list_for_each_entry(ptr, &readyQueue, p_next){
    ptr->priority += 1;
  }
}

void scheduler(){

  if(emptyProcQ(&readyQueue)) {
    currentProcess = NULL;
    //work done
    if (processCount == 0) {
      HALT();
    }
  }
  else {
    currentProcess = removeProcQ(&readyQueue);
    //order of these operations is important
    aging();
    // setTIMER(TIME_SLICE*TIME_SCALE);

    unsigned int time = *((unsigned int *) BUS_REG_TOD_LO);
    currentProcess->last_restart = time;
    if(!currentProcess->first_activation){
      currentProcess->first_activation = time;
    }

    unsigned int *timer = (unsigned int*) BUS_REG_TIMER;
    *timer = (TIME_SLICE * TIME_SCALE);
    //load currentProcess CPU status
    LDST((&(currentProcess-> p_s)));
  }
}
