SCREEN=		sunv
PAGES=		15-99
PRINT=		oki180a4

it:		it.out
		pre <it.out | sunv

it.out:		origami.ms ocl.ms functions.ms
		ehyph origami.ms | mroff -ms -o$(PAGES) >it.out

preview:	title.out body.out
		cat title.out body.out | pre | sunv

body.out:	origami.ms ocl.ms functions.ms
		ehyph origami.ms | mroff -ms >body.out

title.out:	titlepage.mr
		mroff titlepage.mr >title.out

clean:
		rm -f *.out
