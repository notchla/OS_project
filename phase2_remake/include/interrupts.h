/* functions to handle the various interrupts of the Kaya system */

#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H

#include "system.h"

/*
 * main handler.
 * Evoked when a interrupt arises. Marshals the incoming request and call the
 * dedicated sub-handler following the interrupt priority.
 */
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
//perform handler closing operations
void exitInterrupt(state_t* oldstatus);

#endif
