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

#ifndef __STRSTREAM_H
#define __STRSTREAM_H
#ifdef __GNUG__
#pragma interface
#endif
#include <iostream.h>

class strstreambuf : public backupbuf {
    _G_size_t _len; // The current length is max(_len, _pptr-_pbase).
    _G_size_t _size; // Allocated (physical) buffer size.
    char *_buffer;
    int _frozen;
  protected:
    virtual int overflow(int = EOF);
    virtual int underflow();
  public:
    strstreambuf();
    strstreambuf(int initial);
    strstreambuf(char *ptr, int size, char *pstart = NULL);
    ~strstreambuf();
    int frozen() { return _frozen; }
    void freeze(int n=1) { _frozen = n != 0; }
    _G_size_t pcount();
    char *str();
    virtual streampos seekoff(streamoff, _seek_dir, int mode=ios::in|ios::out);
};

class istrstream : public istream {
  public:
    istrstream(char*);
    istrstream(char*, int);
    strstreambuf* rdbuf() { return (strstreambuf*)_strbuf; }
};

class ostrstream : public ostream {
  public:
    ostrstream();
    ostrstream(char *cp, int n, int mode=ios::out);
    _G_size_t pcount() { return ((strstreambuf*)_strbuf)->pcount(); }
    char *str() { return ((strstreambuf*)_strbuf)->str(); }
    void freeze(int n = 1) { ((strstreambuf*)_strbuf)->freeze(n); }
    int frozen() { return ((strstreambuf*)_strbuf)->frozen(); }
    strstreambuf* rdbuf() { return (strstreambuf*)_strbuf; }
};

class strstream : public iostream {
  public:
    strstream();
    strstream(char *cp, int n, int mode);
    _G_size_t pcount() { return ((strstreambuf*)_strbuf)->pcount(); }
    char *str() { return ((strstreambuf*)_strbuf)->str(); }
    void freeze(int n = 1) { ((strstreambuf*)_strbuf)->freeze(n); }
    int frozen() { return ((strstreambuf*)_strbuf)->frozen(); }
    strstreambuf* rdbuf() { return (strstreambuf*)_strbuf; }
};

#endif /*!__STRSTREAM_H*/
