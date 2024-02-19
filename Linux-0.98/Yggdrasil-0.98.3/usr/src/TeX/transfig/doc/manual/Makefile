# 
# Makefile: TransFig makefile for translation of figures into latex
# 
 
all: tex-list
 
trans.tex: trans.fig
	rm -f trans.tex
	fig2dev -Llatex  trans.fig trans.tex
 
tex-list: trans.tex
trans.tex: Makefile
 
clean:
	rm -f trans.tex
