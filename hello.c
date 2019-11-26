#include "printprint.h"
#include "system.h"
#include "termread.h"

int main(void)
{
    char t;
    while((t = getchar()) != -1){
        print_puts(&t);    
    }
    if(t == -1)
        print_puts("error");
    

    /* Go to sleep indefinetely */
    while (1) 
        WAIT();
    return 0;
}

