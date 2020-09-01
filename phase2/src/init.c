#include "pcb.h"
#include "asl.h"
#include "interrupts.h"
#include "exceptions.h"
#include "types_bikaya.h"
#include "system.h"   //include target libs
#include "scheduler.h"
#include "p2test_bikaya_v0.3.h"

//idle_proc simply evokes wait
void idle_proc(){
  WAIT();
}

//set up the state register in address to manage the syscall/interrupt/trap/tlb exception, using the passed handler function
void newArea(memaddr address, memaddr handler) {
  state_t* area;
  area = (state_t*) address;
  #if TARGET_UMPS
  area-> reg_sp = RAMTOP;
  area-> status = ALLOFF; // all interrupts deactivated, processor in kernel mode
  area-> pc_epc = handler;
  area-> reg_t9 = handler;
  #elif TARGET_UARM
  area-> sp = RAMTOP;
  area-> cpsr = STATUS_ALL_INT_DISABLE(STATUS_SYS_MODE); // all interrupts deactivated, processor in kernel mode
  area-> pc = handler;
  area-> ip = handler;
  area-> CP15_Control = CP15_CONTROL_NULL;  //VM disabled
  #endif
}

//create a new pcb and write the state with the passed function address and coprocessor status
void newProcess(memaddr functionAddr, int priority) {
  //create a pcb for the new process
  pcb_t* tempProcess = allocPcb();

  if(functionAddr == (memaddr) idle_proc)
  //save the idle_proc pcb, needed in scheduler for halt and deadlock detection
  idle_ptr = tempProcess;
  else {
    //not counting idle_proc in the active process count
    processCount++;
  }

  unsigned int addr = (RAMTOP - FRAME_SIZE * processCount);
  tempProcess-> priority = priority;
  tempProcess->original_priority = priority;

  #if TARGET_UMPS
  tempProcess-> p_s.reg_sp = addr;
  tempProcess-> p_s.status = ALLOFF | IEON | IEMASK;
  tempProcess-> p_s.pc_epc = functionAddr;
  tempProcess-> p_s.reg_t9 = functionAddr;
  #elif TARGET_UARM
  tempProcess-> p_s.sp = addr;
  tempProcess-> p_s.cpsr = STATUS_ALL_INT_ENABLE(STATUS_SYS_MODE); //all interrupts need to be on
  tempProcess-> p_s.pc = functionAddr;
  tempProcess-> p_s.ip = functionAddr;
  tempProcess-> p_s.CP15_Control = CP15_CONTROL_NULL;  //VM disabled
  #endif
  //schedule the newly created process
  schedInsertProc(tempProcess);
}

//sets up the processo states to load in case of the relative system exception
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

//initializes the data structures for processes and semaphores
void initData() {
  initPcbs();
  initASL();
}

//creates the starting processes (and idle_proc)
void initialProcess() {
  newProcess((memaddr) test, 1);
  newProcess((memaddr) idle_proc, -1); //least priority
}

//calls all the init functions, scheduler in the end
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
