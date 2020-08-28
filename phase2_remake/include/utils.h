#ifndef UTILS_H
#define UTILS_H

#include "system.h"
#include "scheduler.h"
#include "asl.h"
#include "types_bikaya.h"

void mymemcpy(void *dest, void *src, int n);

/* critical_wrapper tracks kernel/user time in a tidy way.
*  inputs:
*   _ call: a pointer to the requested system call (null if updating from interrupt);
*   _ callerState: state vector of the caller process;
*   _ start_time: TOD saved at the beginning of the critical section;
*   _ currentProcess: process control block of the calling process.
*  outputs:
*   _ call_sched: forwards the bool returned by the calls.
*/
int critical_wrapper(int (*call)(), state_t* callerState, cpu_time start_time, pcb_t* currentProcess);

cpu_time update_user_time();

void set_return(state_t* caller, int status);

void set_command(devreg_t* reg, unsigned int command, int subdevice);

void get_line_dev(devreg_t* reg, int* line, int* dev);

unsigned int get_status(devreg_t* reg, int subdevice);

void debug(int, int);

semd_t * getSemDev(int line, int devnum, int read);

void verhogenKill(pcb_t* process);

int pid_in_readyQ(pcb_t* pid);

int set_register(unsigned int reg, unsigned int val);

int log2(unsigned int n);

int lowest_set(int number);

int getDeviceNumber(int line);

unsigned int tx_status(termreg_t *tp);

unsigned int rx_status(termreg_t *tp);


#endif
