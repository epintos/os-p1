Operating Systems First Project
================================

First Project for Operating Systems course at ITBA. Year 2011.

Makefile commands:

- make clear (erase .o files, exectuables and fifo/socket in case an error ocurred)
- make IPC=shmem/msgqueue/fifo/socket (compiles for the chosen IPC)

The executable generated is simulacion

For executing:

./simulation followed by the map file and companies chosen

Example:

./simulation ciudades3.txt empresaB.txt empresaB.txt empresaB.txt
