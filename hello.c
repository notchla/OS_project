#include "printprint.h"
#include "system.h"
#include "termread.h"

int main(void)
{
    char t;
    while((t = getchar()) != -1)
        print_puts(&t);
    print_puts("\n");

    /* Go to sleep indefinetely */
    while (1) 
        WAIT();
    return 0;
}

