
//new processor state locations
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
#define IEMASK            0x0000FF00 //interrupt mask on
// #define VMON              0x01000000 //virtual memory on
#define TIMERON           0x08000000 //local timer on
#define IEONOLD           0x00000010

/* Physical memory frame size */
#define FRAME_SIZE 4096
