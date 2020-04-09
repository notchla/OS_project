#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

#include "types_bikaya.h"

void trapHandler();

//gestore delle syscall
void syscallHandler();

void TLBManager();

//termina il processo corrente e tutta la sua progenie togliendola dalla ready_queue
void kill();

//elimina process e tutta la sua progenie da ready_queue, chiamata in kill()
void recursive_kill(pcb_t* process);

#endif
