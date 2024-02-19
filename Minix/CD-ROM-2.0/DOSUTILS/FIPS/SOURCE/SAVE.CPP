/*
	FIPS - the First nondestructive Interactive Partition Splitting program

	Module save.cpp

	RCS - Header:
	$Header: c:/daten/fips/source/main/RCS/save.cpp 1.4 1995/01/19 00:01:24 schaefer Exp schaefer $

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

#include <stdio.h>
#include <io.h>
#include "global.h"
#include "hdstruct.h"

/* ----------------------------------------------------------------------- */
/* Save root- and boot sector to floppy disk                               */
/* ----------------------------------------------------------------------- */

void save_root_and_boot (harddrive *drive,partition *partition)
{
	FILE *save_file;

	char *filename = "a:\\rootboot.000";

	while (access (filename,0) == 0)
	{
		if (++filename[14] > '9')
			error ("Too many save files on disk");
	}

	if ((save_file = fopen (filename,"wb")) == NULL)
		error ("Can't open file: %s",filename);

	printx ("\nWriting file %s\n", filename);

	if (fwrite (drive->root_sector->data,1,512,save_file) != 512)
		error ("Error writing file: %s",filename);
	if (fwrite (partition->boot_sector->data,1,512,save_file) != 512)
		error ("Error writing file: %s",filename);
	if (fputc (drive->number,save_file) != drive->number)
		error ("Error writing file: %s",filename);
	if (fputc (partition->number,save_file) != partition->number)
		error ("Error writing file: %s",filename);

	if (fclose (save_file))
		error ("Error closing file: %s",filename);
}
