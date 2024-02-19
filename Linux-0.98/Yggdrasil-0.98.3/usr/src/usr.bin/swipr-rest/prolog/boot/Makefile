# Make boot files (library sensitive parts)

# Variables imported from the environment
PLLIB=/usr/prolog/swi-prolog/library
VIPATH=/usr/ucb/vi

.SUFFIXES: .cpp .pl

CPP=/lib/cpp
CPPFLAGS=\
	-C -P -undef \
	-DPLLIB="'$(PLLIB)'" \
	-DVIPATH="'$(VIPATH)'"

PL=	parms.pl

all:	$(PL)

$(PL):	Makefile

install:

lib:

.cpp.pl:
	$(CPP) $(CPPFLAGS) $*.cpp > $*.pl
