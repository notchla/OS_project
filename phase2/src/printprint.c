#include "system.h"

#define ST_READY           1
#define ST_BUSY            3

#define CMD_ACK            1
#define CMD_PRINTCHR       2

#define PRINTER_STATUS_MASK   0xFF

static dtpreg_t *print0_reg = (dtpreg_t *) DEV_REG_ADDR(IL_PRINTER, 0);

static unsigned int txStatus(dtpreg_t *dp)
{
    return ((dp->status) & PRINTER_STATUS_MASK);
}

int print_putchar(char c)
{
    unsigned int stat;

    stat = txStatus(print0_reg);
    if (stat != ST_READY)
        return -1;

    print0_reg->data0 = c;
    print0_reg->command = CMD_PRINTCHR;

    while ((stat = txStatus(print0_reg)) == ST_BUSY)
        ;

    print0_reg->command = CMD_ACK;

    if (stat != 1)
        return -1;
    else
        return 0;
}


void print_puts(const char *str)
{
    while (*str)
        if (print_putchar(*str++))
            return;
}
