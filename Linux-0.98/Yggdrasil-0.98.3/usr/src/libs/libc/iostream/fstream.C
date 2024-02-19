//    This is part of the iostream library, providing input/output for C++.
//    Copyright (C) 1991 Per Bothner.
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

#define _STREAM_COMPAT
#include "ioprivate.h"
#include <fstream.h>
#ifdef __GNUG__
#pragma implementation
#endif

ifstream::ifstream()
{
    _strbuf = new filebuf();
}

ifstream::ifstream(int fd)
{
    _strbuf = new filebuf(fd);
}

ifstream::ifstream(const char *name, int mode, int prot)
{
    _strbuf = new filebuf();
    if (!rdbuf()->open(name, mode, prot))
	set(ios::badbit);
}

ofstream::ofstream()
{
    _strbuf = new filebuf();
}

ofstream::ofstream(int fd)
{
    _strbuf = new filebuf(fd);
}

ofstream::ofstream(const char *name, int mode, int prot)
{
    _strbuf = new filebuf();
    if (!rdbuf()->open(name, mode, prot))
	set(ios::badbit);
}

#if 0
static int mode_to_sys(enum open_mode mode)
{
    return O_WRONLY;
}

static char* fopen_cmd_arg(io_mode i)
{
    return "w";
}
#endif

void ifstream::open(const char *name, int mode, int prot)
{
    if (!rdbuf()->open(name, mode, prot))
	set(ios::badbit);
}

void ifstream::close()
{
    rdbuf()->close();
}

void ofstream::close()
{
    rdbuf()->close();
}

void ofstream::open(const char *name, int mode, int prot)
{
    if (!rdbuf()->open(name, mode, prot))
	set(ios::badbit);
}

fstream::fstream()
{
    _strbuf = new filebuf();
}

fstream::fstream(int fd)
{
    _strbuf = new filebuf(fd);
}

fstream::fstream(const char *name, int mode, int prot=0664)
{
    _strbuf = new filebuf();
    if (!rdbuf()->open(name, mode, prot))
	set(ios::badbit);
}

void fstream::open(const char *name, int mode, int prot=0664)
{
    if (!rdbuf()->open(name, mode, prot))
	set(ios::badbit);
}

void fstream::close()
{
    rdbuf()->close();
}
