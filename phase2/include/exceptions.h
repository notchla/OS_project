#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

#include "types_bikaya.h"

void trapHandler();

//called when there is an incoming syscall to be managed
void syscallHandler();

void TLBManager();

void Get_Cpu_Time(state_t* callerState, unsigned int start_time);

void create_process(state_t* callerState, unsigned int start_time);

//terminates the current process and all the children and removes them from the ready_queue
void kill();

void get_pid_ppid(state_t* callerState, unsigned int start_time);

//recursively deletes the while tree of process. auxiliary function to kill()
void recursive_kill(pcb_t* process);
#endif
