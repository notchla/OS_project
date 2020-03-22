#ifndef _SCHED_
#define _SCHED_
#include "types_bikaya.h"
#include "listx.h"

extern struct list_head readyQueue;
extern pcb_t* currentProcess;
extern int processCount;
extern int  softBlockCount;

void scheduler();
void incProcCount();
void schedInsertProc(pcb_t* process);
int getProcCount();

#endif
