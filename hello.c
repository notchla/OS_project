#include "printprint.h"
#include "system.h"

int main(void)
{
    print_puts("hello, world\n");

    /* Go to sleep indefinetely */
    while (1) 
        WAIT();
    return 0;
}

