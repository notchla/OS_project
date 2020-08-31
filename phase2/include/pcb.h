/* functions to handle the pcb structures and the pcb list */

#ifndef PCB_H
#define PCB_H
#include <types_bikaya.h>

/* PCB handling functions */

/* PCB free list handling functions */
//initialize the static pcb array and add each block in the array to the free pcb list
void initPcbs(void);
//adds the pcb p to the free pcb list
void freePcb(pcb_t *p);
//removes a pcb from the free pcb list if possible, otherwise returns null
pcb_t *allocPcb(void);

/* PCB queue handling functions */
//initializes the list_head to a new list
void mkEmptyProcQ(struct list_head *head);
//returns true if the passed list is empty
int emptyProcQ(struct list_head *head);
//inserts the pcb p to the list pointed by head with cost O(n)
void insertProcQ(struct list_head *head, pcb_t *p);
//returns the first pcb in the list poined by head
pcb_t *headProcQ(struct list_head *head);
//removes the first pcb from the list pointed by head
pcb_t *removeProcQ(struct list_head *head);
//searches for the pcb p in the list pointed by head, removes it and returns p if p is in the list, null otherwise
pcb_t *outProcQ(struct list_head *head, pcb_t *p);


/* Tree view functions */
//returns true if p has no children
int emptyChild(pcb_t *this);
//inserts p in the end of the list of the children of prnt
void insertChild(pcb_t *prnt, pcb_t *p);
//removes and returns the first child of p
pcb_t *removeChild(pcb_t *p);
//removes p from the list of children of p->parent
pcb_t *outChild(pcb_t *p);

#endif
