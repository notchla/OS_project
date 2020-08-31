#include "asl.h"
#include "const_bikaya.h"
#include "utils.h"

static LIST_HEAD(semdFree);//lista dei semafori liberi
static LIST_HEAD(ASL);//lista dei semafori attivi

int semdevices[(1 + DEV_USED_INTS) * DEV_PER_INT];
semdev semDev;

//reference : https://stackoverflow.com/questions/18851835/create-my-own-memset-function-in-c
//fa un cast del puntatore alla struttura ad u_char e setta a 0 ogni byte che compone la struttura
void *my_memset_sem(void *struct_ptr, int c, int len){
    unsigned char* p = struct_ptr;
    while(len > 0){
        *p = 0;
        p++;
        len--;
    }
    return struct_ptr;
}

void initASL(){
    static semd_t semdFree_table[MAXPROC];
    for (int i = 0; i < MAXPROC; i++)
    {
        INIT_LIST_HEAD(&semdFree_table[i].s_procQ);
        list_add_tail(&semdFree_table[i].s_next, &semdFree);
    }
    for(int dev=0; dev < DEV_PER_INT; dev++) {
      semDev.disk[dev].s_key = &semdevices[(DISK_LINE - LOWEST_LINE) * DEV_PER_INT + dev];
      semDev.tape[dev].s_key = &semdevices[(TAPE_LINE - LOWEST_LINE) * DEV_PER_INT + dev];
      semDev.network[dev].s_key = &semdevices[(NETWORK_LINE - LOWEST_LINE) * DEV_PER_INT + dev];
      semDev.printer[dev].s_key = &semdevices[(PRINTER_LINE - LOWEST_LINE) * DEV_PER_INT + dev];
      semDev.terminalR[dev].s_key = &semdevices[(TERMINAL_LINE - LOWEST_LINE) * DEV_PER_INT + dev];
      semDev.terminalT[dev].s_key = &semdevices[(TERMINAL_LINE - LOWEST_LINE + 1) * DEV_PER_INT + dev];
    }
}

semd_t* getSemd(int* key){
    if(key == NULL)
        return NULL;
    semd_t* ptr;
    list_for_each_entry(ptr,&ASL,s_next){
        if(ptr->s_key == key)
            return ptr;
    }
    return NULL;
}

//if semdfree isn't empty return the first free semaphore in the list with all his values initialized to zero
semd_t* allocSemd(){
    if(list_empty(&semdFree))
        return NULL;
    else{
        semd_t* semaphore;
        semaphore = container_of(semdFree.next, semd_t, s_next);
        list_del(semdFree.next);
        my_memset_sem(semaphore, 0, sizeof(*semaphore));
        return semaphore;

    }
}

void insertSem(semd_t* semaphore){
    if (semaphore == NULL)
        return;
    semd_t* ptr;
    list_for_each_entry(ptr, &ASL, s_next){
        if(semaphore->s_key > ptr->s_key){
            list_add(&semaphore->s_next, ptr->s_next.prev);
            return;
        }
    }
    list_add_tail(&semaphore->s_next,&ASL);//semaphore has least priority
}

int insertBlocked(int* key, pcb_t* p){
    if(key == NULL || p == NULL)
        return 0;

    semd_t* ptr = getSemd(key);
    if (ptr!=NULL){
        if(ptr->s_procQ.next == NULL)
            INIT_LIST_HEAD(&ptr->s_procQ);
        p->p_semkey = key;
        //check if p_next is already used to connect p in a list, if this is true we need to call list_del on p_next before we connect it to the other list, to not invalidate the original list
        if(p->p_next.next->prev == &p->p_next && p->p_next.prev->next == &p->p_next){
            list_del(&p->p_next);
            INIT_LIST_HEAD(&p->p_next);
        }
        list_add_tail(&p->p_next,&ptr->s_procQ);
        return 0;
    }
    //if a semaphore with s_key == key isn't present in ASL, we allocat one
    semd_t* semaphore = allocSemd();
    if(semaphore != NULL){
        semaphore->s_key = key;
        INIT_LIST_HEAD(&semaphore->s_procQ);
        insertSem(semaphore);
        p->p_semkey = key;
        //check if p_next is already used to connect p in a list, if this is true we need to call list_del on p_next before we connect it to the other list, to not invalidate the original list
        if(p->p_next.next->prev == &p->p_next && p->p_next.prev->next == &p->p_next){
            list_del(&p->p_next);
            INIT_LIST_HEAD(&p->p_next);
        }
        list_add_tail(&p->p_next,&semaphore->s_procQ);
        return 0;
    }
    else
        return 1;//semaphore can't be allocated
}

pcb_t* removeBlocked(int* key){
    if(key == NULL)
        return NULL;

    semd_t* ptr = getSemd(key);

    if(ptr!=NULL){
        if(!list_empty(&ptr->s_procQ)){
            pcb_t* removed = container_of(ptr->s_procQ.next, pcb_t, p_next);
            removed->p_semkey = NULL;
            list_del(ptr->s_procQ.next);
            INIT_LIST_HEAD(&removed->p_next);
            if(list_empty(&ptr->s_procQ)){
                list_del(&ptr->s_next);

                ptr->s_key = NULL;
                INIT_LIST_HEAD(&ptr->s_next);
                INIT_LIST_HEAD(&ptr->s_procQ);
                list_add_tail(&ptr->s_next,&semdFree);
            }
            return removed;
        }
        else
            return NULL;

    }
    return NULL;
}

pcb_t* removeBlockedonDevice(int* key){
    if(key == NULL)
        return NULL;
    semd_t* ptr = getSemd(key);
    if(ptr!=NULL){
        if(!list_empty(&ptr->s_procQ)){
            pcb_t* removed = container_of(ptr->s_procQ.next, pcb_t, p_next);
            removed->p_semkey = NULL;
            list_del(ptr->s_procQ.next);
            INIT_LIST_HEAD(&removed->p_next);
            if(list_empty(&ptr->s_procQ)){
                list_del(&ptr->s_next);
                INIT_LIST_HEAD(&ptr->s_next);
                INIT_LIST_HEAD(&ptr->s_procQ);
            }
            return removed;
        }
        else
            return NULL;

    }
    return NULL;
}

pcb_t* DevicesOutBlocked(pcb_t* p){
    if(p == NULL || p->p_semkey == NULL)
        return NULL;
    semd_t* ptr = getSemd(p->p_semkey);
    if(ptr!=NULL){
        pcb_t* temp;
        list_for_each_entry(temp,&ptr->s_procQ,p_next){
            if(temp == p){
                list_del(&temp->p_next);
                p->p_semkey = NULL;
                if(list_empty(&ptr->s_procQ)){
                    list_del(&ptr->s_next);
                    INIT_LIST_HEAD(&ptr->s_next);
                    INIT_LIST_HEAD(&ptr->s_procQ);
                }
                return p;
            }
        }
        return NULL;
    }
    return NULL;
}

pcb_t* outBlocked(pcb_t* p){
    if(p == NULL || p->p_semkey == NULL)
        return NULL;
    semd_t* ptr = getSemd(p->p_semkey);
    if(ptr!=NULL){
        pcb_t* temp;
        list_for_each_entry(temp,&ptr->s_procQ,p_next){
            if(temp == p){
                list_del(&temp->p_next);
                p->p_semkey = NULL;
                if(list_empty(&ptr->s_procQ)){
                    list_del(&ptr->s_next);
                    ptr->s_key = NULL;
                    INIT_LIST_HEAD(&ptr->s_next);
                    INIT_LIST_HEAD(&ptr->s_procQ);
                    list_add_tail(&ptr->s_next,&semdFree);
                }
                return p;
            }
        }
        return NULL;
    }
    return NULL;
}

unsigned int isBlocked(pcb_t* p){
    if(p == NULL || p->p_semkey == NULL)
        return FALSE;
    semd_t* ptr = getSemd(p->p_semkey);
    if(ptr!=NULL){
        pcb_t* temp;
        list_for_each_entry(temp,&ptr->s_procQ,p_next){
            if(temp == p){
                return TRUE;
            }
        }
        return FALSE;
    }
    return FALSE;
}

pcb_t* headBlocked(int* key){
    if(key == NULL)
        return NULL;
    semd_t* ptr = getSemd(key);
    if(ptr!=NULL){
        if(list_empty(&ptr->s_procQ))
            return NULL;
        return container_of(ptr->s_procQ.next, pcb_t, p_next);
    }
    return NULL;
}

void outChildBlocked(pcb_t* p){
    if(p == NULL)
        return;
    pcb_t* child;
    list_for_each_entry(child,&p->p_child,p_sib){
        outChildBlocked(child);
    }
    outBlocked(p);
}
