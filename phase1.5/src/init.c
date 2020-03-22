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
void NEWAREA(unsigned int address, unsigned int handler) {
  state_t* area;
  area = (state_t*) address;
  area-> reg_sp = RAMTOP;
  area-> status = ALLOFF | IEMASK;
  area-> pc_epc = handler;
  area-> reg_t9 = handler;
}
#elif TARGET_UARM
void NEWAREA(unsigned int address, unsigned int handler) {
  state_t* area;
  area = (state_t*) address;
  area-> sp = RAMTOP;
  area-> cpsr = CP15_DISABLE_VM(STATUS_ALL_INT_DISABLE(STATUS_SYS_MODE));
  area-> pc = handler;
  area-> ip = handler;
}
#endif

void NEWPROCESS(memaddr functionAddr) {
  pcb_t* tempProcess = allocPcb();
  incProcCount();
  unsigned int addr = (RAMTOP - FRAME_SIZE*getProcCount());
  tempProcess-> priority = getProcCount();
  #if TARGET_UMPS
  tempProcess-> p_s.reg_sp = addr;
  tempProcess-> p_s.status = ALLOFF | TIMERON | IEON;
  tempProcess-> p_s.pc_epc = functionAddr;
  tempProcess-> p_s.reg_t9 = functionAddr;
  #elif TARGET_UARM
  tempProcess-> p_s.sp = addr;
  tempProcess-> p_s.cpsr = CP15_DISABLE_VM(STATUS_ENABLE_TIMER(STATUS_ENABLE_INT(STATUS_SYS_MODE)));
  tempProcess-> p_s.pc = functionAddr;
  tempProcess-> p_s.ip = functionAddr;
  #endif
  schedInsertProc(tempProcess); //schedule the newly created process
}

void initROM() {
  #if TARGET_UMPS
  //syscall
  NEWAREA(SYS_NEWAREA, (memaddr) syscallHandler);
  //traps
  NEWAREA(TRAP_NEWAREA, (memaddr) trapHandler);
  //TLB
  NEWAREA(TBL_NEWAREA, (memaddr) TLBManager);
  //interrupts
  NEWAREA(IE_NEWAREA, (memaddr) interruptHandler);

  #elif TARGET_UARM
  //syscall
  NEWAREA(SYSBK_NEWAREA, (memaddr) syscallHandler);
  //traps
  NEWAREA(PGMTRAP_NEWAREA, (memaddr) trapHandler);
  //TLB
  NEWAREA(TLB_NEWAREA, (memaddr) TLBManager);
  //interrupts
  NEWAREA(INT_NEWAREA, (memaddr) interruptHandler);
  #endif
}

void initData() {
  initPcbs();
  //initASL();
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

  scheduler();

  return 0;
}
