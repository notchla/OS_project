#include "asl.h"
#include "const.h"
#include "utils.h"

static LIST_HEAD(semdFree);//lista dei semafori liberi
static LIST_HEAD(ASL);//lista dei semafori attivi

int semdevices[(1 + DEV_USED_INTS) * DEV_PER_INT];

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

//inizializza l' array dei semafori e inserisci i semafori nella lista dei semafori liberi
void initASL(){
    static semd_t semdFree_table[MAXPROC];
    for (int i = 0; i < MAXPROC; i++)
    {
        INIT_LIST_HEAD(&semdFree_table[i].s_procQ);
        list_add_tail(&semdFree_table[i].s_next, &semdFree);
    }
    
}

//ritorna il semaforo con chiave key
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

//se semdfree non e' libera ritorna il primo semaforo libero nella lista con tutti i suoi valori inizializati a zero
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
        if(semaphore->s_key > ptr->s_key){
            list_add(&semaphore->s_next, ptr->s_next.prev);
            return;
        }
    }
    list_add_tail(&semaphore->s_next,&ASL);//semaphore ha priorita minima
}

//inserisci il pcb puntato da p in coda alla coda dei processi bloccati dal semaforo con chiave pari a key
int insertBlocked(int* key, pcb_t* p){
    if(key == NULL || p == NULL)
        return 0;

    semd_t* ptr = getSemd(key);
    if (ptr!=NULL){
        if(ptr->s_procQ.next == NULL)
            INIT_LIST_HEAD(&ptr->s_procQ);
        p->p_semkey = key;
        if(p->p_next.next != NULL)//controllo su pcb per vedere se appartiene gia ad una lista.se pcb appartiene ad una lista chiamiamo list_del su p->next per non invalidare la lista a cui apparteneva
            list_del(&p->p_next);
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
        if(p->p_next.next->prev == &p->p_next && p->p_next.prev->next == &p->p_next){//controllo su pcb per vedere se appartiene gia ad una lista.se pcb appartiene ad una lista chiamiamo list_del su p->next per non invalidare la lista a cui apparteneva
            list_del(&p->p_next);
            INIT_LIST_HEAD(&p->p_next);
        }
        list_add_tail(&p->p_next,&semaphore->s_procQ);
        return 0;
    }
    else
        return 1;//semaphore non puo essere allocato
}

//rimuove il primo pcd bloccato dal semaforo con coda pari a key, se la coda diventa vuota il semaforo viene rimosso da asl e inserito in semdfree
pcb_t* removeBlocked(int* key){
    if(key == NULL)
        return NULL;

    semd_t* ptr = getSemd(key);

    if(ptr!=NULL){
        if(!list_empty(&ptr->s_procQ)){
            pcb_t* removed = container_of(ptr->s_procQ.next, pcb_t, p_next);
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

//rimuove il processo puntato da p dal suo semaforo, se la coda diventa vuota il semaforo viene rimmosso da asl e inserito in semdfree. se il semaforo indicato da semkey non contiene il processo ritorna null
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
//ritorna il puntatore al primo processo puntato dal semaforo con chiave pari a key
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

//rimozione di p e di tutti i processi radicati in p dai propri semafori
void outChildBlocked(pcb_t* p){
    if(p == NULL)
        return;
    pcb_t* child;
    list_for_each_entry(child,&p->p_child,p_sib){
        outChildBlocked(child);
    }
    outBlocked(p);
}