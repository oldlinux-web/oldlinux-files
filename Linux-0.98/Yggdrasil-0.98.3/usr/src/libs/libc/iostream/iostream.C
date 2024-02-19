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
#define _STREAM_COMPAT
#include "ioprivate.h"
#include <iostream.h>
#include <stdio.h>  /* Needed for sprintf */
#include <ctype.h>
#include "floatio.h"

#define	BUF		(MAXEXP+MAXFRACT+1)	/* + decimal point */

//#define isspace(ch) ((ch)==' ' || (ch)=='\t' || (ch)=='\n')

extern backupbuf not_open_filebuf;

istream::istream(streambuf *sb, ostream* tied) : ios(sb, tied)
{
    _flags |= ios::dont_close;
    _gcount = 0;
}

int skip_ws(streambuf* sb)
{
    int ch;
    for (;;) {
	ch = sb->sbumpc();
	if (ch == EOF || !isspace(ch))
	    return ch;
    }
}

istream& istream::get(char& c)
{
    if (ipfx1()) {
	int ch = _strbuf->sbumpc();
	if (ch == EOF) set(ios::eofbit|ios::failbit);
	else c = (char)ch;
    }
    return *this;
}

istream& istream::ignore(int n=1, int delim = EOF)
{
    if (ipfx1()) {
	register streambuf* sb = _strbuf;
	if (delim == EOF) {
	    _gcount = sb->ignore(n);
	    return *this;
	}
	_gcount = 0;
	for (;;) {
#if 0
	    if (n != MAXINT) // FIXME
#endif
	    if (--n < 0)
		break;
	    int ch = sb->sbumpc();
	    if (ch == EOF) {
		set(ios::eofbit|ios::failbit);
		break;
	    }
	    _gcount++;
	    if (ch == delim)
		break;
	}
    }
    return *this;
}

istream& istream::read(char *s, int n)
{
    if (ipfx1()) {
	_gcount = _strbuf->sgetn(s, n);
	if (_gcount != n)
	    set(ios::failbit);
    }
    return *this;
}

istream& istream::seekg(streampos pos)
{
    pos = _strbuf->seekpos(pos, ios::in);
    if (pos == streampos(EOF))
	set(ios::badbit);
    return *this;
}

istream& istream::seekg(streamoff off, _seek_dir dir)
{
    streampos pos = _strbuf->seekoff(off, dir, ios::in);
    if (pos == streampos(EOF))
	set(ios::badbit);
    return *this;
}

streampos istream::tellg()
{
    streampos pos = _strbuf->seekoff(0, ios::cur, ios::in);
    if (pos == streampos(EOF))
	set(ios::badbit);
    return pos;
}

istream& istream::scan(const char *format ...)
{
    if (ipfx0()) {
	va_list ap;
	va_start(ap, format);
	_strbuf->vscan(format, ap, &_state);
	va_end(ap);
    }
    return *this;
}

istream& istream::vscan(const char *format, _G_va_list args)
{
    if (ipfx0())
	_strbuf->vscan(format, args, &_state);
    return *this;
}

istream& operator>>(istream& is, char& c)
{
    if (is.ipfx0()) {
	int ch = is._strbuf->sbumpc();
	if (ch == EOF)
	    is.set(ios::eofbit|ios::failbit);
	else
	    c = (char)ch;
    }
    return is;
}

istream& operator>>(istream& is, char* ptr)
{
    if (is.ipfx0()) {
	register streambuf* sb = is._strbuf;
	int ch = sb->sbumpc();
	if (ch == EOF)
	    is.set(ios::eofbit|ios::failbit);
	else {
	    int w = is.width(0);
	    *ptr++ = ch;
	    for (;;) {
		ch = sb->sbumpc();
		if (ch == EOF) {
		    is.set(ios::eofbit|ios::failbit);
		    break;
		}
		else if (isspace(ch)) {
		    sb->sputbackc(ch);
		    break;
		}
		else if (w == 1) {
		    is.set(ios::failbit);
		    sb->sputbackc(ch);
		    break;
		}
		else *ptr++ = ch;
		w--;
	    }
	}
    }
    *ptr = '\0';
    return is;
}

static int read_int(istream& stream, unsigned long& val, int& neg)
{
    if (!stream.ipfx0())
	return 0;
    register streambuf* sb = stream.rdbuf();
    int base = 10;
    int ndigits = 0;
    register int ch = skip_ws(sb);
    if (ch == EOF)
	goto eof_fail;
    neg = 0;
    if (ch == '+') {
	ch = skip_ws(sb);
    }
    else if (ch == '-') {
	neg = 1;
	ch = skip_ws(sb);
    }
    if (ch == EOF) goto eof_fail;
    if (!(stream.flags() & (ios::hex|ios::dec|ios::oct))) {
	if (ch == '0') {
	    ch = sb->sbumpc();
	    if (ch == EOF) {
		val = 0;
		return 1;
	    }
	    if (ch == 'x' || ch == 'X') {
		base = 16;
		ch = sb->sbumpc();
		if (ch == EOF) goto eof_fail;
	    }
	    else {
		sb->sputbackc(ch);
		base = 8;
		ch = '0';
	    }
	}
    }
    else if (stream.flags() & ios::hex)
	base = 16;
    else if (stream.flags() & ios::oct)
	base = 8;
    val = 0;
    for (;;) {
	if (ch == EOF)
	    break;
	int digit;
	if (ch >= '0' && ch <= '9')
	    digit = ch - '0';
	else if (ch >= 'A' && ch <= 'F')
	    digit = ch - 'A' + 10;
	else if (ch >= 'a' && ch <= 'f')
	    digit = ch - 'a' + 10;
	else
	    digit = 999;
	if (digit >= base) {
	    sb->sputbackc(ch);
	    if (ndigits == 0)
		goto fail;
	    else
		return 1;
	}
	ndigits++;
	val = base * val + digit;
	ch = sb->sbumpc();
    }
    return 1;
  fail:
    stream.set(ios::failbit);
    return 0;
  eof_fail:
    stream.set(ios::failbit|ios::eofbit);
    return 0;
}

#define READ_INT(TYPE) \
istream& operator>>(istream& is, TYPE& i)\
{\
    unsigned long val; int neg;\
    if (read_int(is, val, neg)) {\
	if (neg) val = -val;\
	i = (TYPE)val;\
    }\
    return is;\
}

READ_INT(short)
READ_INT(unsigned short)
READ_INT(int)
READ_INT(unsigned int)
READ_INT(long)
READ_INT(unsigned long)

istream& operator>>(istream& is, double& x)
{
    if (is.ipfx0()) {
	is.scan("%lg", &x);
    }
    return is;
}
istream& operator>>(istream& is, float& x)
{
    if (is.ipfx0()) {
	is.scan("%g", &x);
    }
    return is;
}

istream& operator>>(istream& is, register streambuf* sbuf)
{
    if (is.ipfx0()) {
	register streambuf* inbuf = is.rdbuf();
	// FIXME: Should optimize!
	for (;;) {
	    register int ch = inbuf->sbumpc();
	    if (ch == EOF) {
		is.set(ios::eofbit);
		break;
	    }
	    if (sbuf->sputc(ch) == EOF) {
		is.set(ios::failbit);
		break;
	    }
	}
    }
    return is;
}

ostream& operator<<(ostream& os, char c)
{
    if (os.opfx()) {
	int w = os.width(0);
	char fill_char = os.fill();
	register int padding = w > 0 ? w - 1 : 0;
	register streambuf *sb = os._strbuf;
	if (!(os.flags() & ios::left)) // Default adjustment.
	    while (--padding >= 0) sb->sputc(fill_char);
	sb->sputc(c);
	if (os.flags() & ios::left) // Left adjustment.
	    while (--padding >= 0) sb->sputc(fill_char);
	os.osfx();
    }
    return os;
}

void write_int(ostream& stream, unsigned long val, int neg)
{
    char buf[10 + sizeof(unsigned long) * 3];
    char *show_base = "";
    int show_base_len = 0;
    if ((stream.flags() & (ios::oct|ios::hex)) == 0) // Decimal
	sprintf(buf, "%lu", val);
    else if (stream.flags() & ios::oct) { // Oct
	sprintf(buf, "%lo", val);
	if (stream.flags() & ios::showbase)
	    show_base = "0", show_base_len = 1;
    }
    else if (stream.flags() & ios::uppercase) {// Hex
	sprintf(buf, "%lX", val);
	if (stream.flags() & ios::showbase)
	    show_base = "0X", show_base_len = 2;
    }
    else { // Hex
	sprintf(buf, "%lx", val);
	if (stream.flags() & ios::showbase)
	    show_base = "0x", show_base_len = 2;
    }
    int buf_len = strlen(buf);
    int w = stream.width(0);
    int show_pos = 0;

    // Calculate padding.
    int len = buf_len;
    if (neg) len++;
    else if (val != 0 && (stream.flags() & ios::showpos)) len++, show_pos=1;
    len += show_base_len;
    int padding = len > w ? 0 : w - len;

    // Do actual output.
    register streambuf* sbuf = stream.rdbuf();
    ios::fmtflags pad_kind =
	stream.flags() & (ios::left|ios::right|ios::internal);
    char fill_char = stream.fill();
    if (padding > 0
	&& pad_kind != (ios::fmtflags)ios::left
	&& pad_kind != (ios::fmtflags)ios::internal) // Default (right) adjust.
	sbuf->padn(fill_char, padding);
    if (neg) sbuf->sputc('-');
    else if (show_pos) sbuf->sputc('+');
    if (show_base_len)
	sbuf->sputn(show_base, show_base_len);
    if (pad_kind == (ios::fmtflags)ios::internal && padding > 0)
	sbuf->padn(fill_char, padding);
    sbuf->sputn(buf, buf_len);
    if (pad_kind == (ios::fmtflags)ios::left && padding > 0) // Left adjustment
	sbuf->padn(fill_char, padding);
    stream.osfx();
}

ostream& operator<<(ostream& os, int n)
{
    if (os.opfx()) {
	int neg = 0;
	if (n < 0 && (os.flags() & (ios::oct|ios::hex)) == 0)
	    n = -n, neg = 1;
	write_int(os, n, neg);
    }
    return os;
}

ostream& operator<<(ostream& os, long n)
{
    if (os.opfx()) {
	int neg = 0;
	if (n < 0 && (os.flags() & (ios::oct|ios::hex)) == 0)
	    n = -n, neg = 1;
	write_int(os, n, neg);
    }
    return os;
}

ostream& operator<<(ostream& os, unsigned int n)
{
    if (os.opfx())
	write_int(os, n, 0);
    return os;
}

ostream& operator<<(ostream& os, unsigned long n)
{
    if (os.opfx())
	write_int(os, n, 0);
    return os;
}

ostream& operator<<(ostream& os, float n)
{
    return os << (double)n;
}

ostream& operator<<(ostream& os, double n)
{
    if (os.opfx()) {
	// Uses __cvt_double (renamed from static cvt), in Chris Torek's
	// stdio implementation.  The setup code uses the same logic
	// as in __vsbprintf.C (also based on Torek's code).
	int format_char;
#if 0
	if (os.flags() ios::showpos) sign = '+';
#endif
	if (os.flags() & ios::fixed)
	    format_char = 'f';
	else if (os.flags() & ios::scientific)
	    format_char = os.flags() & ios::uppercase ? 'E' : 'e';
	else
	    format_char = os.flags() & ios::uppercase ? 'G' : 'g';

	int fpprec = 0; // 'Extra' (suppressed) floating precision.
	int prec = os.precision();
	if (prec < 0) prec = 6; // default.
	else if (prec > MAXFRACT) {
	    if (os.flags() & (ios::fixed|ios::scientific) & ios::showpos)
		fpprec = prec - MAXFRACT;
	    prec = MAXFRACT;
	}

	// Do actual conversion.
#ifdef USE_DTOA
	if (__outfloat(n, os.rdbuf(), format_char, os.width(0),
		       os.precision(), os.flags(), 0, os.fill()) < 0)
	    os.set(ios::badbit|ios::failbit); // ??
#else
	int negative;
	char buf[BUF];
	int sign = '\0';
	char *cp = buf;
	*cp = 0;
	int size = __cvt_double(n, os.precision(),
				os.flags() & ios::showpoint ? 0x80 : 0,
				&negative,
				format_char, cp, buf + sizeof(buf));
	if (negative) sign = '-';
	if (*cp == 0)
	    cp++;

	// Calculate padding.
	int fieldsize = size + fpprec;
	if (sign) fieldsize++;
	int padding = 0;
	int w = os.width(0);
	if (fieldsize < w)
	    padding = w - fieldsize;

	// Do actual output.
	register streambuf* sbuf = os.rdbuf();
	register i;
	char fill_char = os.fill();
	ios::fmtflags pad_kind =
	    os.flags() & (ios::left|ios::right|ios::internal);
	if (pad_kind != (ios::fmtflags)ios::left // Default (right) adjust.
	    && pad_kind != (ios::fmtflags)ios::internal)
	    for (i = padding; --i >= 0; ) sbuf->sputc(fill_char);
	if (sign)
	    sbuf->sputc(sign);
	if (pad_kind == (ios::fmtflags)ios::internal)
	    for (i = padding; --i >= 0; ) sbuf->sputc(fill_char);
	
	// Emit the actual concented field, followed by extra zeros.
	sbuf->sputn(cp, size);
	for (i = fpprec; --i >= 0; ) sbuf->sputc('0');

	if (pad_kind == (ios::fmtflags)ios::left) // Left adjustment
	    for (i = padding; --i >= 0; ) sbuf->sputc(fill_char);
#endif
	os.osfx();
    }
    return os;
}

ostream& operator<<(ostream& stream, const char *s)
{
    if (stream.opfx()) {
	int len = strlen(s);
	int w = stream.width(0);
	char fill_char = stream.fill();
	register streambuf *sbuf = stream.rdbuf();
	register int padding = w > len ? w - len : 0;
	if (!(stream.flags() & ios::left)) // Default adjustment.
	    while (--padding >= 0) sbuf->sputc(fill_char);
	sbuf->sputn(s, len);
	if (stream.flags() & ios::left) // Left adjustment.
	    while (--padding >= 0) sbuf->sputc(fill_char);
	stream.osfx();
    }
    return stream;
}

ostream& operator<<(ostream& os, void *p)
{
    if (os.opfx()) {
	os.form("%p", p);
	os.osfx();
    }
    return os;
}

ostream& operator<<(ostream& os, register streambuf* sbuf)
{
    if (os.opfx()) {
	register streambuf* outbuf = os.rdbuf();
	// FIXME: Should optimize!
	for (;;) {
	    register int ch = sbuf->sbumpc();
	    if (ch == EOF) break;
	    if (outbuf->sputc(ch) == EOF) {
		os.set(ios::badbit);
		break;
	    }
	}
	os.osfx();
    }
    return os;
}

ostream::ostream(streambuf* sb, ostream* tied) : ios(sb, tied)
{
    _flags |= ios::dont_close;
}

ostream& ostream::seekp(streampos pos)
{
    pos = _strbuf->seekpos(pos, ios::out);
    if (pos == streampos(EOF))
	set(ios::badbit);
    return *this;
}

ostream& ostream::seekp(streamoff off, _seek_dir dir)
{
    streampos pos = _strbuf->seekoff(off, dir, ios::out);
    if (pos == streampos(EOF))
	set(ios::badbit);
    return *this;
}

streampos ostream::tellp()
{
    streampos pos = _strbuf->seekoff(0, ios::cur, ios::out);
    if (pos == streampos(EOF))
	set(ios::badbit);
    return pos;
}

ostream& ostream::form(const char *format ...)
{
    if (opfx()) {
	va_list ap;
	va_start(ap, format);
	_strbuf->vform(format, ap);
	va_end(ap);
    }
    return *this;
}

ostream& ostream::vform(const char *format, _G_va_list args)
{
    if (opfx())
	_strbuf->vform(format, args);
    return *this;
}

ostream& ostream::flush()
{
    if (_strbuf->sync())
	set(ios::badbit);
    return *this;
}

ostream& flush(ostream& outs)
{
    outs.rdbuf()->overflow(EOF);
    return outs;
}

istream& ws(istream& ins)
{
    if (ins.ipfx1()) {
	int ch = skip_ws(ins._strbuf);
	if (ch == EOF)
	    ins.set(ios::eofbit);
	else
	    ins._strbuf->sputbackc(ch);
    }
    return ins;
}

// Skip white-space.  Return 0 on failure (EOF), or 1 on success.
// Differs from ws() manipulator in that failbit is set on EOF.
// Called by ipfx() and ipfx0() if needed.

int istream::_skip_ws()
{
    int ch = skip_ws(_strbuf);
    if (ch == EOF) {
	set(ios::eofbit|ios::failbit);
	return 0;
    }
    else {
	_strbuf->sputbackc(ch);
	return 1;
    }
}

ostream& ends(ostream& outs)
{
    outs.put(0);
    return outs;
}

ostream& endl(ostream& outs)
{
    return flush(outs.put('\n'));
}

ostream& ostream::write(const char *s, int n)
{
    if (opfx()) {
	if (_strbuf->sputn(s, n) != n)
	    set(ios::failbit);
    }
    return *this;
}

void ostream::do_osfx()
{
    if (flags() & ios::unitbuf)
	flush();
    if (flags() & ios::stdio) {
	fflush(stdout);
	fflush(stderr);
    }
}

const unsigned long ios::basefield = ios::hex|ios::oct|ios::dec;
const unsigned long ios::floatfield = ios::scientific|ios::fixed;
const unsigned long ios::adjustfield =
    ios::left|ios::right|ios::internal;

iostream::iostream(streambuf* sb, ostream* tied) : ios(sb, tied)
{
    _flags |= ios::dont_close;
    _gcount = 0;
}

ostream& iostream::form(const char *format ...) // Copy of ostream::form.
{
    va_list ap;
    va_start(ap, format);
    _strbuf->vform(format, ap);
    va_end(ap);
    return *(ostream*)this;
}
istream& iostream::scan(const char *format ...) // Copy of istream::scan
{
    if (ipfx0()) {
	va_list ap;
	va_start(ap, format);
	_strbuf->vscan(format, ap, &_state);
	va_end(ap);
    }
    return *this;
}

// NOTE: extension for compatibility with old libg++.
// Not really compatible with fistream::close().
#ifdef _STREAM_COMPAT
void ios::close()
{
    if (!(_flags & (unsigned int)ios::dont_close))
	delete _strbuf;
    else if (_strbuf->_flags & _S_IS_FILEBUF)
	((struct filebuf*)_strbuf)->close();
    else if (_strbuf != &not_open_filebuf)
	_strbuf->sync();
    _flags |= ios::dont_close;
    _strbuf = &not_open_filebuf;
}

int istream::skip(int i)
{
    int old = (_flags & ios::skipws) != 0;
    if (i)
	_flags |= ios::skipws;
    else
	_flags &= ~ios::skipws;
    return old;
}
#endif
