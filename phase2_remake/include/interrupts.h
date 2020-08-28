#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H

#include "system.h"

void interruptHandler();
//interval timer handler
int timerHandler(state_t* oldstatus);
//handlers for lines > 3 (devices)
int diskHandler(state_t* oldstatus);
int tapeHandler(state_t* oldstatus);
int netHandler(state_t* oldstatus);
int printerHandler(state_t* oldstatus);
int termHandler(state_t* oldstatus);

//helper methods
void devHandler(int line, state_t* oldstatus);
void exitInterrupt(state_t* oldstatus);
void verhogenDevice(int line, unsigned int status, int deviceNumber, int read);
void ACKDevice(unsigned int* commandRegister);

#endif
