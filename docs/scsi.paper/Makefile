# Makefile -- Makefile for "Writing a SCSI Device Driver for Linux"
# Created: Mon Jun  7 21:16:08 1993 by faith@cs.unc.edu
# Revised: Mon Jun  7 21:30:03 1993 by faith@cs.unc.edu
# Copyright 1993 Rickard E. Faith (faith@cs.unc.edu)
# Written at the University of North Carolina, 1993
# This program comes with ABSOLUTELY NO WARRANTY.
# 
# $Log: Makefile,v $
# Revision 1.1  1993/06/07  21:35:46  root
# Initial revision
#
#

scsi.dvi: scsi.tex scsi.bib
	latex scsi
	bibtex scsi
	latex scsi
	latex scsi
	latex scsi

.PHONEY: clean veryclean
clean:
	rm -f *~ *.aux *.bbl *.blg *.dvi-jep *.log *.toc

veryclean:
	rm -f *~ *.aux *.bbl *.blg *.dvi-jep *.log *.toc *.dvi *.ps
