#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

#include "types_bikaya.h"
#include "const_bikaya.h"

//called on trap invocation
void trapHandler();

//called when there is an incoming syscall to be managed
void syscallHandler();
//called on a tlb violation
void TLBManager();
//SYSCALL(1, unsigned int* user, unsigned int* kernel, unsigned int* wallclock)
//set the usertime, the kerneltime and total time of the process
int getCPUTime(state_t* callerState);
//SYSCALL(2, state_t* statep. int priority, void **cpid)
//create a new process as a child of the caller, statep is the initial state of the new process. If the creation ended well the return value is 0 else -1.
//If cpid!=NULL and the creation ended well *cpid is set to the id of the created child, rapresented by the address of his pcb_t
int createProcess(state_t* callerState);
//SYSCALL(3, void* pid, 0, 0)
//terminates the current process and all the children and removes them from the ready_queue or relative semaphore
int kill(state_t* callerState);
//SYSCALL(4, int* semmaddr, 0, 0)
//release operation on the semaphore with key == semmaddr
int verhogen(state_t* callerState);
//SYSCALL(5, int* semmaddr, 0, 0)
//request operation on the semaphore with key == semmaddr
int passeren(state_t* callerState);
//SYSCALL(8, void **pid, void **ppid, 0);
//set the id of the current process in *pid and the id of the currentprocess parent in *ppid
int getPIDPPID(state_t* callerState);
//invoked when the first parameter of SYSCALL is > 8, if a custom syscall for the invoking proc is defined load the state pointed by sysnew (see pcb_t definition), else terminate the invoking process
int customSyscall(state_t* callerState);
//SYSCALL(6, unsigned int command, unsigned int* register, int subdevice)
//I/O operation, command is copied in the command field of the register pointed by register. For terminal device, subdevice == 0 corresponds to a transmission and 1 corresponds to a recv
int doIO(state_t* callerState);
//SYSCALL(7, int type, state_t* old, state_t* new);
//this SYSCALL define which hanlder to call in case of syscall/breakpoint(type=0), TLB(type=1), Program Trap(type=2) trap. The syscall must be called once for type otherwise the process is killed
int specPassup(state_t* callerState);

//recursively deletes the whole tree of process radicated in process. auxiliary function to kill()
void recursiveKill(pcb_t* process);

#endif
