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
#ifdef NAMES_HAVE_UNDERSCORE
#define UNDERSCORE "_"
#else
#define UNDERSCORE ""
#endif

struct _fake_filebuf;

extern _fake_filebuf _cin_sbuf, _cout_sbuf, _cerr_sbuf;

#if 0 /* defined(__GNUC__) */
// This hack was intended to trick gdb into thinking cin is really a
// istream (and not a _fake_istream), but doesn't seem to work.
#define STR_NAME(TYPE, NAME) TYPE __##NAME asm(UNDERSCORE #NAME)
#else
#define STR_NAME(TYPE, NAME) TYPE NAME
#endif
#define cin CIN
#define cout COUT
#define cerr CERR
#define clog CLOG
#include "iostream.h"
#undef cin
#undef cout
#undef cerr
#undef clog

#ifdef __GNUG__
#define PAD 0 /* g++ allows 0-length arrays. */
#else
#define PAD 1
#endif
class _fake_istream {
    _ios_fields base;
    char filler[sizeof(struct istream)-sizeof(struct _ios_fields)+PAD];
};
class _fake_ostream {
    _ios_fields base;
    char filler[sizeof(struct ostream)-sizeof(struct _ios_fields)+PAD];
};

#define STD_STR(SBUF, TIE, EXTRA_FLAGS) \
 (streambuf*)&SBUF, TIE, 0, ios::dont_close|ios::skipws|EXTRA_FLAGS, ' ', 0, 6

STR_NAME(_fake_ostream, cout)  = { STD_STR(_cout_sbuf, NULL, 0) };
STR_NAME(_fake_ostream, cerr) =
  { STD_STR(_cerr_sbuf, (ostream*)&cout, ios::unitbuf) };
STR_NAME(_fake_istream, cin)  = { STD_STR(_cin_sbuf,  (ostream*)&cout, 0) };

/* Only for (partial) compatibility with AT&T's library. */
STR_NAME(_fake_ostream, clog) = { STD_STR(_cerr_sbuf, (ostream*)&cout, 0) };
