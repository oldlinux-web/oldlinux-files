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

#ifndef _FSTREAM_H
#define _FSTREAM_H
#ifdef __GNUG__
#pragma interface
#endif
#include <iostream.h>

class ifstream : public istream {
  public:
    ifstream();
    ifstream(int fd);
    ifstream(const char *name, int mode=ios::in, int prot=0664);
    void open(const char *name, int mode=ios::in, int prot=0664);
    void close();
    filebuf* rdbuf() const { return (filebuf*)_strbuf; }
#ifdef _STREAM_COMPAT
    int filedesc() { return rdbuf()->fd(); }
    ifstream& raw() { rdbuf()->setbuf(NULL, 0); return *this; }
#endif
};

class ofstream : public ostream {
  public:
    ofstream();
    ofstream(int fd);
    ofstream(const char *name, int mode=ios::out, int prot=0664);
    void open(const char *name, int mode=ios::out, int prot=0664);
    void close();
    filebuf* rdbuf() const { return (filebuf*)_strbuf; }
#ifdef _STREAM_COMPAT
    int filedesc() { return rdbuf()->fd(); }
    ofstream& raw() { rdbuf()->setbuf(NULL, 0); return *this; }
#endif
};

class fstream : public iostream {
  public:
    fstream();
    fstream(int fd);
    fstream(const char *name, int mode, int prot=0664);
    void open(const char *name, int mode, int prot=0664);
    void close();
    filebuf* rdbuf() const { return (filebuf*)_strbuf; }
#ifdef _STREAM_COMPAT
    int filedesc() { return rdbuf()->fd(); }
    fstream& raw() { rdbuf()->setbuf(NULL, 0); return *this; }
#endif
};
#endif /*!_FSTREAM_H*/

