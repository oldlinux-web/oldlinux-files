/*
    FIPS - the First nondestructive Interactive Partition Splitting program
    Module restorrb.c

    Copyright (C) 1993 Arno Schaefer

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <dos.h>
#include <bios.h>
#include <alloc.h>
#include <conio.h>
#include <ctype.h>

#include "rtypes.h"
#include "rversion.h"

#define DISK_INT 0x13

#define RESET_DISK 0
#define WRITE_SECTOR 3
#define VERIFY_SECTOR 4

#define DISK1 0x80

/* ----------------------------------------------------------------------- */
/* Copyright notice and version number                                     */
/* ----------------------------------------------------------------------- */

void notice (void)
{
	printf ("\nFIPS version " FIPS_VERSION ", Copyright (C) 1993/94 Arno Schaefer\n");
	printf ("Module RESTORRB.EXE - Please read the file README.1ST\n");
	printf ("FIPS comes with ABSOLUTELY NO WARRANTY, see file COPYING for details\n");
	printf ("This is free software, and you are welcome to redistribute it\n");
	printf ("under certain conditions; again see file COPYING for details.\n\n");
}

/* ----------------------------------------------------------------------- */
/* Error Handling                                                          */
/* ----------------------------------------------------------------------- */

int getx (void)
{
	int character = getch();

	if (character == 3)
	{
		printf ("\n");
		exit (0);
	}
	return (character);
}

void error (char *message)
{
	fprintf (stderr,"\nError: %s!\n",message);
	exit (-1);
}

/* ----------------------------------------------------------------------- */
/* BIOS calls                                                              */
/* ----------------------------------------------------------------------- */

int reset_drives (void)
{
	union REGS regs;

	regs.h.ah = RESET_DISK;
	regs.h.dl = DISK1;
	int86 (DISK_INT,&regs,&regs);
	if (regs.x.cflag) return (-1);
	return 0;
}

/* ----------------------------------------------------------------------- */
/* read / write sectors                                                    */
/* ----------------------------------------------------------------------- */

int verify_sector (int drive_number,dword head,dword cylinder,dword sector,byte *buffer)
{
	if (biosdisk (VERIFY_SECTOR,drive_number,head,cylinder,sector,1,buffer)) return (-1);
	return 0;
}

int write_sector (int drive_number,dword head,dword cylinder,dword sector,byte *buffer)
{
	int i;
	boolean done=false;
	for (i=0;i<3;i++)
	{
		if (!biosdisk (WRITE_SECTOR,drive_number,head,cylinder,sector,1,buffer))
		{
			done=true;
			break;
		}
		reset_drives();
	}
	if (!done) return (-1);
	return (verify_sector (drive_number,head,cylinder,sector,buffer));
}

int write_root_sector (int drive_number,byte *buffer)
{
	return (write_sector (drive_number,0,0,1,buffer));
}

/* ----------------------------------------------------------------------- */
/* User Input                                                              */
/* ----------------------------------------------------------------------- */

void ask_for_write_permission (char *filename)
{
	int character = 'x';

	printf ("\nReady to write old root- and bootsector from file %s to disk\n", filename);
	printf ("Do you want to proceed (y/n): ");

	while ((character != 'y') && (character != 'n')) character = getx();
	printf ("%c\n",character);
	if (character == 'n') exit (0);
}

/* ----------------------------------------------------------------------- */
/* Main                                                                    */
/* ----------------------------------------------------------------------- */

void main (void)
{
	byte rootsector[512];
	byte bootsector[512];
	int drive_number,partition_number,i;
	FILE *handle;
	dword head,cylinder,sector;
	char *filename = "a:\\rootboot.000";
	int no_of_savefiles = 0;
	char first = 'x';
	char list[10];

	notice();

	if (reset_drives ()) error ("Drive Initialization Failure");

	for (i='0';i<='9';i++)
	{
		filename[14] = i;
		if (access (filename,0) == 0)
		{
			if (first == 'x') first = i;
			list[no_of_savefiles++] = i;
			printf ("Found save file %s\n",filename);
		}
	}

	if (no_of_savefiles == 0) error ("No savefile ROOTBOOT.00? found on disk A:");

	if (no_of_savefiles > 1)
	{
		printf ("\nWhich file do you want to restore (");
		for (i = 0; i < no_of_savefiles; i++)
		{
			printf ("%c/", list[i]);
		}
		printf ("\b)? ");

		while (true)
		{
			int c;
			if (isdigit (c = getx()))
			{
				boolean found = false;

				for (i = 0; i < no_of_savefiles; i++)
				{
					if (c == list[i]) found = true;
				}

				if (found)
				{
					printf ("%c\n", c);
					filename[14] = c;
					break;
				}
			}
		}
	}
	else
	{
		filename[14] = first;
	}

	if ((handle = fopen (filename,"rb")) == NULL)
		error ("Can't open file");

	for (i=0;i<512;i++)
	{
		int character = fgetc (handle);
		if (character == EOF) error ("Error reading file from disk");
		*(rootsector + i) = character;
	}
	for (i=0;i<512;i++)
	{
		int character = fgetc (handle);
		if (character == EOF) error ("Error reading file from disk");
		*(bootsector + i) = character;
	}
	if ((drive_number = fgetc (handle)) == EOF) error ("Error reading file from disk");
	if ((partition_number = fgetc (handle)) == EOF) error ("Error reading file from disk");
	if (fclose (handle)) error ("Error closing file");

	head = (dword) rootsector[0x1be+16*partition_number+1];
	cylinder = (((dword) rootsector[0x1be+16*partition_number+2] << 2) & 0x300)
		| (dword) rootsector[0x1be+16*partition_number+3];
	sector = (dword) rootsector[0x1be+16*partition_number+2] & 0x3f;

	ask_for_write_permission(filename);

	if (write_root_sector (drive_number,rootsector))
		error ("Error writing rootsector");

	if (write_sector (drive_number,head,cylinder,sector,bootsector))
		error ("Error writing bootsector");
}
