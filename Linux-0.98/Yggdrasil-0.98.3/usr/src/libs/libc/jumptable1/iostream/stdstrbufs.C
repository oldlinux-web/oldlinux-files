//    This is part of the iostream library, providing input/output for C++.
//    Copyright (C) 1992 Per Bothner.
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Library General Public
//    License as published by the Free Software Foundation; either
//    version 2 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Library General Public License for more details.
//
//    You should have received a copy of the GNU Library General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "ioprivate.h"

#if _G_NAMES_HAVE_UNDERSCORE
#define UNDERSCORE "_"
#else
#define UNDERSCORE ""
#endif

// To avoid problems depending on constructor order (and for
// efficiency) the standard streambufs (and streams) are
// constructed statically using C-style '{ ... }' initializers.
// Since you're not allowed to do this for structs that
// have virtuals, we define fake streambuf and stream classes
// that don't have any C++-isms, and initialize those.
// To initialize the vtable field of the standard filebufs,
// we use the expression 'vt_filebuf' which must evaluate to
// (the address of) the virtual function table for the
// filebuf class.

#if !defined(vt_filebuf)
#ifndef __GNUG__
// This works for cfront.
#define vt_filebuf __vtbl__7filebuf
extern char vt_filebuf[1];
#elif _G_DOLLAR_IN_LABEL
extern char vt_filebuf[1] asm(UNDERSCORE "_vt$filebuf");
#else
extern char vt_filebuf[1] asm(UNDERSCORE "_vt.filebuf");
#endif
#endif /* !defined(vt_filebuf) */

struct _fake_filebuf {
    struct __streambuf s;
    char* vtable;
    struct __file_fields f;
};

#define DEF_STD(NAME, FD, CHAIN, FLAGS) \
    _fake_filebuf NAME[1] = {{\
       { _IO_MAGIC+_S_LINKED+_S_IS_FILEBUF+_S_IS_BACKUPBUF+FLAGS, \
	 0, 0, 0, 0, 0, 0, 0, 0, CHAIN, 0, 0, 0, 0, 0},\
       vt_filebuf, {FD}}};

DEF_STD(_cin_sbuf, 0, 0, _S_NO_WRITES);
DEF_STD(_cout_sbuf, 1, (streambuf*)_cin_sbuf, _S_NO_READS);
DEF_STD(_cerr_sbuf, 2, (streambuf*)_cout_sbuf, _S_NO_READS);

DEF_STD(not_open_filebuf, -1, (streambuf*)0, _S_NO_READS+_S_NO_WRITES);

streambuf* streambuf::_list_all = (streambuf*)_cerr_sbuf;
