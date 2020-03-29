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
#define LOCALTIME_INT             2
#define INTERVALTIME_INT          4
#define DISK_INT                  8
#define TAPE_INT                  16
#define NETWORK_INT               32
#define PRINTER_INT               64
#define TERM_INT                  128

//filter interrupt cause bits
#define CAUSE_MASK                0x000000FF
