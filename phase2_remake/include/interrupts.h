#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H

#include "system.h"

//gestore degli interrupt
void interruptHandler();
//helper for lines > 3 (devices)
int deviceHandler(state_t* oldstatus);
int termHandler(state_t* oldstatus);
int timeHandler(state_t* oldstatus);

void exitInterrupt(state_t* oldstatus);

void verhogenDevice(int line, unsigned int status, int deviceNumber);
void ACKDevice(unsigned int* commandRegister);

int getDeviceNumber(int line);
int log2(unsigned int);
int lowest_set(int);

unsigned int tx_status(termreg_t *tp);
unsigned int rx_status(termreg_t *tp);
#endif
