/*
    asl.h declare the functions that haldles the process and device semaphores.

*/

#ifndef ASL_H
#define ASL_H

#include <types_bikaya.h>
#include "const_bikaya.h"
#include "system.h"

extern semdev semDev; //struct that declare the device semaphores

/* ASL handling functions */

//key is a semaphore key, returns the active semaphore that has semkey == key, null otherwise
semd_t* getSemd(int *key);
//initialize list of free senaphores and device registers
void initASL();
//insert semaphore in ASL
void insertSem(semd_t* semaphore);
//insert the pcb pointed by p in the queue of process blocked in the semaphore with semkey == key
int insertBlocked(int *key,pcb_t* p);
//return and remove the first blocked pcb from the semaphore with semkey == key
pcb_t* removeBlocked(int *key);
//return and remove the first blocked pcb from the device semaphore with semkey == key
pcb_t* removeBlockedonDevice(int* key);
//return and remove the pcb pointed by p from his semaphore, if the blocked queue is then empty, remove the semaphore from ASL and insert it in semdfree, return null if p isn't blocked on any device
pcb_t* outBlocked(pcb_t *p);
//return and remove the pcb pointed by p from his device semaphore, if the blocked queue is then empty, remove the semaphore from ASL
pcb_t* DevicesOutBlocked(pcb_t* p);
//return true if pcb is blocked on a semaphore
unsigned int isBlocked(pcb_t* p);
//return the first blocked pcb on the semaphore without removing it
pcb_t* headBlocked(int *key);
//remove the pcb pointed by p and all his childs form their corrispective semaphores
void outChildBlocked(pcb_t *p);

#endif
