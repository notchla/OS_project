#ifndef ASL_H
#define ASL_H

#include <types_bikaya.h>
#include "types_bikaya.h"

static semdev semDevice; // struct that contains the semaphores for the devices

int semDev[DEV_USED_INTS * DEV_PER_INT]; //int array that holds the values of the dev semaphores

/* ASL handling functions */
semd_t* getSemd(int *key);
void initASL();

int insertBlocked(int *key,pcb_t* p);
pcb_t* removeBlocked(int *key);
pcb_t* outBlocked(pcb_t *p);
pcb_t* headBlocked(int *key);
void outChildBlocked(pcb_t *p);
void insertSem(semd_t* semaphore);

#endif
