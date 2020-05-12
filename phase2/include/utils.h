#ifndef UTILS_H
#define UTILS_H

#include "system.h"
#include "types_bikaya.h"


void mymemcpy(void *dest, void *src, int n);

int critical_wrapper(int (*call)(), state_t* callerState, unsigned int start_time, pcb_t* currentProcess);

unsigned int update_user_time();

void set_return(state_t* caller, int status);

#endif
