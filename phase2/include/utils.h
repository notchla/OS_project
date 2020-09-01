/* generic useful helper functionts */

#ifndef UTILS_H
#define UTILS_H

#include "system.h"
#include "scheduler.h"
#include "asl.h"
#include "types_bikaya.h"

//copies the source memory area into dest
void mymemcpy(void *dest, void *src, int n);

/* track kernel/user time in a tidy way.
*  inputs:
*   _ call: a pointer to the requested system call (null if updating from interrupt);
*   _ callerState: state vector of the caller process;
*   _ start_time: TOD saved at the beginning of the critical section;
*   _ currentProcess: pcb of the calling process.
*  outputs:
*   _ call_sched: forwards the bool returned by the calls.
*/
int critical_wrapper(int (*call)(), state_t* callerState, cpu_time start_time, pcb_t* currentProcess);

/* helper to the critical_wrapper; updates the user time of the process
  inputs:
  _ currentProcess: pcb of the calling process
  outputs:
  _ start_time: the TOD when the function was called, to be passed to the critical_wrapper
*/
cpu_time update_user_time(pcb_t* currentProcess);

//set the return register of caller to status
void set_return(state_t* caller, int status);

/* set the command registerin the passed device register to command.
 * subdevices indicates wether the terminal is recieving or transmitting
 */
void set_command(devreg_t* reg, unsigned int command, int subdevice);
//returns the line and device of a given device register
void get_line_dev(devreg_t* reg, int* line, int* dev);
//gets the status (transm/recv) for a generic device (terminal)
unsigned int get_status(devreg_t* reg, int subdevice);
unsigned int tx_status(termreg_t *tp);
unsigned int rx_status(termreg_t *tp);

//debug call with key/value pairing
void debug(int, int);
//returns the semd structure of the respective device semaphore
semd_t * getSemDev(int line, int devnum, int read);
//Vs the relative device semaphore that blocks process
void verhogenKill(pcb_t* process);
//performs a V on the respective device semaphore
void verhogenDevice(int line, unsigned int status, int deviceNumber, int read);
//sends an ACK to the commandRegister device register
void ACKDevice(unsigned int* commandRegister);
//returns true if the pcb pid is in the readyQueue
int pid_in_readyQ(pcb_t* pid);
//set the cpu register reg if != null
int set_register(unsigned int reg, unsigned int val);
//custom recursive implementation of the base 2 logarithm
int log2(unsigned int n);
//keeps set only the lowest set bit
int lowest_set(int number);
//returns the interrupting device number at line
int getDeviceNumber(int line);



#endif
