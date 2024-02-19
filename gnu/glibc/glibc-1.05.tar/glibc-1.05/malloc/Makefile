# Copyright (C) 1991, 1992 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

#
#	Makefile for malloc routines
#
subdir	:= malloc

all: 

headers	:= malloc.h obstack.h

dist-routines := malloc free realloc calloc morecore \
		 valloc memalign mcheck mtrace mstats
routines      := $(dist-routines)	\
		 cfree			\
		 obstack
gpl2lgpl := obstack.c obstack.h	# Frob these guys' copying notices.

install-lib := libmcheck.a

distribute := mcheck-init.c ChangeLog TODO \
	      malloc/gmalloc-head.c dist-README dist-Makefile \
	      mtrace.awk


# Make the standalone malloc distribution.
malloc-dist := README COPYING.LIB Makefile ChangeLog \
	       $(addsuffix .c,$(dist-routines)) $(headers) gmalloc.c \
	       gmalloc-head.c mtrace.awk
%.uu: %
	uuencode $< < $< > $@-tmp
	mv $@-tmp $@
%.Z: %
	compress -c $< > $@-tmp
	mv $@-tmp $@
malloc.tar: $(addprefix malloc/,$(malloc-dist))
	tar ch$(verbose)f $@ $^
malloc/%.c: %.c malloc/
	rm -f $@
	ln -s ../$< $@
malloc/%.h: %.h malloc/
	rm -f $@
	ln -s ../$< $@
malloc/Makefile: dist-Makefile
	sed -e 's,<DIST-SOURCES>,$(addsuffix .c,$(dist-routines)),' \
	    -e 's,<DIST-OBJECTS>,$(addsuffix .o,$(dist-routines)),' \
	    -e 's,<DIST-HEADERS>,$(headers),' < $< > $@-tmp
	mv $@-tmp $@
malloc/gmalloc.c: malloc/Makefile \
		  $(addprefix malloc/,$(headers) \
			      	      $(addsuffix .c,$(dist-routines)))
	$(MAKE) -C malloc gmalloc.c
malloc/README: dist-README
	@rm -f $@
	cp $< $@
malloc/%: %
	@rm -f $@
	cp $< $@


include ../Rules

$(objpfx)libmcheck.a: $(objpfx)mcheck-init.o
	-rm -f $@
	ln $< $@
