#include "printprint.h"
#include "system.h"
#include "termread.h"

int main(void)
{
    char *str;
    str = get_line();
    print_puts(str);
    /* Go to sleep indefinetely */
    while (1) 
        WAIT();
    return 0;
}

