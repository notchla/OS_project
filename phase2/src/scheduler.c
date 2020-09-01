#include "scheduler.h"
#include "pcb.h"
#include "system.h"
#include "listx.h"
#include "p2test_bikaya_v0.3.h"
#include "termprint.h"
#include "utils.h"

/* global variables initialization */
struct list_head readyQueue = LIST_HEAD_INIT(readyQueue);
pcb_t* currentProcess = NULL;
int processCount = 0;
int blockedCount = 0;
pcb_t* idle_ptr = NULL;

void schedInsertProc(pcb_t* process){
    process->priority = process->original_priority;
    insertProcQ(&readyQueue, process);
}

//performs aging on all the processes except for idle_proc
void aging(){
  pcb_t* ptr;
  list_for_each_entry(ptr, &readyQueue, p_next){
    //idle_proc must be always the last pcb in the readyqueue
    //aging is not performed on idle_proc
    if(ptr != idle_ptr)
      ptr->priority += 1;
  }
}

void scheduler(){
  if(!emptyProcQ(&readyQueue)) {
    /* order of these operations is IMPORTANT, halt condition must be checked before deadlock condition*/

    //get a process from the queue
    currentProcess = removeProcQ(&readyQueue);
    // the only active process is idle_proc, system now waiting
    if(currentProcess == idle_ptr) {
      if(processCount == 0) {
        //work done. idle_ptr does not count in the processCount
        HALT();
      } else {
        if(blockedCount == 0) {
          //deadlock
          PANIC();
        }
      }
    }

    aging();

    // set interval timer to the time slice of the current process
    cpu_time *timer = (unsigned int*) BUS_REG_TIMER;
    *timer = (TIME_SLICE * TIME_SCALE);

    //set/update the restart/activation times in the pcb
    cpu_time time = *((unsigned int *)BUS_REG_TOD_LO);
    currentProcess->last_restart = time;
    if(currentProcess->first_activation == 0){
      currentProcess->first_activation = time;
    }
    //load the newly loaded process
    LDST((&(currentProcess-> p_s)));
  }
}
