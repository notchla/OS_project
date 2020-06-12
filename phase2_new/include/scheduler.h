#ifndef _SCHED_
#define _SCHED_
#include "types_bikaya.h"
#include "listx.h"

extern struct list_head readyQueue;
extern pcb_t* currentProcess;
extern int processCount;
extern int  softBlockCount;

void mymemcpy(void *dest, void *src, int n);
//gestisce l'esecuzione dei processi tramite priorita e meccanismo di aging
void scheduler();
//incrementa il numero di processi
void incProcCount();
//inserisce un processo nella ready queue
void schedInsertProc(pcb_t* process);
//ritorna il numero di processi
int getProcCount();

#endif
