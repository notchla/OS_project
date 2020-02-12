#include "asl.h"
#include "const.h"

static LIST_HEAD(semdFree);
static LIST_HEAD(ASL);//lista dei semafori attivi

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

//inserisce con costo O(n) il semaforo nella lista di priorita dei semafori attivi,la priorita e' dettata da s_key
void insertSem(semd_t* semaphore){
    if (semaphore == NULL)
        return;
    semd_t* ptr;
    list_for_each_entry(ptr, &ASL, s_next){
        if(*semaphore->s_key > *ptr->s_key){
            list_add(&semaphore->s_next, ptr->s_next.prev);
            return;
        }
    }
    list_add_tail(&semaphore->s_next,&ASL);//semaphore ha priorita minima
}

int insertBlocked(int* key, pcb_t* p){
    if(key == NULL || p == NULL)
        return 0;

    semd_t* ptr = getSemd(key);
    if (ptr!=NULL){
        if(ptr->s_procQ.next == NULL)
            INIT_LIST_HEAD(&ptr->s_procQ);
        p->p_semkey = key;
        list_add_tail(&p->p_next,&ptr->s_procQ);
        return 0;
    }
    //se un semaforo con s_key = key non e' presente in asl,ne allochiamo uno
    semd_t* semaphore = allocSemd();
    if(semaphore != NULL){
        semaphore->s_key = key;
        INIT_LIST_HEAD(&semaphore->s_procQ);
        insertSem(semaphore);
        p->p_semkey = key;
        list_add_tail(&p->p_next,&semaphore->s_procQ);
        return 0;
    }
    else
        return 1;//semaphore non puo essere allocato
}

pcb_t* removeBlocked(int* key){
    if(key == NULL)
        return NULL;

    semd_t* ptr = getSemd(key);

    if(ptr!=NULL){
        if(!list_empty(&ptr->s_procQ)){
            pcb_t* removed = container_of(ptr->s_procQ.next, pcb_t, p_next);
            list_del(ptr->s_procQ.next);
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

pcb_t* outBlocked(pcb_t* p){
    if(p == NULL || p->p_semkey == NULL)
        return NULL;
    semd_t* ptr = getSemd(p->p_semkey);
    if(ptr!=NULL){
        pcb_t* temp;
        list_for_each_entry(temp,&ptr->s_procQ,p_next){//cicla sui processi bloccati dal semaforo con s_key == p->semkey
            if(temp == p){
                list_del(&temp->p_next);//rimuove il pcb dalla coda dei processi bloccati
                if(list_empty(&ptr->s_procQ)){//se la coda e' vuota elimino il semaforo e lo restituisco a semfree
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

pcb_t* headBlocked(int* key){
    if(key == NULL)
        return NULL;
    semd_t* ptr = getSemd(key);
    if(ptr!=NULL){
        if(list_empty(&ptr->s_procQ))//se la lista dei processi bloccati da ptr e' vuota ritorna null
            return NULL;
        return container_of(ptr->s_procQ.next, pcb_t, p_next);//ritorna il primo pcb bloccato dal semaforo
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