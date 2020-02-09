#include "pcb.h"
#include "const.h"

//DEBUG PRINT
#ifdef TARGET_UMPS
#include "umps/libumps.h"
#include "umps/arch.h"
#endif
#ifdef TARGET_UARM
#include "uarm/libuarm.h"
#include "uarm/arch.h"
#endif
#define ST_READY       1
#define ST_BUSY        3
#define ST_TRANSMITTED 5

#define CMD_ACK      1
#define CMD_TRANSMIT 2

#define CHAR_OFFSET      8
#define TERM_STATUS_MASK 0xFF


/******************************************************************************
 * I/O Routines to write on a terminal
 ******************************************************************************/

/* This function returns the terminal transmitter status value given its address */
static unsigned int tx_status(termreg_t *tp) {
    return ((tp->transm_status) & TERM_STATUS_MASK);
}

/* This function prints a string on specified terminal and returns TRUE if
 * print was successful, FALSE if not   */
unsigned int termprint_debug(char *str, unsigned int term) {
    termreg_t *term_reg;

    unsigned int stat;
    unsigned int cmd;

    unsigned int error = FALSE;

    if (term < DEV_PER_INT) {
        term_reg = (termreg_t *)DEV_REG_ADDR(IL_TERMINAL, term);

        /* test device status */
        stat = tx_status(term_reg);
        if ((stat == ST_READY) || (stat == ST_TRANSMITTED)) {
            /* device is available */

            /* print cycle */
            while ((*str != '\0') && (!error)) {
                cmd                      = (*str << CHAR_OFFSET) | CMD_TRANSMIT;
                term_reg->transm_command = cmd;

                /* busy waiting */
                while ((stat = tx_status(term_reg)) == ST_BUSY)
                    ;

                /* end of wait */
                if (stat != ST_TRANSMITTED) {
                    error = TRUE;
                } else {
                    /* move to next char */
                    str++;
                }
            }
        } else {
            /* device is not available */
            error = TRUE;
        }
    } else {
        /* wrong terminal device number */
        error = TRUE;
    }

    return (!error);
}
//END DEBUG PRINT

static LIST_HEAD(pcbFree);

//reference : https://stackoverflow.com/questions/18851835/create-my-own-memset-function-in-c
//fa un cast del puntatore alla struttura ad u_char e setta a 0 ogni byte che compone la struttura
void *my_memset(void *struct_ptr, int c, int len){
    unsigned char* p = struct_ptr;
    while(len > 0){
        *p = 0;
        p++;
        len--;
    }
    return struct_ptr;
}

//inizializza il vettore statico dei pcb,e aggiunge ogni blocco nel vettore alla lista dei pcb liberi
void initPcbs(){
    static pcb_t pcbFree_table[MAXPROC];
    for (int i = 0; i < MAXPROC; i++)
    {
        list_add_tail(&pcbFree_table[i].p_next, &pcbFree);
    }
    
}

//aggiunge il pcb alla lista libera
void freePcb(pcb_t* p){
    list_add_tail(&p->p_next, &pcbFree);
}

//rimuove dalla lista libera un pcb se disponibile, altrimenti ritorna null
pcb_t* allocPcb(){
    if(list_empty(&pcbFree))
        return NULL;
    else{
        struct pcb_t* process;
        process = container_of(pcbFree.next, pcb_t, p_next);
        list_del(pcbFree.next);
        my_memset(process, 0, sizeof(*process)); //setta ogni campo del pcb a zero
        return process;
        
    }
}

void mkEmptyProcQ(struct list_head* head){

    INIT_LIST_HEAD(head);
}

int emptyProcQ(struct list_head* head){
    return list_empty(head);
}

//inserisce con consto O(n) il blocco nella lista di priorità puntata da head
void insertProcQ(struct list_head* head, pcb_t* p){
    if((head == NULL || p == NULL))
        return;
    pcb_t* ptr;
    list_for_each_entry(ptr, head, p_next){
        if(p->priority > ptr->priority){
            list_add(&p->p_next,ptr->p_next.prev);
            return;
        }
    }
    list_add_tail(&p->p_next,head);//il blocco ha priorità minima nella lista
}

//ritorna il blocco incima alla lista, considerando head come sentinella
pcb_t* headProcQ(struct list_head* head){
    if (!(head == NULL || list_empty(head)))
        return container_of(head->next, pcb_t, p_next);
    else
        return NULL;
}

//rimuove il primo blocco della lista puntata della sentinella head
pcb_t* removeProcQ(struct list_head* head){
    if(!(head==NULL || list_empty(head))){
        pcb_t* pcb;
        pcb = container_of(head->next, pcb_t, p_next);
        list_del(head->next);
        return pcb;
    }
    else
        return NULL;
}

//cerca p nella lista puntata da head, rimuove e ritorna p se p appartiene alla lista,null altrimenti
pcb_t* outProcQ(struct list_head* head, pcb_t* p){
    if(!(head == NULL || list_empty(head) || p == NULL)){
        pcb_t* tmp = NULL;
        list_for_each_entry(tmp, head, p_next){
            if(tmp == p){
                list_del(&p->p_next);
                return tmp;
            }
        }
        return NULL; //p not in list pointed by head
    }
    else
        return NULL;
}