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

#ifndef NO_JUMP_TABLE

#include <stdioprivate.h>

struct _iobuf {
    int _cnt;
    unsigned char *_ptr;
    unsigned char *_base;
    int _bufsiz;
    short int _flag;
    char _file;
};

typedef struct {
    int _magic;
    FILE *stream;
} forward_fp;

struct fake_iobuf {
    struct forward_fp forward;
    char dummy[sizeof(struct _iobuf)-sizeof(forward_fp)];
};

fake_iobuf _iob[3] = {
    { _OLD_STDIO_MAGIC, stdin },
    { _OLD_STDIO_MAGIC, stdout },
    { _OLD_STDIO_MAGIC, stderr }
};
#endif
