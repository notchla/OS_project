# Phase 2
Implementation of the third level of the Kaya architecture: the kernel. Syscall support for 8 calls; interrupt, trap and TLB exception handlers.

## Requirements and build
In order to compile scons>2.4 is needed, together with the libraries noted in the 'requirements.txt' file (can be installed with the pip command).
To build type "scons uarm" for uarm platform and "scons umps" for umps2 platform. The built kernel files will be created in the build/ directory
