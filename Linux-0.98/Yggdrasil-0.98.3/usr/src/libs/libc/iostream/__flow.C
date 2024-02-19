//    This is part of the iostream library, providing input/output for C++.
//    Copyright (C) 1991, 1992 Per Bothner.
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
#ifdef __GNUG__
#pragma implementation
#endif
#include "ioprivate.h"
#include <string.h>

#ifdef _G_FRIEND_BUG
int __underflow(register streambuf *sb) { return __UNDERFLOW(sb); }
int __UNDERFLOW(register streambuf *sb)
#else
int __underflow(register streambuf *sb)
#endif
{
    if (sb->put_mode())
        if (sb->switch_to_get_mode() == EOF) return EOF;
    if (sb->_gptr < sb->_egptr)
	return *(unsigned char*)sb->_gptr;
    if (sb->in_backup()) {
	sb->switch_to_main_get_area();
	if (sb->_gptr < sb->_egptr)
	    return *sb->_gptr;
    }
    if (sb->have_markers()) {
	// Append [_gbase.._egptr] to backup area.
	int least_mark = sb->_least_marker();
	// needed_size is how much space we need in the backup area.
	int needed_size = (sb->_egptr - sb->_eback) - least_mark;
	int current_Bsize = sb->_other_egptr - sb->_other_gbase;
	int avail; // Extra space available for future expansion.
	if (needed_size > current_Bsize) {
	    avail = 0; // 100 ?? FIXME
	    char *new_buffer = new char[avail+needed_size];
	    if (least_mark < 0) {
		memcpy(new_buffer + avail,
		       sb->_other_egptr + least_mark,
		       -least_mark);
		memcpy(new_buffer +avail - least_mark,
		       sb->_eback,
		       sb->_egptr - sb->_eback);
	    }
	    else
		memcpy(new_buffer + avail,
		       sb->_eback + least_mark,
		       needed_size);
	    delete [] sb->_other_gbase;
	    sb->_other_gbase = new_buffer;
	    sb->_other_egptr = new_buffer + avail + needed_size;
	}
	else {
	    avail = current_Bsize - needed_size;
	    if (least_mark < 0) {
		memmove(sb->_other_gbase + avail,
			sb->_other_egptr + least_mark,
			-least_mark);
		memcpy(sb->_other_gbase + avail - least_mark,
		       sb->_eback,
		       sb->_egptr - sb->_eback);
	    }
	    else if (needed_size > 0)
		memcpy(sb->_other_gbase + avail,
		       sb->_eback + least_mark,
		       needed_size);
	}
	// FIXME: Dubious arithmetic if pointers are NULL
	sb->_aux_limit = sb->_other_gbase + avail;
	// Adjust all the streammarkers.
	int delta = sb->_egptr - sb->_eback;
	for (register streammarker *mark = sb->_markers;
	     mark != NULL; mark = mark->_next)
	    mark->_pos -= delta;
    }
    else if (sb->have_backup())
	sb->free_backup_area();
    return sb->underflow();
}

#ifdef _G_FRIEND_BUG
int __overflow(register streambuf *sb, int c) { return __OVERFLOW(sb, c); }
int __OVERFLOW(register streambuf *sb, int c)
#else
int __overflow(streambuf* sb, int c)
#endif
{
    return sb->overflow(c);
}
