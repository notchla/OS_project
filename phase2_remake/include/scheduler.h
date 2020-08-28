/* scheduler function and additional helper */
#ifndef _SCHED_
#define _SCHED_
#include "types_bikaya.h"
#include "listx.h"

extern struct list_head readyQueue;
extern pcb_t* currentProcess;
extern int processCount;
extern int blockedCount;
extern pcb_t* idle_ptr;

//manages the process execution using a simple round-robin preemptive scheduler, with priority and aging
void scheduler();
//inserts a process to the readyQueue
void schedInsertProc(pcb_t* process);

#endif
