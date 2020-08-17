#ifndef UTILS_H
#define UTILS_H

#include "system.h"
#include "types_bikaya.h"

void mymemcpy(void *dest, void *src, int n);

int critical_wrapper(int (*call)(), state_t* callerState, cpu_time start_time, pcb_t* currentProcess);

cpu_time update_user_time();

void set_return(state_t* caller, int status);

unsigned int get_status(termreg_t* reg, int subdevice);

void debug(int, int);

#endif
