#include "scheduler.h"
#include "pcb.h"


static LIST_HEAD(readyQueue);
static pcb_t* currentProcess;
static int processCount = 0;
static int  softBlockCount = 0;

void incProcCount(){
    ++processCount;
}

void schedInsertProc(pcb_t* process){
    insertProcQ(&readyQueue, process);
}

int getProcCount(){
    return processCount;
}



void scheduler(){

}