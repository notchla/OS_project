#include "scheduler.h"
#include "pcb.h"
#include "system.h"
#include "listx.h"
#include "p2test_bikaya_v0.3.h"
#include "termprint.h"
#include "utils.h"

struct list_head readyQueue = LIST_HEAD_INIT(readyQueue);
pcb_t* currentProcess = NULL;
int processCount = 0;
int blockedCount = 0;
pcb_t* idle_ptr = NULL;

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
    if(ptr != idle_ptr) //idle_proc must be always the last pcb in the readyqueue
      ptr->priority += 1;
  }
}

void scheduler(){
  if(!emptyProcQ(&readyQueue)) {
    /* order of these operations is IMPORTANT */
    currentProcess = removeProcQ(&readyQueue);
    // the only active process is idle_proc, system now waiting
    if(currentProcess == idle_ptr) {
      if(processCount == 0) {
        //work done
        HALT();
      } else {
        if(blockedCount == 0) {
          //deadlock
          PANIC();
        }
        //already doing the waiting with idle_proc
      }
    }
    aging();

    // setTIMER(TIME_SLICE*TIME_SCALE);
    cpu_time *timer = (unsigned int*) BUS_REG_TIMER;
    *timer = (TIME_SLICE * TIME_SCALE);
    //set/update the restart/activation times in the pcb
    cpu_time time = *((unsigned int *)BUS_REG_TOD_LO);
    currentProcess->last_restart = time;
    if(currentProcess->first_activation == 0){
      currentProcess->first_activation = time;
    }
    //load currentProcess CPU status
    LDST((&(currentProcess-> p_s)));
  }
}
