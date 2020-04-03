#define TIME_SLICE 3000  //time slice in us

//syscall codes
#define CREATE_PROCESS            1
#define TERMINATE_PROCESS         3
#define VERHOGEN                  2 //temporary
#define PASSEREN                  4
#define EXCEPTION_STATE_VECTOR    5
#define GET_CPU_TIME              6
#define WAIT_CLOCK                7
#define WAIT_IO_DEVICE            8

//interrupt lines
#define LOCALTIME_INT             0x00000002
#define INTERVALTIME_INT          0x00000004
#define DISK_INT                  0x00000008
#define TAPE_INT                  0x00000010
#define NETWORK_INT               0x00000020
#define PRINTER_INT               0x00000040
#define TERM_INT                  0x00000080

//filter interrupt cause bits
#define CAUSE_MASK                0x000000FF

//archidecture specific custom macros
#ifdef TARGET_UARM
/* returns the complete cause vector */
#define CAUSE_ALL_GET(cause) ((cause & 0xFF000000) >> 24)
#endif
