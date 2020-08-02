#include "pcb.h"
#include "const.h"

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
        INIT_LIST_HEAD(&process->p_next);
        my_memset(process, 0, sizeof(*process)); //setta ogni campo del pcb a zero
        INIT_LIST_HEAD(&process->p_child);//serve per scrivere in maniera ricorsiva outchildblocked() nel modulo asl
        INIT_LIST_HEAD(&process->p_sib);
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

//ritorna 1 se la lista dei figli di p e' vuota
int emptyChild(pcb_t* this){
    if(this->p_child.next == NULL)//INIT_LIST_HEAD not called on this->p_child
        return 1;
    else
        return list_empty(&this->p_child);
}

//inserisce p in coda alla lista dei figli di prnt;
void insertChild(pcb_t* prnt, pcb_t* p){
    if(prnt != NULL && p!= NULL){
        if(prnt->p_child.next == NULL)
            INIT_LIST_HEAD(&prnt->p_child);//inizializza la coda dei figli se non e' gia stato fatto
        list_add_tail(&p->p_sib,&prnt->p_child);
        p->p_parent = prnt;
    }
}

//rimuove e ritorna il primo figlio di p;
pcb_t* removeChild(pcb_t* p){
    if(p!=NULL && p->p_child.next!=NULL){
        if(!list_empty(&p->p_child)){//se p ha figli
            pcb_t* tmp;
            tmp = container_of(p->p_child.next,pcb_t,p_sib);//ottieni il primo figlio
            list_del(p->p_child.next);//rimuovi il primo figlio
            tmp->p_parent = NULL;
            INIT_LIST_HEAD(&tmp->p_sib);//reset dei parametri del figlio che lo collegavano all'albero
            return tmp;
        }
        else
            return NULL;
    }
    else
        return NULL;
}
//precondizione: p->parent contiene p nella lista puntata da p_child
//rimuove p dalla lista dei figli di p->parent
pcb_t* outChild(pcb_t* p){
    if(p!=NULL && p->p_parent!=NULL){
        list_del(&p->p_sib);
        INIT_LIST_HEAD(&p->p_sib);
        p->p_parent = NULL;//reset dei parametri di p che lo collegavano all'albero
        return p;
    }
    else
        return NULL;
}
