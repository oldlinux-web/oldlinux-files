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

#ifdef __GNUG__
#pragma implementation
#endif
#include "ioprivate.h"
#include <strstream.h>

istrstream::istrstream(char *cp) : istream(NULL)
{
    _strbuf = new strstreambuf(cp, 0, NULL);
}

istrstream::istrstream(char *cp, int n) : istream(NULL)
{
    _strbuf = new strstreambuf(cp, n, NULL);
}

ostrstream::ostrstream() : ostream(NULL)
{
    _strbuf = new strstreambuf;
}

ostrstream::ostrstream(char *cp, int n, int mode) : ostream(NULL)
{
    char *pstart;
    if (mode == ios::app || mode == ios::ate)
	pstart = cp + strlen(cp);
    else
	pstart = cp;
    _strbuf = new strstreambuf(cp, n, pstart);
}


strstream::strstream() : iostream(NULL)
{
    _strbuf = new strstreambuf;
}

strstream::strstream(char *cp, int n, int mode) : iostream(NULL)
{
    char *pstart;
    if (mode == ios::app || mode == ios::ate)
	pstart = cp + strlen(cp);
    else
	pstart = cp;
    _strbuf = new strstreambuf(cp, n, pstart);
}


char *strstreambuf::str()
{
    freeze(1);
    return _buffer;
}

_G_size_t strstreambuf::pcount()
{
    _G_size_t put_len = pptr() - pbase();
    if (put_len < _len) put_len = _len;
    return put_len;
}

int strstreambuf::overflow(int c = EOF)
{
  const int flush_only = c == EOF;
  size_t pos = pptr() - pbase();
  size_t get_pos = gptr() - pbase();
  if (pos > _len) _len = pos;
  if (pos >= _size + flush_only) {
      char *new_buf;
      size_t new_size = 2 * _size;
      if (_frozen) /* not allowed to enlarge */
	  return EOF;
      else {
	  new_buf = ALLOC_BUF(new_size);
	  memcpy(new_buf, _buffer, _size);
      }
      if (new_buf == NULL) {
//	  __ferror(fp) = 1;
	  return EOF;
      }
      _size = new_size;
#if 0
      if (lenp == &_len) /* use '\0'-filling */
	  memset(new_buf + pos, 0, _size - pos);
#endif
      _buffer = new_buf;
      setb(new_buf, new_buf + _size, 1);
    }

  setp(_buffer, _buffer + _size);
  pbump(pos);
  setg(_buffer, _buffer + get_pos, _buffer + _len);
  if (!flush_only) {
      *pptr() = (unsigned char) c;
      pbump(1);
  }
  return c;
}

int strstreambuf::underflow()
{
    size_t ppos = pptr() - pbase();
    if (ppos > _len) _len = ppos;
    setg(_buffer, gptr(), _buffer + _len);
    if (gptr() < egptr())
	return *gptr();
    else
	return EOF;
}

strstreambuf::strstreambuf()
{
    _frozen = 0;
    _len = 0;
    _size = 128;
    _buffer = ALLOC_BUF(_size);
    setb(_buffer, _buffer+_size);
    setp(_buffer, _buffer+_size);
    setg(_buffer, _buffer, _buffer);
}

strstreambuf::strstreambuf(int initial)
{
    _frozen = 0;
    _len = 0;
    if (initial < 16)
	initial = 16;
    _size = initial;
    _buffer = ALLOC_BUF(_size);
    setb(_buffer, _buffer+_size);
    setp(_buffer, _buffer+_size);
    setg(_buffer, _buffer, _buffer);
}

strstreambuf::strstreambuf(char *ptr, int size, char *pstart)
{
    _frozen = 1;
    if (size == 0)
	_size = strlen(ptr);
    else if (size < 0) {
	// If size is negative 'the characters are assumed to
	// continue indefinitely.'
	// The following semi-portable kludge tries to do that.
	// It assumes that sizeof(long) == sizeof(pointer).
	// Hence, (unsigned long)(-1) should be the largest
	// possible address.
	unsigned long highest = (unsigned long)(-1);
	// Pointers are signed on some brain-damaged systems, in
	// which case we divide by two to get the maximum signed address.
	if  ((char*)highest < ptr)
	    highest >>= 1;
	_size = (char*)highest - ptr;
    }
    else
	_size = size;
    _buffer = ptr;
    setb(_buffer, _buffer+_size);
    if (pstart) {
	setp(_buffer, _buffer+_size);
	pbump(pstart-_buffer);
	setg(_buffer, _buffer, pstart);
    }
    else {
	setp(_buffer, _buffer); 
	setg(_buffer, _buffer, _buffer+_size);
    }
    _len = egptr() - _buffer;
}

strstreambuf::~strstreambuf()
{
    if (_frozen == 0)
        FREE_BUF(_buffer);
}

virtual streampos strstreambuf::seekoff(streamoff off, _seek_dir dir,
					int mode /*=ios::in|ios::out*/)
{
    size_t cur_size = pcount();
    streampos new_pos = EOF;

    // Move the get pointer, if requested.
    if (mode & ios::in) {
	switch (dir) {
	  case ios::end:
	    off += cur_size;
	    break;
	  case ios::cur:
	    off += gptr() - pbase();
	    break;
	  default: /*case ios::beg: */
	    break;
	}
	if (off < 0 || (size_t)off > cur_size)
	    return EOF;
	setg(_buffer, _buffer + off, _buffer + cur_size);
	new_pos = off;
    }

    // Move the put pointer, if requested.
    if (mode & ios::out) {
	switch (dir) {
	  case ios::end:
	    off += cur_size;
	    break;
	  case ios::cur:
	    off += pptr() - pbase();
	    break;
	  default: /*case ios::beg: */
	    break;
	}
	if (off < 0 || (size_t)off > cur_size)
	    return EOF;
	pbump(_buffer + off - pptr());
	new_pos = off;
    }
    return new_pos;
}
