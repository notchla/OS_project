#ifndef CONST_BIKAYA_INCLUDED
#define CONST_BIKAYA_INCLUDED

#ifdef TARGET_UMPS


/* Values for CP0 Cause.ExcCode */
#define EXC_INTERRUPT      0
#define EXC_TLBMOD         1
#define EXC_TLBINVLOAD     2
#define EXC_TLBINVSTORE    3
#define EXC_ADDRINVLOAD    4
#define EXC_ADDRINVSTORE   5
#define EXC_BUSINVFETCH    6
#define EXC_BUSINVLDSTORE  7
#define EXC_SYSCALL        8
#define EXC_BREAKPOINT     9
#define EXC_RESERVEDINSTR  10
#define EXC_COPROCUNUSABLE 11
#define EXC_ARITHOVERFLOW  12
#define EXC_BADPTE         13
#define EXC_PTEMISS        14

/* Interrupt lines used by the devices */
#define INT_T_SLICE  1 /* time slice interrupt */
#define INT_TIMER    2 /* timer interrupt */
#define INT_LOWEST   3 /* minimum interrupt number used by real devices */
#define INT_DISK     3
#define INT_TAPE     4
#define INT_UNUSED   5 /* network? */
#define INT_PRINTER  6
#define INT_TERMINAL 7

#define FRAMESIZE 4096


#endif

#ifdef TARGET_UARM
#include "uarm/uARMconst.h"
#endif


/* nucleus (phase2)-handled SYSCALL values */
#define GETCPUTIME       1
#define CREATEPROCESS    2
#define TERMINATEPROCESS 3
#define VERHOGEN         4
#define PASSEREN         5
#define WAITIO           6
#define SPECPASSUP       7
#define GETPID           8

#define DEFAULT_PRIORITY 1
#define TRUE             1
#define FALSE            0

#define TIME_SLICE 3000  //time slice in us

//syscall codes
#define TERMINATE_PROCESS         3

//interrupt lines
#define LOCALTIME_INT             0x00000002
#define INTERVALTIME_INT          0x00000004
#define DISK_INT                  0x00000008
#define TAPE_INT                  0x00000010
#define NETWORK_INT               0x00000020
#define PRINTER_INT               0x00000040
#define TERM_INT                  0x00000080

#define DEV_USED_INTS   5 /* Number of ints reserved for devices: 3,4,5,6,7 */
#define DEV_UNUSED_INTS 3 /* Number of devices without reserved interrupt registers */
#define DEV_PER_INT     8 /* Maximum number of devices per interrupt line */

//filter interrupt cause bits
#define CAUSE_MASK                0x000000FF

//archidecture specific custom macros
#ifdef TARGET_UARM
/* returns the complete cause vector */
#define CAUSE_ALL_GET(cause) ((cause & 0xFF000000) >> 24)
#endif

#define TERM_STATUS_MASK   0xFF

//device status
#define ST_READY           1
#define ST_BUSY            3
#define ST_TRANSMITTED     5
#define ST_RECEIVED        5
//device commands
#define CMD_ACK            1
#define CMD_TRANSMIT       2

#endif
