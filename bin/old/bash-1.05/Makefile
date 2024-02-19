# Hey Emacs, this Makefile is in -*- text -*- mode!
#
# Makefile for Bash.
# If your cpp doesn't like -P, just get rid of it.
# If you wish to use Gcc, then just type "make CC=gcc".
# If you wish to use GNU's Make, then change the MAKE define.
# If you don't like the destination, then change DESTDIR.  (This only
# matters if you are typing `make install'.)
# The file that you most likely want to look at is cpp-Makefile.
#
# If you haven't read README, now might be a good time.

DESTDIR = /usr/gnu/bin
MAKE = make
#CPP_DEFINES = -DHAVE_GCC -DHAVE_FIXED_INCLUDES -DHAVE_BISON

CPP = /lib/cpp $(CPPFLAGS)
#CPP = $(CC) -E

CPPFLAGS = -P $(SYSTEM) $(CPP_DEFINES) -DM_DESTDIR=$(DESTDIR)
CPP_ARGS = -DCPP_CC=$(CC) `./makeargs.sh`

all:	bash-Makefile
	$(MAKE) -f bash-Makefile

bash-Makefile: cpp-Makefile Makefile machines.h makeargs.sh
	cp cpp-Makefile tmp-Makefile.c
	@echo $(CPP) $(CPP_ARGS) tmp-Makefile.c \| cat -s >bash-Makefile
	@$(CPP) $(CPP_ARGS) tmp-Makefile.c | cat -s >bash-Makefile
	rm -f tmp-Makefile.c

# Subsequent lines contain targets that are correctly handled by an
# existing bash-Makefile.

install newversion mailable distribution backup : bash-Makefile
	$(MAKE) -f bash-Makefile $@

bash.tar.Z carryall documentation tags clone clean dist-clean: bash-Makefile
	$(MAKE) -f bash-Makefile $@
