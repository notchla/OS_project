#include "system.h"

#define ST_READY           1
#define ST_BUSY            3
#define ST_RECEIVED        5
#define ST_TRANSMITTED     5

#define CMD_ACK            1
#define CMD_TRANSMIT       2

#define CHAR_OFFSET        8
#define TERM_STATUS_MASK   0xFF

static termreg_t *term0_reg = (termreg_t *) DEV_REG_ADDR(IL_TERMINAL, 0);

/*static dtpreg_t *print0_reg = (dtpreg_t *) DEV_REG_ADDR(IL_PRINTER, 0);*/

static unsigned int tx_status(termreg_t *tp)
{
    return ((tp->recv_status) & TERM_STATUS_MASK);
}

char getchar()
{

    unsigned int stat;

    char c;

    stat = tx_status(term0_reg);
    if (stat != ST_READY && stat != ST_RECEIVED)
        return stat;

    term0_reg->recv_command = ST_RECEIVED;

    while ((stat = tx_status(term0_reg)) == ST_BUSY)
        ;

    c = 'c';

    term0_reg->recv_command = CMD_ACK;
    
    if (stat != ST_RECEIVED)
        return stat;
    else if(c == '\n') 
        return stat;
    else
        return c;
}
