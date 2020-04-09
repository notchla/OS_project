# Phase 1.5
This is an intermediate part of the project, halfway to the conclusion. The aim is to provide support for
later developement, by implementing the system init, the handling of interrupts and a simple round-robin scheduler.
A process termination syscall (code = 3) has also been implemented, which recoursively frees the process itself and the whole subtree.

We used the data structures implemented prevoiusly to handle the processes.

## Requirements and build
In order to compile scons>2.4 is needed, together with the libraries noted in the 'requirements.txt' file (can be installed with the pip command).
To build type "scons uarm" for uarm platform and "scons umps" for umps2 platform. The built kernel files will be created in the build/ directory
