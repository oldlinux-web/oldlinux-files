/* dbm.h  -  The include file for dbm users.  */

/*  This file is part of GDBM, the GNU data base manager, by Philip A. Nelson.
    Copyright (C) 1990, 1991  Free Software Foundation, Inc.

    GDBM is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    GDBM is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GDBM; see the file COPYING.  If not, write to
    the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

    You may contact the author by:
       e-mail:  phil@cs.wwu.edu
      us-mail:  Philip A. Nelson
                Computer Science Department
                Western Washington University
                Bellingham, WA 98226
        phone:  (206) 676-3035
       
*************************************************************************/
#ifndef _DBM_H 
#define _DBM_H

/* The data and key structure.  This structure is defined for compatibility. */
typedef struct {
	char *dptr;
	int   dsize;
      } datum;


/* These are the routines in dbm. */


#ifdef __STDC__

#ifdef __cplusplus
extern "C" {
#endif

extern int dbminit(const char *file);
extern int store(datum key, datum content);
extern int delete(datum key);
extern datum fetch(datum);
extern datum firstkey(void);
extern datum nextkey(datum);

#ifdef __cplusplus
}
#endif

#else /* not __STDC__ */

extern int	dbminit ();
extern int	store ();
extern int	delete ();
extern datum fetch();
extern datum firstkey();
extern datum nextkey();

#endif

/* To make some versions work we need the following define. */

#define dbmclose()

#endif /* _DBM_H */
