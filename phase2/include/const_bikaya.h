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

//processor state locations
#define SYS_NEWAREA         0x200003D4
#define SYS_OLDAREA         0x20000348
#define TRAP_NEWAREA        0x200002BC
#define TRAP_OLDAREA        0x20000230
#define TLB_NEWAREA         0x200001A4
#define TLB_OLDAREA         0x20000118
#define IE_NEWAREA          0x2000008C
#define IE_OLDAREA          0x20000000

//state register useful values (umps)
/*  current values are pushed in the previous(active) whenever an
    interrput/exception is raised                                             */
#define ALLOFF            0x00000000 //ALL off, kernel mode
#define IEON              0x00000004 //turn on interrupts
#define IECON             0x00000001 //current interrupt on
#define UMODE             0x00000002 //current user mode on
#define IEMASK            0x00000400 //interrupt mask on
// #define VMON              0x01000000 //virtual memory on
#define TIMERON           0x08000000 //local timer on
#define IEONOLD           0x00000010

#define FRAMESIZE 4096

/* interrupt device map and register location (from pops 7.3, complete description figure) */
#define BITMAPSTART   0x1000003C
#define DEVREGSTART   0x10000050

#endif
#ifdef TARGET_UARM
#include "uarm/uARMconst.h"
/* get the whole cause register */
#define CAUSE_ALL_GET(cause) ((cause & 0xFF000000) >> 24)

#define BITMAPSTART   0x10006FE0
#define DEVREGSTART   0x100002DC

#endif

/* architecture generic consts */

/* device register size in bytes */
#define DEVREGSIZE     16
//bytes in a word
#define WORDLEN   4
/* Number of ints reserved for devices: 3,4,5,6,7 */
#define DEV_USED_INTS 5
/* number of interrupts without devices */
#define INTERNAL_INTS (INT_TERMINAL - DEV_USED_INTS + 1)
/* Maximum number of devices per interrupt line */
#define DEV_PER_INT   8

#define TIME_SLICE 3000  //time slice in us

/* Interrupt lines used by the devices */
#define INT_T_SLICE  1 /* time slice interrupt */
#define INT_TIMER    2 /* timer interrupt */
#define INT_LOWEST   3 /* minimum interrupt number used by real devices */
#define INT_DISK     3
#define INT_TAPE     4
#define INT_UNUSED   5 /* network? */
#define INT_PRINTER  6
#define INT_TERMINAL 7

//bit representing interrupt line
#define INT_CPU_BIT                   0x00000001
#define INT_T_SLICE_BIT               0x00000002
#define INT_TIMER_BIT                 0x00000004
#define INT_DISK_BIT                  0x00000008
#define INT_TAPE_BIT                  0x00000010
#define INT_UNUSED_BIT                0x00000020
#define INT_PRINTER_BIT               0x00000040
#define INT_TERMINAL_BIT              0x00000080

//filter interrupt cause bits
#define CAUSE_MASK                0x000000FF

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

/* Maxi number of overall (eg, system, daemons, user) concurrent processes */
#define MAXPROC 20
/* number of usermode processes (not including master proc and system daemons */
#define UPROCMAX 3

#define	HIDDEN static
#define	TRUE 	1
#define	FALSE	0
#define ON 	1
#define OFF 	0
#define EOS '\0'

#define CMD_ACK            1
#define CMD_TRANSMIT       2

#define CR 0x0a   /* carriage return as returned by the terminal */

#endif
