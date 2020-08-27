#ifndef UTILS_H
#define UTILS_H

#include "system.h"
#include "asl.h"
#include "types_bikaya.h"

void mymemcpy(void *dest, void *src, int n);

int critical_wrapper(int (*call)(), state_t* callerState, cpu_time start_time, pcb_t* currentProcess);

cpu_time update_user_time();

void set_return(state_t* caller, int status);

void set_command(devreg_t* reg, unsigned int command, int subdevice);

void get_line_dev(devreg_t* reg, int* line, int* dev);

unsigned int get_status(devreg_t* reg, int subdevice);

void debug(int, int);

semd_t * getSemDev(int line, int devnum, int read);

#endif
