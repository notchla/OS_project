#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H

#include "system.h"

//interrupt handler loaded during init in the interrupt rom area
void interruptHandler();
//helper for lines > 3 (devices)
void deviceHandler();

int getDeviceNumber(int line);

devreg_t* getDeviceRegister(int line, int deviceNumber);

unsigned int tx_status(termreg_t *tp);

#endif
