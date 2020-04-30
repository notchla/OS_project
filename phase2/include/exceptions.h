#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

#include "types_bikaya.h"

void trapHandler();

//called when there is an incoming syscall to be managed
void syscallHandler();

void TLBManager();

//terminates the current process and all the children and removes them from the ready_queue
void kill();

//recursively deletes the while tree of process. auxiliary function to kill()
void recursive_kill(pcb_t* process);

#endif
