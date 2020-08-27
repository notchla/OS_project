#ifndef ASL_H
#define ASL_H

#include <types_bikaya.h>
#include "const_bikaya.h"
#include "system.h"

extern semdev semDev;

/* ASL handling functions */
semd_t* getSemd(int *key);
void initASL();

void insertSem(semd_t* semaphore);

int insertBlocked(int *key,pcb_t* p);
pcb_t* removeBlocked(int *key);
pcb_t* removeBlockedonDevice(int* key);
pcb_t* outBlocked(pcb_t *p);
unsigned int isBlocked(pcb_t* p);
pcb_t* headBlocked(int *key);
void outChildBlocked(pcb_t *p);

#endif
