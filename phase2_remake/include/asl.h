#ifndef ASL_H
#define ASL_H

#include <types_bikaya.h>
#include "const_bikaya.h"
#include "system.h"

extern int semdevices[(1 + DEV_USED_INTS) * DEV_PER_INT];

/* ASL handling functions */
semd_t* getSemd(int *key);
void initASL();

int insertBlocked(int *key,pcb_t* p);
pcb_t* removeBlocked(int *key);
pcb_t* outBlocked(pcb_t *p);
pcb_t* headBlocked(int *key);
void outChildBlocked(pcb_t *p);

#endif