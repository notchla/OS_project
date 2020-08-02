#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H

#include "system.h"

//gestore degli interrupt
void interruptHandler();
//helper for lines > 3 (devices)
void deviceHandler();

int getDeviceNumber(int line);
int log2(unsigned int);
int lowest_set(int);

unsigned int tx_status(termreg_t *tp);
unsigned int rx_status(termreg_t *tp);
#endif
