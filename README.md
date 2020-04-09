# BIKAYA - OS Project 2019/2020 @ UniBo
BiKaya is an educational operative system based upon a 6 layer abstraction architecture, initially proposed by Dijkstra. This work is part of the operative system couse held at Alma Mater Studiorum, Bologna.

## The project
The final purpose of the project is to present a simple yet fully working kernel, which should be designed to be executed on two different architectures: MIPS and ARM.
The work is split in 3 phases:
- Phase 0: in order to learn some know-how of the emulators used, a simple program that writes the input of a terminal to a file (using a printer) has been written.
- Phase 1: realization of a "queue management" system, which provides support for multiple data structures, which will be used in phase 2. This takes place in the second layer in the BiKaya stack.
- Phase 1.5: using the interface to data structures implemented in the previous level, write the scheduler, interrupt handler and part of the syscall handler.
- Phase 2: Complete the previous phase with full syscall support, TLB and trap managers. This takes place in the third layer in the BiKaya stack.


## Requirements
In order to ease the developement and testing, those system will be virtualized using dedicated emulators:
- uMPS, for the MPIS architecture: https://github.com/tjonjic/umps
- uARM, for the ARM architecture: https://github.com/mellotanica/uARM

## Contributors:
- Lorenzo Liso
- Gianluca Galletti
- Raffaello Balica
