LATEX=latex
FIG2DEV=fig2dev

IMAGES=bootloader.tex map.tex image.tex unstripped.tex other.tex

.SUFFIXES:	.fig .tex .dvi

.fig.tex:
		$(FIG2DEV) -L latex $*.fig $*.tex

doc.dvi:	doc.tex $(IMAGES)
		$(LATEX) doc

again:
		$(LATEX) doc
