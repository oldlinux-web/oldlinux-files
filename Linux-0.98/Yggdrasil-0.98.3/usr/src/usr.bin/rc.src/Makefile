# Makefile for rc.

# Check the definitions in stddef.h and the configuration parameters below
# to make sure they are correct for your system.

# Configuration parameters for rc:
# 
# Configurations may be added or deleted using ed; e.g., the ed command
# for adding the NODIRENT configuration would be
# 
# /#NODIRENT/s/^.//
#
# Note that config.h automatically sets up configurations for some
# systems.

# Define the macro NODIRENT if your system has <sys/dir.h> but not
# <dirent.h>. (e.g., NeXT-OS)
#NODIRENT	= -DNODIRENT

# Define the macro DEVFD if your system supports /dev/fd.
#DEVFD		= -DDEVFD

# Define the macro NOLIMITS if your system does not support Berkeley
# limits.
#NOLIMITS	= -DNOLIMITS

# Define the macro NOSIGCLD if your system uses SIGCLD in the System
# V way. (e.g., Irix)
#NOSIGCLD	= -DNOSIGCLD

# Define the macro READLINE if you want rc to call GNU readline
# instead of read(2) on interactive shells.
#READLINE	= -DREADLINE

# Define the macro NOEXECVE if your Unix does not interpret #! in the
# kernel, and set EXECVE to execve.o.
#NOEXECVE	= -DNOEXECVE
#EXECVE		= execve.o

# Define the macro ADDON if you wish to extend rc via locally-defined
# builtins. An interface is provided in addon.[ch]. Note that the author
# does not endorse any such extensions, rather hopes that this way
# rc will become useful to more people.
#ADDON		= addon.o

# If you want rc to default to some interpreter for files which don't
# have a legal #! on the first line, define the macro DEFAULTINTERP.
#DEFAULTINTERP	= -DDEFAULTINTERP=\"/bin/sh\"

# If your /bin/sh (or another program you care about) rejects
# environment variables with special characters in them, rc can put
# out ugly variable names using [_0-9a-zA-Z] that encode the real
# name; define PROTECT_ENV for this hack.
#PROTECT_ENV	= -DPROTECT_ENV

# If your window system has a broken terminal emulator (and I'm talking
# specifically about the NeXT here) which expects your shell to do csh-
# like job control stuff on startup, define PROTECT_JOB so that rc can
# do the "right" thing.
#PROTECT_JOB	= -DPROTECT_JOB

CONFIG = $(NODIRENT) $(NOCMDARG) $(DEVFD) $(NOLIMITS) $(NOSIGCLD) \
	$(READLINE) $(NOEXECVE)	$(DEFAULTINTERP) $(PROTECT_ENV) $(PROTECT_JOB)

# Use an ANSI compiler (or at least one that groks prototypes and void *):
CC=gcc -g -O
CFLAGS=$(CONFIG)
LDFLAGS=

# Use bison if you will, but yacc generates a smaller y.tab.c, and the speed
# of the parser is largely irrelevant in a shell.
YACC=yacc

OBJS =  $(ADDON) builtins.o except.o exec.o $(EXECVE) fn.o footobar.o getopt.o \
	glob.o glom.o hash.o heredoc.o input.o lex.o list.o main.o match.o \
	nalloc.o open.o redir.o sigmsgs.o status.o tree.o utils.o var.o \
	version.o wait.o walk.o which.o y.tab.o

# If rc is compiled with GNU readline, you must supply the correct arguments to
# ld on this line. Typically this would be something like:
#
#	$(CC) -o rc $(OBJS) -lreadline -ltermcap

rc: $(OBJS)
	$(CC) -o rc $(OBJS)

sigmsgs.c: mksignal
	sh mksignal /usr/include/sys/signal.h

y.tab.c: parse.y
	$(YACC) -d parse.y

clean:
	rm -f a.* *.o *.s core *.tab.* *.out

history: history.c
	$(CC) -o ./- history.c
	rm -f ./-- ./-p ./--p
	ln -s ./- ./--
	ln -s ./- ./-p
	ln -s ./- ./--p

C: force
	-mkdir C
	for i in *.c; do awk -f cfix.awk $$i >C/$$i; done
	for i in *.h; do awk -f hfix.awk $$i >C/$$i; done
	sed 's/CFLAGS=/&-Dconst= /' Makefile > C/Makefile
	cp mksignal *.y C

force:

# dependencies:

sigmsgs.h: sigmsgs.c
lex.o y.tab.o: y.tab.c
builtins.c fn.c status.c hash.c: sigmsgs.h
