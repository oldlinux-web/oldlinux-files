############################################################# 
# Makefile for the Linux Network Administrators' Guide
# Copyright (C) Olaf Kirch, 1992, 1993.
#
#############################################################
VERSION=0.4.3

#############################################################
# Get the filesets for the various format distributions
#############################################################
include FILESETS

#############################################################
# Commands
#############################################################
TEXIFY=	  texify
MAKEINFO= makeinfo
TEXI2ROFF=texi2roff

#############################################################
# Rules section
#############################################################
all:	dvi	# default target

clean:  cleantex cleanps cleaninfo cleanroff

#############################################################
# Targets relating to fig picture conversion
#############################################################
tcpip/dns.tex:tcpip/dns.fig
	fig2dev -Leepic tcpip/dns.fig > tcpip/dns.tex

#tcpip/groucho.tex:tcpip/groucho.fig
#	fig2dev -Leepic tcpip/groucho.fig > tcpip/groucho.tex

news/flow.tex: news/flow.fig
	fig2dev -Leepic news/flow.fig > news/flow.tex

#############################################################
# Targets relating to TeX/DVI/PS
dvi:	nag.dvi

ps:	nag.ps

nag.dvi:$(TEX)
	export TEXINPUTS=:styles; latex nag.tex || touch nag.tex
	-@grep -s Rerun nag.log && touch nag.tex

nag.ps:	nag.dvi
	dvips -M nag.dvi -o nag.ps

cleantex:
	rm -f *.aux */*.aux *.log *.lof *.toc nag.dvi nag.dvi-err

cleanps:
	rm -f nag.ps

#############################################################
# Targets relating to TeXinfo/Info
#############################################################
texi:	net.texi

info: 	netguide

netguide:$(TEXI)
	$(MAKEINFO) nag.texi

net.texi:$(TEXISRC)
	$(TEXIFY) net.tex
	$(TEXIFY) net.tex

cleaninfo:
	rm -f netguide netguide-[0-9] netguide-[0-9][0-9] \
		net.texi net.auxi net.ff net.menu

#############################################################
# Targets relating to groff/ASCII
#############################################################
roff:	$(ROFF)

net.ms:$(ROFFSRC)
	texi2roff -ms net.texi > net.ms

ascii:	nag.ascii

nag.ascii:$(ROFF)
	groff -mgs nag.ms | sed 's/.//g' > nag.ascii

cleanroff:
	rm -f net.ms nag.ascii

#############################################################
# Make distribution files
#############################################################
dist:	srcdist dvidist psdist texidist roffdist

srcdist:
	@ls $(SRCDIST)  | sed "s/^/.\/netguide\//" > MANIFEST
	-cd ..; tar  -cvf - -T netguide/MANIFEST |\
		 gzip > nagsrc-$(VERSION).tgz

dvidist:
	@ls $(DVIDIST)  | sed "s/^/.\/netguide\//" > MANIFEST
	-cd ..; tar  -cvf - -T netguide/MANIFEST |\
		gzip > nagdvi-$(VERSION).tgz

psdist:
	@ls $(PSDIST)   | sed "s/^/.\/netguide\//" > MANIFEST
	-cd ..; tar  -cvf - -T netguide/MANIFEST |\
		gzip > nagps-$(VERSION).tgz

texidist:
	@ls $(TEXIDIST) | sed "s/^/.\/netguide\//" > MANIFEST
	-cd ..; tar  -cvf - -T netguide/MANIFEST |\
		gzip > nagtexi-$(VERSION).tgz

roffdist:
	@ls $(ROFFDIST) | sed "s/^/.\/netguide\//" > MANIFEST
	-cd ..; tar  -cvf - -T netguide/MANIFEST |\
		gzip > nagroff-$(VERSION).tgz

