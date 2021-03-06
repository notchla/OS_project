#include "pcb.h"
#include "asl.h"
#include "interrupts.h"
#include "exceptions.h"
#include "types_bikaya.h"
#include "p1.5test_bikaya_v0.c"
#include "system.h"   //include target libs
#include "scheduler.h"

extern void test1();
extern void test2();
extern void test3();

#if TARGET_UMPS
void newArea(unsigned int address, unsigned int handler) {
  state_t* area;
  area = (state_t*) address;
  area-> reg_sp = RAMTOP;
  area-> status = ALLOFF;
  area-> pc_epc = handler;
  area-> reg_t9 = handler;
}
#elif TARGET_UARM
void newArea(unsigned int address, unsigned int handler) {
  state_t* area;
  area = (state_t*) address;
  area-> sp = RAMTOP;
  area-> cpsr = STATUS_ALL_INT_DISABLE(STATUS_SYS_MODE);
  area-> pc = handler;
  area-> ip = handler;
  area-> CP15_Control = CP15_CONTROL_NULL;  //VM disabled
}
#endif

void newProcess(memaddr functionAddr) {
  pcb_t* tempProcess = allocPcb();
  incProcCount();
  unsigned int addr = (RAMTOP - FRAME_SIZE*getProcCount());
  tempProcess-> priority = getProcCount();
  tempProcess->original_priority = getProcCount();

  #if TARGET_UMPS
  tempProcess-> p_s.reg_sp = addr;
  tempProcess-> p_s.status = ALLOFF | IEON | IEMASK;
  tempProcess-> p_s.pc_epc = functionAddr;
  tempProcess-> p_s.reg_t9 = functionAddr;
  #elif TARGET_UARM
  tempProcess-> p_s.sp = addr;
  tempProcess-> p_s.cpsr = STATUS_ENABLE_TIMER(STATUS_ALL_INT_DISABLE(STATUS_SYS_MODE)); //timer interrupts need to be on
  tempProcess-> p_s.pc = functionAddr;
  tempProcess-> p_s.ip = functionAddr;
  tempProcess-> p_s.CP15_Control = CP15_CONTROL_NULL;  //VM disabled
  #endif
  //schedule the newly created process
  schedInsertProc(tempProcess);
}

void initROM() {
  #if TARGET_UMPS
  //syscall
  newArea(SYS_NEWAREA, (memaddr) syscallHandler);
  //traps
  newArea(TRAP_NEWAREA, (memaddr) trapHandler);
  //TLB
  newArea(TLB_NEWAREA, (memaddr) TLBManager);
  //interrupts
  newArea(IE_NEWAREA, (memaddr) interruptHandler);

  #elif TARGET_UARM
  //syscall
  newArea(SYSBK_NEWAREA, (memaddr) syscallHandler);
  //traps
  newArea(PGMTRAP_NEWAREA, (memaddr) trapHandler);
  //TLB
  newArea(TLB_NEWAREA, (memaddr) TLBManager);
  //interrupts
  newArea(INT_NEWAREA, (memaddr) interruptHandler);
  #endif
}

void initData() {
  initPcbs();
  // initASL();
}

void initialProcess() {
  newProcess((memaddr) test1);
  newProcess((memaddr) test2);
  newProcess((memaddr) test3);
}

int main() {
  //initialize data structures for process handling
  initData();
  //setup NEWAREAs in menory
  initROM();
  //load the init processes
  initialProcess();
  //call scheduler to switch the processes
  scheduler();

  return 0;
}
