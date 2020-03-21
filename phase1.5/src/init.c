#include "pcb.h"
#include "asl.h"
#include "interrupts.h"
#include "exceptions.h"
#include "types_bikaya.h"
#include "p1.5test_bikaya_v0.c"
#include "system.h"   //include target libs

struct list_head readyQueue;
pcb_t* currentProcess;
int processCount = 0;

extern void test1();
extern void test2();
extern void test3();

#if TARGET_UMPS
void NEWAREA(state_t* area, unsigned int address, unsigned int handler) {
  area = (state_t*) address;
  area-> reg_sp = RAMTOP;
  area-> status = ALLOFF | IEMASK;
  area-> pc_epc = handler;
  area-> reg_t9 = handler;
}
#elif TARGET_UARM
void NEWAREA(state_t* area, unsigned int address, unsigned int handler) {
  area = (state_t*) address;
  area-> sp = RAMTOP;
  area-> cpsr = CP15_DISABLE_VM(STATUS_ALL_INT_DISABLE(STATUS_SYS_MODE));
  area-> pc = handler;
  area-> ip = handler;
}
#endif

void NEWPROCESS(memaddr functionAddr) {
  currentProcess = allocPcb();
  ++processCount;
  unsigned int addr = (RAMTOP - FRAME_SIZE*processCount);
  currentProcess-> priority = processCount;
  #if TARGET_UMPS
  currentProcess-> p_s.reg_sp = addr;
  currentProcess-> p_s.status = ALLOFF | TIMERON | IEON;
  currentProcess-> p_s.pc_epc = functionAddr;
  currentProcess-> p_s.reg_t9 = functionAddr;
  #elif TARGET_UARM
  currentProcess-> p_s.sp = addr;
  currentProcess-> p_s.cpsr = CP15_DISABLE_VM(STATUS_ENABLE_TIMER(STATUS_ENABLE_INT(STATUS_SYS_MODE)));
  currentProcess-> p_s.pc = functionAddr;
  currentProcess-> p_s.ip = functionAddr;
  #endif
  insertProcQ(&readyQueue, currentProcess);
}

void initROM() {
  #if TARGET_UMPS

  state_t* newArea;
  //syscall
  NEWAREA(newArea, SYS_NEWAREA, (memaddr) syscallHandler);
  //traps
  NEWAREA(newArea, TRAP_NEWAREA, (memaddr) trapHandler);
  //TLB
  NEWAREA(newArea, TBL_NEWAREA, (memaddr) TLBManager);
  //interrupts
  NEWAREA(newArea, IE_NEWAREA, (memaddr) interruptHandler);

  #elif TARGET_UARM
  state_t* newArea;
  //syscall
  NEWAREA(newArea, SYSBK_NEWAREA, (memaddr) syscallHandler);
  //traps
  NEWAREA(newArea, PGMTRAP_NEWAREA, (memaddr) trapHandler);
  //TLB
  NEWAREA(newArea, TLB_NEWAREA, (memaddr) TLBManager);
  //interrupts
  NEWAREA(newArea, INT_NEWAREA, (memaddr) interruptHandler);
  #endif
}

void initData() {
  initPcbs();
  //initASL();
  mkEmptyProcQ(&readyQueue);

}

void initialProcess() {
  NEWPROCESS((memaddr) test1);
  NEWPROCESS((memaddr) test2);
  NEWPROCESS((memaddr) test3);
}

int main() {
  //initialize data structures for process handling
  initData();
  //setup newareas in menory
  initROM();
  //load the init processes
  initialProcess();

  //scheduler();

  return 0;
}
