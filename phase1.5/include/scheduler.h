#ifndef _SCHED_
#define _SCHED_
#include "types_bikaya.h"

void scheduler();
void incProcCount();
void schedInsertProc(pcb_t* process);
int getProcCount();

#endif