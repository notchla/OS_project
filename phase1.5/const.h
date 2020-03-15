
//new processor state locations
#define SYSCALLNEWAREA    0x200003D4
#define PGRTRAPNEWAREA    0x200002BC
#define TBLMGMTNEWAREA    0x200001A4
#define IENEWAREA         0x2000008C

//state register useful values
/*  current values are pushed in the previous(active) whenever an
    interrput/exception is raised                                             */
#define ALLOFF            0x00000000 //ALL off, kernel mode
#define IEON              0x00000001 //current interrupt on
#define UMODE             0x00000002 //current user mode on
#define IEMASK            0x0000FF00 //interrupt mask on
#define VMON              0x01000000 //virtual memory once
#define TIMERON           0x08000000 //local timer once
