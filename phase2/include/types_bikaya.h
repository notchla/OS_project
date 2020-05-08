#ifndef TYPES_BIKAYA_H_INCLUDED
#define TYPES_BIKAYA_H_INCLUDED

#include "system.h"

#include "listx.h"

typedef unsigned int memaddr;
typedef unsigned int cpu_time;
/* Process Control Block (PCB) data structure */
typedef struct pcb_t {
    /*process queue fields */
    struct list_head p_next;

    /*process tree fields */
    struct pcb_t *   p_parent;
    struct list_head p_child, p_sib;

    /* processor state, etc */
    state_t p_s;

    /* process priority */
    int priority;
    int original_priority;

    /* key of the semaphore on which the process is eventually blocked */
    int *p_semkey;

    cpu_time user_timer;        //time elapsed in user mode
    cpu_time kernel_timer;      //time elapsed in kernel mode
    cpu_time first_activation;  //TOD of first process load
    cpu_time last_restart;      //TOD of last load

    state_t* sysNew;
    state_t* sysOld;
    state_t* TlbNew;
    state_t* TlbOld;
    state_t* pgtNew;
    state_t* pgtOld;


} pcb_t;



/* Semaphore Descriptor (SEMD) data structure */
typedef struct semd_t {
    struct list_head s_next;

    // Semaphore key
    int *s_key;

    // Queue of PCBs blocked on the semaphore
    struct list_head s_procQ;
} semd_t;

typedef struct semdev {
    semd_t disk[DEV_PER_INT];
    semd_t tape[DEV_PER_INT];
    semd_t network[DEV_PER_INT];
    semd_t printer[DEV_PER_INT];
    semd_t terminalR[DEV_PER_INT];
    semd_t terminalT[DEV_PER_INT];
} semdev;

#endif
