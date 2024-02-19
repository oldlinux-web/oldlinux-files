#
#   Makefile for dvips
#
.SUFFIXES: .pro .lpro

# where the installed binary goes
BINDIR = /usr/TeX/bin
HDRFILE	=/usr/TeX/lib/ps
#BINDIR = /ug/bin
#HDRFILE	=/ug/lib/tex82

# see the file paths.h for the paths

# where the manual page goes
MANFILE = /usr/man/man1/dvips.1

PRO	= tex.pro texps.pro texps-scriptprinter.pro

# add -DDEBUG to turn on debugging capability
# add -DTPIC for tpic support
# add -DScriptPrinter for DEC ScriptPrinter
# add -DUltraScript for Imagen UltraScript support
#
DEFS= -DDEBUG -DTPIC -DPSFIG -DScriptPrinter

CFLAGS = -O $(DEFS)

SRC = dospecial.c dviinput.c fontdef.c loadfont.c dvips.c tfmload.c \
	download.c prescan.c scanpage.c skippage.c output.c scalewidth.c \
	dosection.c dopage.c resident.c search.c unpack.c drawPS.c \
	header.c dpicheck.c

OBJ = dospecial.o dviinput.o fontdef.o loadfont.o dvips.o tfmload.o \
	download.o prescan.o scanpage.o skippage.o output.o scalewidth.o \
	dosection.o dopage.o resident.o search.o unpack.o drawPS.o \
	header.o dpicheck.o

all : dvips $(PRO)

dvips : $(OBJ)
	cc $(CFLAGS) $(OBJ) -lm -o dvips

$(OBJ) : structures.h

squeeze : squeeze.o
	cc $(CFLAGS) squeeze.o -o squeeze

tex.pro : tex.lpro squeeze
texps.pro : texps.lpro squeeze
texps-scriptprinter.pro : texps-scriptprinter.lpro squeeze

.lpro.pro:; ./squeeze < $*.lpro > $*.pro

resident.o dvips.o loadfont.o tfmload.o : paths.h

man : dvips.1

dist :
	tar cf dvips.tar README Makefile tex.lpro texps.lpro config.ps *.c *.h
	compress dvips.tar

# install : dvips tex.pro dvips.1
install : dvips tex.pro texps.pro
	-install -c -m 644 dvips.1 $(MANFILE)
	-install -s -c -m 755 dvips $(BINDIR)/dvips
	-cp $(PRO) $(HDRFILE)
	-(cd $(HDRFILE) ; chmod 644 $(PRO))
#	install -c -m 644 dvips.1 $(MANFILE)

clean :
	rm -f *.o dvips squeeze tex.pro

lint :
	lint $(DEFS) $(SRC)
	lint $(DEFS) squeeze.c

shar :
	shar Makefile README tex.lpro texps.lpro config.ps structures.h \
	$(SRC) > dvips.shar
	compress -f dvips.shar

depend:
	$(CC) -M $(SRC) > Makefile.depend
#
#	append Makefile.depend yourself
#
dospecial.o: dospecial.c
dospecial.o: ./structures.h
dospecial.o: ./paths.h
dospecial.o: ./debug.h
dviinput.o: dviinput.c
dviinput.o: ./structures.h
dviinput.o: ./paths.h
dviinput.o: ./debug.h
fontdef.o: fontdef.c
fontdef.o: ./structures.h
fontdef.o: ./paths.h
fontdef.o: ./debug.h
loadfont.o: loadfont.c
loadfont.o: ./structures.h
loadfont.o: ./paths.h
loadfont.o: ./debug.h
dvips.o: dvips.c
dvips.o: ./structures.h
dvips.o: ./paths.h
dvips.o: ./debug.h
tfmload.o: tfmload.c
tfmload.o: ./structures.h
tfmload.o: ./paths.h
tfmload.o: ./debug.h
download.o: download.c
download.o: ./structures.h
download.o: ./paths.h
download.o: ./debug.h
prescan.o: prescan.c
prescan.o: ./structures.h
prescan.o: ./paths.h
prescan.o: ./debug.h
scanpage.o: scanpage.c
scanpage.o: ./structures.h
scanpage.o: ./paths.h
scanpage.o: ./debug.h
skippage.o: skippage.c
skippage.o: ./structures.h
skippage.o: ./paths.h
skippage.o: ./debug.h
output.o: output.c
output.o: ./structures.h
output.o: ./paths.h
output.o: ./debug.h
scalewidth.o: scalewidth.c
scalewidth.o: ./structures.h
scalewidth.o: ./paths.h
scalewidth.o: ./debug.h
dosection.o: dosection.c
dosection.o: ./structures.h
dosection.o: ./paths.h
dosection.o: ./debug.h
dopage.o: dopage.c
dopage.o: ./structures.h
dopage.o: ./paths.h
dopage.o: ./debug.h
resident.o: resident.c
resident.o: ./structures.h
resident.o: ./paths.h
resident.o: ./debug.h
search.o: search.c
search.o: ./structures.h
search.o: ./paths.h
search.o: ./debug.h
search.o: ./debug.h
unpack.o: unpack.c
unpack.o: ./structures.h
unpack.o: ./paths.h
unpack.o: ./debug.h
drawPS.o: drawPS.c
header.o: header.c
header.o: ./structures.h
header.o: ./paths.h
header.o: ./debug.h
dpicheck.o: dpicheck.c
dpicheck.o: ./structures.h
dpicheck.o: ./paths.h
dpicheck.o: ./debug.h
