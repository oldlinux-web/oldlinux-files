INFORMATION on mdb version 2.6.0 (Sept 9/96).
   
MDB is the MINIX debugger which allows you to place breakpoints and
control the execution of a program. It has a lot of the features that
you would expect in a program debugger; for example, stack traces and
single step execution.

The current version works with MINIX for PC and was developed and tested
under MINIX 1.7.x (32 bit version). It should work with 16 bit MINIX. 

How to Install

1) Place MDB.TAZ on a floppy with FDVOL
2) Boot Minix, login as bin and type: setup /usr/local/src
3) Move to /usr/local/src/mdb and review additional information in the
   README file.
4) Edit Makefile for Compiler and extra options as required.
5) make 
6) make install
7) make install_man

Comments to:

Philip Murton. 
philip.murton@utoronto.ca
\
