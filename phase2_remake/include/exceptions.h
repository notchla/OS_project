#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

#include "types_bikaya.h"
#include "const_bikaya.h"

void trapHandler();

//called when there is an incoming syscall to be managed
void syscallHandler();

void TLBManager();

int get_cpu_time(state_t* callerState);

int create_process(state_t* callerState);
//terminates the current process and all the children and removes them from the ready_queue
int kill();

int verhogen(state_t* callerState);

int passeren(state_t* callerState);

int get_pid_ppid(state_t* callerState);

int do_IO(state_t* callerState);

int spec_passup(state_t* callerState);

//recursively deletes the while tree of process. auxiliary function to kill()
void recursive_kill(pcb_t* process);

int passup_kill(state_t* callerState);//called in specpassup if specpassup is invoke more than once for the same type

void kill_current(); //used in traphandler and tlbhanlder

#endif
