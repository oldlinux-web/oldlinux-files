/*
	FIPS - the First nondestructive Interactive Partition Splitting program

	Module disk_io.cpp

	RCS - Header:
	$Header: c:/daten/fips/source/main/RCS/global.h 1.4 1995/01/19 00:01:25 schaefer Exp schaefer $

	Copyright (C) 1993 Arno Schaefer

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


	Report problems and direct all questions to:

	schaefer@rbg.informatik.th-darmstadt.de
*/

#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>

#include "types.h"
#include "hdstruct.h"
#include "primpart.h"

struct global_vars
{
	boolean test_mode;
	boolean verbose_mode;
	boolean debug_mode;

	int drive_number_cmdline;

	FILE *debugfile;
	void open_debugfile (int argc,char *argv[]);

	global_vars (void);
	~global_vars (void);
};

extern global_vars global;

void printx (char *fmt,...);
int getx (void);
void error (char *message,...);
void warning (boolean wait_key, char *message,...);
void infomsg (char *message,...);

void hexwrite (byte *buffer,int number,FILE *file);

void exit_function (void);
void notice (void);
void evaluate_argument_vector (int argc,char *argv[]);
void save_root_and_boot (harddrive *drive,partition *partition);

#endif
