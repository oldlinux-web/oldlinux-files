/*
	FIPS - the First nondestructive Interactive Partition Splitting program

	Module input.h

	RCS - Header:
	$Header: c:/daten/fips/source/main/RCS/input.h 1.4 1995/01/19 00:01:27 schaefer Exp schaefer $

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

#ifndef INPUT_H
#define INPUT_H

#include "types.h"
#include "hdstruct.h"

int ask_for_drive_number (void);
int ask_for_partition_number (partition_info parts[]);
dword ask_for_new_start_cylinder (int start_cylinder, int min_cylinder,int max_cylinder, int sectors_per_cylinder);
void ask_for_write_permission (void);
boolean ask_if_continue (void);
boolean ask_if_save (void);
void ask_if_proceed (void);
char ask_yes_no (void);
char ask_correction (void);

#endif
