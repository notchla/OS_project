#include "system.h"

#define ST_READY           1
#define ST_BUSY            3
#define ST_RECEIVED        5
#define ST_TRANSMITTED     5

#define CMD_ACK            1
#define CMD_RECEIVE        2

#define CHAR_OFFSET        8
#define TERM_STATUS_MASK   0xFF

#define BUFF_SIZE 256

static termreg_t *term0_reg = (termreg_t *) DEV_REG_ADDR(IL_TERMINAL, 0);
static char TERM_BUFF[BUFF_SIZE + 1];
/*static dtpreg_t *print0_reg = (dtpreg_t *) DEV_REG_ADDR(IL_PRINTER, 0);*/

static unsigned int rx_status(termreg_t *tp)
{
    return ((tp->recv_status) & TERM_STATUS_MASK);
}

char getchar()
{

    unsigned int stat;

    char c;

    stat = rx_status(term0_reg);
    if (stat != ST_READY && stat != ST_RECEIVED)
        return -1;

    term0_reg->recv_command = CMD_RECEIVE;

    while ((stat = rx_status(term0_reg)) == ST_BUSY)
        ;

    c = term0_reg->recv_status >> CHAR_OFFSET;

    term0_reg->recv_command = CMD_ACK;
    
    if (stat != ST_RECEIVED)
        return -1;
    else
        return c;
}

char* get_line(){
    int i = 0;
    char t;
    while(i<BUFF_SIZE){
        t = getchar();
        if(t == '\n' || t == -1) {
            break;
        }
        TERM_BUFF[i] = t;
        i++;
    }
    if(t == '\n'){
        TERM_BUFF[i] = '\n';
        TERM_BUFF[i+1] = '\0';
    } else if(t == -1) {
        TERM_BUFF[i] = '\0';
    }
    else
        TERM_BUFF[BUFF_SIZE] = '\0';
    return TERM_BUFF;
    
}