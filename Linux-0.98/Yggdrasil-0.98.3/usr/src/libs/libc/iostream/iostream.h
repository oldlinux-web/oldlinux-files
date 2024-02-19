//    This is part of the iostream library, providing -*- C++ -*- input/output.
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

#ifndef _IOSTREAM_H
#ifdef __GNUG__
#pragma interface
#endif
#define _IOSTREAM_H
#define Q_ENV

#include <streambuf.h>

class istream; class ostream;
typedef ios& (*__manip)(ios&);
typedef istream& (*__imanip)(istream&);
typedef ostream& (*__omanip)(ostream&);

extern istream& ws(istream& ins);
extern ostream& flush(ostream& outs);
extern ostream& endl(ostream& outs);
extern ostream& ends(ostream& outs);

class ostream : public ios
{
    void do_osfx();
  public:
    ostream() { }
    ostream(streambuf* sb, ostream* tied=NULL);
    int opfx() { if (!good()) return 0; if (_tie) _tie->flush(); return 1; }
    void osfx() { if (flags() & (ios::unitbuf|ios::stdio))
		      do_osfx(); }
    streambuf* ostreambuf() const { return _strbuf; }
    ostream& flush();
    ostream& put(char c) { _strbuf->sputc(c); return *this; }

    ostream& write(const char *s, int n);
    ostream& write(const unsigned char *s, int n) { return write((const char*)s, n);}
#ifndef _G_BROKEN_SIGNED_CHAR
    ostream& write(const signed char *s, int n) { return write((const char*)s, n);}
#endif
    ostream& write(const void *s, int n) { return write((const char*)s, n);}
    ostream& seekp(streampos);
    ostream& seekp(streamoff, _seek_dir);
    streampos tellp();
    ostream& form(const char *format ...);
    ostream& vform(const char *format, _G_va_list args);
};

extern ostream& operator<<(ostream&, char c);
inline ostream& operator<<(ostream& os, unsigned char c)
{ return os << (char)c; }
#ifndef _G_BROKEN_SIGNED_CHAR
extern ostream& operator<<(ostream &os, signed char c) { return os << (char)c;}
#endif
extern ostream& operator<<(ostream&, const char *s);
inline ostream& operator<<(ostream& os, const unsigned char *s)
{ return os << (const char*)s; }
#ifndef _G_BROKEN_SIGNED_CHAR
inline ostream& operator<<(ostream& os, const signed char *s)
{ return os << (const char*)s; }
#endif
extern ostream& operator<<(ostream&, void *p);
extern ostream& operator<<(ostream&, int n);
extern ostream& operator<<(ostream&, long n);
extern ostream& operator<<(ostream&, unsigned int n);
extern ostream& operator<<(ostream&, unsigned long n);
inline ostream& operator<<(ostream& os, short n) {return os << (int)n;}
inline ostream& operator<<(ostream& os, unsigned short n)
{return os << (unsigned int)n;}
extern ostream& operator<<(ostream&, float n);
extern ostream& operator<<(ostream&, double n);
inline ostream& operator<<(ostream& os, __omanip func) { return (*func)(os); }
inline ostream& operator<<(ostream& os, __manip func) {(*func)(os); return os;}
extern ostream& operator<<(ostream&, streambuf*);

class istream : public ios
{
    _G_ssize_t _gcount;

    int _skip_ws();
  public:
    istream() { _gcount = 0; }
    istream(streambuf* sb, ostream*tied=NULL);
    streambuf* istreambuf() const { return _strbuf; }
    istream& get(char& c);
    istream& get(unsigned char& c) { return get((char&)c); }
#ifndef _G_BROKEN_SIGNED_CHAR
    istream& get(signed char& c)  { return get((char&)c); }
#endif
    istream& read(char *ptr, int n);
    istream& read(unsigned char *ptr, int n) { return read((char*)ptr, n); }
#ifndef _G_BROKEN_SIGNED_CHAR
    istream& read(signed char *ptr, int n) { return read((char*)ptr, n); }
#endif
    istream& read(void *ptr, int n) { return read((char*)ptr, n); }
    // Should get() and/or peek() set failbit and/or eofbit? FIXME!
    istream& getline(char* ptr, int len, char delim = '\n');
    istream& get(char* ptr, int len, char delim = '\n');
    istream& get(streambuf& sb, char delim = '\n');
    istream& gets(char **s, char delim = '\n');
    int ipfx(int need) {
	if (!good()) { set(ios::failbit); return 0; }
	if (_tie && (need == 0 || rdbuf()->in_avail() < need)) _tie->flush();
	if (!need && (flags() & ios::skipws)) return _skip_ws();
	return 1;
    }
    int ipfx0() { // Optimized version of ipfx(0).
	if (!good()) { set(ios::failbit); return 0; }
	if (_tie) _tie->flush();
	if (flags() & ios::skipws) return _skip_ws();
	return 1;
    }
    int ipfx1() { // Optimized version of ipfx(1).
	if (!good()) { set(ios::failbit); return 0; }
	if (_tie && rdbuf()->in_avail() == 0) _tie->flush();
	return 1;
    }
    int get() { if (!ipfx1()) return EOF;
		int ch = _strbuf->sbumpc();
		if (ch == EOF) set(ios::eofbit);
		return ch; }
    int peek() { if (!ipfx1()) return EOF;
		int ch = _strbuf->sgetc();
		if (ch == EOF) set(ios::eofbit);
		return ch; }
    _G_ssize_t gcount() { return _gcount; }
    istream& ignore(int n=1, int delim = EOF);
    istream& seekg(streampos);
    istream& seekg(streamoff, _seek_dir);
    streampos tellg();
    istream& putback(char ch) {
	if (good() && _strbuf->sputbackc(ch) == EOF) clear(ios::badbit);
	return *this;}
    istream& unget() {
	if (good() && _strbuf->sungetc() == EOF) clear(ios::badbit);
	return *this;}
    istream& scan(const char *format ...);
    istream& vscan(const char *format, _G_va_list args);
#ifdef _STREAM_COMPAT
    istream& unget(char ch) { return putback(ch); }
    int skip(int i);
#endif
};

extern istream& operator>>(istream&, char*);
inline istream& operator>>(istream& is, unsigned char* p)
{ return is >> (char*)p; }
#ifndef _G_BROKEN_SIGNED_CHAR
inline istream& operator>>(istream& is, signed char*p) {return is >> (char*)p;}
#endif
extern istream& operator>>(istream&, char& c);
extern istream& operator>>(istream& s, unsigned char& c) {return s>>(char&)c;}
#ifndef _G_BROKEN_SIGNED_CHAR
extern istream& operator>>(istream& s, signed char& c) {return s >> (char&)c;}
#endif
extern istream& operator>>(istream&, int&);
extern istream& operator>>(istream&, long&);
extern istream& operator>>(istream&, short&);
extern istream& operator>>(istream&, unsigned int&);
extern istream& operator>>(istream&, unsigned long&);
extern istream& operator>>(istream&, unsigned short&);
extern istream& operator>>(istream&, float&);
extern istream& operator>>(istream&, double&);
inline istream& operator>>(istream& is, __manip func) {(*func)(is); return is;}
inline istream& operator>>(istream& is, __imanip func) { return (*func)(is); }
extern istream& operator>>(istream&, streambuf*);

class iostream : public ios {
    _G_ssize_t _gcount;
  public:
    iostream() { _gcount = 0; }
    iostream(streambuf* sb, ostream*tied=NULL);
    operator istream&() { return *(istream*)this; }
    operator ostream&() { return *(ostream*)this; }
    // NOTE: These duplicate istream methods.
    istream& get(char& c) { return ((istream*)this)->get(c); }
    istream& get(unsigned char& c) { return ((istream*)this)->get((char&)c); }
#ifndef _G_BROKEN_SIGNED_CHAR
    istream& get(signed char& c) { return ((istream*)this)->get((char&)c); }
#endif
    istream& read(char *ptr, int n) { return ((istream*)this)->read(ptr, n); }
    istream& read(unsigned char *ptr, int n)
	{ return ((istream*)this)->read((char*)ptr, n); }
#ifndef _G_BROKEN_SIGNED_CHAR
    istream& read(signed char *ptr, int n)
	{ return ((istream*)this)->read((char*)ptr, n); }
#endif
    istream& read(void *ptr, int n)
	{ return ((istream*)this)->read((char*)ptr, n); }
    istream& getline(char* ptr, int len, char delim = '\n')
	{ return ((istream*)this)->getline(ptr, len, delim); }
    istream& get(char* ptr, int len, char delim = '\n')
	{ return ((istream*)this)->get(ptr, len, delim); }
    istream& gets(char **s, char delim = '\n')
	{ return ((istream*)this)->gets(s, delim); }
    istream& ignore(int n=1, int delim = EOF)
	{ return ((istream*)this)->ignore(n, delim); }
    int ipfx(int need) { return ((istream*)this)->ipfx(need); }
    int ipfx0()  { return ((istream*)this)->ipfx0(); }
    int ipfx1()  { return ((istream*)this)->ipfx1(); }
    int get() { return _strbuf->sbumpc(); }
    int peek() { return ipfx1() ? _strbuf->sgetc() : EOF; }
    _G_ssize_t gcount() { return _gcount; }
    istream& putback(char ch) { return ((istream*)this)->putback(ch); }
    istream& unget() { return ((istream*)this)->unget(); }
    istream& seekg(streampos pos) { return ((istream*)this)->seekg(pos); }
    istream& seekg(streamoff off, _seek_dir dir)
	{ return ((istream*)this)->seekg(off, dir); }
    streampos tellg() { return ((istream*)this)->tellg(); }
    istream& scan(const char *format ...);
    istream& vscan(const char *format, _G_va_list args)
	{ return ((istream*)this)->vscan(format, args); }
#ifdef _STREAM_COMPAT
    istream& unget(char ch) { return putback(ch); }
#endif

    // NOTE: These duplicate ostream methods.
    int opfx() { return ((ostream*)this)->opfx(); }
    void osfx() { ((ostream*)this)->osfx(); }
    ostream& flush() { return ((ostream*)this)->flush(); }
    ostream& put(char c) { return ((ostream*)this)->put(c); }
    ostream& write(const char *s, int n)
	{ return ((ostream*)this)->write(s, n); }
    ostream& write(const unsigned char *s, int n)
	{ return ((ostream*)this)->write((const char*)s, n); }
#ifndef _G_BROKEN_SIGNED_CHAR
    ostream& write(const signed char *s, int n)
	{ return ((ostream*)this)->write((const char*)s, n); }
#endif
    ostream& write(const void *s, int n)
	{ return ((ostream*)this)->write((const char*)s, n); }
    ostream& form(const char *format ...);
    ostream& vform(const char *format, _G_va_list args)
	{ return ((ostream*)this)->vform(format, args); }
    ostream& seekp(streampos pos) { return ((ostream*)this)->seekp(pos); }
    ostream& seekp(streamoff off, _seek_dir dir)
	{ return ((ostream*)this)->seekp(off, dir); }
    streampos tellp() { return ((ostream*)this)->tellp(); }
};

extern istream cin;
extern ostream cout, cerr, clog; // clog->rdbuf() == cerr->rdbuf()

struct Iostream_init { } ;  // Compatibility hack for AT&T libraray.

inline ios& dec(ios& i)
{ i.setf(ios::dec, ios::dec|ios::hex|ios::oct); return i; }
inline ios& hex(ios& i)
{ i.setf(ios::hex, ios::dec|ios::hex|ios::oct); return i; }
inline ios& oct(ios& i)
{ i.setf(ios::oct, ios::dec|ios::hex|ios::oct); return i; }

#endif /*!_IOSTREAM_H*/
