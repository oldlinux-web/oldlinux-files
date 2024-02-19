# Generated automatically from Makefile.in by configure.
# Makefile for GNU shell utilities programs.
# Do not use this makefile directly, but only from `../Makefile'.
# Copyright (C) 1991 Free Software Foundation, Inc.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

SHELL = /bin/sh

srcdir = .
VPATH = .

SOURCES = basename.c date.c dirname.c env.c expr.c id.c logname.c \
pathchk.c printenv.c printf.c sleep.c su.c tee.c test.c tty.c who.c \
whoami.c yes.c nice.c stty.c uname.c version.c
DISTFILES = Makefile.in system.h $(SOURCES) groups.sh nohup.sh \
false.sh true.sh

PROGS = basename date dirname env expr false groups id logname \
pathchk printenv printf sleep tee test true tty who whoami yes \
 uname nice nohup

all: $(PROGS) su
.PHONY: all

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(DEFS) -I$(srcdir)/../lib $<

install: all
	for f in $(PROGS); do $(INSTALL_PROGRAM) $$f $(bindir)/$(binprefix)$$f; done
	rm -f $(bindir)/'['; ln $(bindir)/$(binprefix)test $(bindir)/'['
	@if test `./id -u` = 0; then \
	echo you are root, so installing su; \
	$(INSTALL_PROGRAM) su $(bindir)/$(binprefix)su; \
	chown root $(bindir)/$(binprefix)su; \
	chmod 4755 $(bindir)/$(binprefix)su; \
	else echo you are not root, so not installing su; fi
.PHONY: install

TAGS: $(SOURCES)
	etags $(SOURCES)

clean:
	rm -f $(PROGS) *.o core
.PHONY: clean

mostlyclean: clean
.PHONY: mostlyclean

distclean: clean
	rm -f Makefile
.PHONY: distclean

realclean: distclean
	rm -f TAGS
.PHONY: realclean

dist:
	ln $(DISTFILES) ../`cat ../.fname`/src
.PHONY: dist

$(PROGS): ../lib/libshu.a version.o

basename: basename.o
	$(CC) $(LDFLAGS) -o $@ basename.o ../lib/libshu.a version.o $(LIBS)

date: date.o
	$(CC) $(LDFLAGS) -o $@ date.o ../lib/libshu.a version.o $(LIBS)

dirname: dirname.o
	$(CC) $(LDFLAGS) -o $@ dirname.o ../lib/libshu.a version.o $(LIBS)

env: env.o
	$(CC) $(LDFLAGS) -o $@ env.o ../lib/libshu.a version.o $(LIBS)

expr: expr.o
	$(CC) $(LDFLAGS) -o $@ expr.o ../lib/libshu.a version.o $(LIBS)

false: false.sh
	rm -f $@
	cp $(srcdir)/false.sh false

id: id.o
	$(CC) $(LDFLAGS) -o $@ id.o ../lib/libshu.a version.o $(LIBS)

groups: groups.sh
	rm -f $@
	sed "s,@bindir@,$(bindir),g" $(srcdir)/groups.sh > $@
	chmod +x $@

logname: logname.o
	$(CC) $(LDFLAGS) -o $@ logname.o ../lib/libshu.a version.o $(LIBS)

nohup: nohup.sh
	rm -f $@
	sed "s,@bindir@,$(bindir),g" $(srcdir)/nohup.sh > $@
	chmod +x $@

pathchk: pathchk.o
	$(CC) $(LDFLAGS) -o $@ pathchk.o ../lib/libshu.a version.o $(LIBS)

printenv: printenv.o
	$(CC) $(LDFLAGS) -o $@ printenv.o ../lib/libshu.a version.o $(LIBS)

# Link with -lm in case strtod.o needs to get `pow'.
printf: printf.o
	$(CC) $(LDFLAGS) -o $@ printf.o ../lib/libshu.a version.o $(LIBS) -lm

sleep: sleep.o
	$(CC) $(LDFLAGS) -o $@ sleep.o ../lib/libshu.a version.o $(LIBS)

su: su.o
	$(CC) $(LDFLAGS) -o $@ su.o ../lib/libshu.a version.o $(LIBS)

tee: tee.o
	$(CC) $(LDFLAGS) -o $@ tee.o ../lib/libshu.a version.o $(LIBS)

test: test.o
	$(CC) $(LDFLAGS) -o $@ test.o ../lib/libshu.a version.o $(LIBS)

true: true.sh
	rm -f $@
	cp $(srcdir)/true.sh true

tty: tty.o
	$(CC) $(LDFLAGS) -o $@ tty.o ../lib/libshu.a version.o $(LIBS)

who: who.o
	$(CC) $(LDFLAGS) -o $@ who.o ../lib/libshu.a version.o $(LIBS)

whoami: whoami.o
	$(CC) $(LDFLAGS) -o $@ whoami.o ../lib/libshu.a version.o $(LIBS)

yes: yes.o
	$(CC) $(LDFLAGS) -o $@ yes.o ../lib/libshu.a version.o $(LIBS)

nice: nice.o
	$(CC) $(LDFLAGS) -o $@ nice.o ../lib/libshu.a version.o $(LIBS)

stty: stty.o
	$(CC) $(LDFLAGS) -o $@ stty.o ../lib/libshu.a version.o $(LIBS)

uname: uname.o
	$(CC) $(LDFLAGS) -o $@ uname.o ../lib/libshu.a version.o $(LIBS)

# Special rules for some .o files.

su.o: su.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(DEFS) -DSYSLOG_FAILURE -DSYSLOG_SUCCESS -I$(srcdir)/../lib $(srcdir)/su.c

test.o: test.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(DEFS) -DTEST_STANDALONE -I$(srcdir)/../lib $(srcdir)/test.c

env.o id.o nice.o pathchk.o stty.o su.o tty.o uname.o who.o: ../lib/getopt.h
expr.o: ../lib/regex.h

# Prevent GNU make v3 from overflowing arg limit on SysV.
.NOEXPORT:
