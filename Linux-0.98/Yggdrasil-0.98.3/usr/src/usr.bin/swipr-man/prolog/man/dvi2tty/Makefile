# Makefile for dvi2tty and disdvi       23/01/89   M.J.E. Mol
#
#
# This Makefile does not work for MSDOS. Make your 
# own one, or compile by hand.
#
#CFLAGS = 

# For BSD Unix use the following CFLAGS definition
CFLAGS = -Dstrchr=index
all:	dvi2tty disdvi

dvi2tty:dvi2tty.o dvistuff.o
	cc -o dvi2tty dvi2tty.o dvistuff.o

disdvi:disdvi.o
	cc -o disdvi disdvi.o
