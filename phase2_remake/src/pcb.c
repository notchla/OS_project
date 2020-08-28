#include "pcb.h"
#include "const_bikaya.h"

static LIST_HEAD(pcbFree);

//reference : https://stackoverflow.com/questions/18851835/create-my-own-memset-function-in-c
//casts the pointer to u_char and sets to null every byte
void *my_memset(void *struct_ptr, int c, int len){
    unsigned char* p = struct_ptr;
    while(len > 0){
        *p = 0;
        p++;
        len--;
    }
    return struct_ptr;
}

void initPcbs(){
    static pcb_t pcbFree_table[MAXPROC];
    for (int i = 0; i < MAXPROC; i++)
    {
        list_add_tail(&pcbFree_table[i].p_next, &pcbFree);
    }

}

void freePcb(pcb_t* p){
    list_add_tail(&p->p_next, &pcbFree);
}

pcb_t* allocPcb(){
    if(list_empty(&pcbFree))
        return NULL;
    else{
        struct pcb_t* process;
        process = container_of(pcbFree.next, pcb_t, p_next);
        list_del(pcbFree.next);
        INIT_LIST_HEAD(&process->p_next);
        my_memset(process, 0, sizeof(*process)); //sets every bit to 0
        INIT_LIST_HEAD(&process->p_child);// only used recursively in outchildblocked.() in asl
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
    list_add_tail(&p->p_next,head);//the pcb has minimum priority
}

pcb_t* headProcQ(struct list_head* head){
    if (!(head == NULL || list_empty(head)))
        return container_of(head->next, pcb_t, p_next);
    else
        return NULL;
}

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

int emptyChild(pcb_t* this){
    if(this->p_child.next == NULL)//INIT_LIST_HEAD not called on this->p_child
        return 1;
    else
        return list_empty(&this->p_child);
}

void insertChild(pcb_t* prnt, pcb_t* p){
    if(prnt != NULL && p!= NULL){
        if(prnt->p_child.next == NULL)
            INIT_LIST_HEAD(&prnt->p_child);//initializes the children list
        list_add_tail(&p->p_sib,&prnt->p_child);
        p->p_parent = prnt;
    }
}

pcb_t* removeChild(pcb_t* p){
    if(p!=NULL && p->p_child.next!=NULL){
        if(!list_empty(&p->p_child)){//p has children
            pcb_t* tmp;
            tmp = container_of(p->p_child.next,pcb_t,p_sib);//get the first child
            list_del(p->p_child.next);//remove the first child
            tmp->p_parent = NULL;
            INIT_LIST_HEAD(&tmp->p_sib); //reset the parameters of the child
            return tmp;
        }
        else
            return NULL;
    }
    else
        return NULL;
}

pcb_t* outChild(pcb_t* p){
    if(p!=NULL && p->p_parent!=NULL){
        list_del(&p->p_sib);
        INIT_LIST_HEAD(&p->p_sib);
        p->p_parent = NULL; //reset the parameters of the child
        return p;
    }
    else
        return NULL;
}
