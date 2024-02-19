/*  DEC/CMS REPLACEMENT HISTORY, Element VMS_TYPES.H */
/*  *4    13-AUG-1992 12:49:50 TP "Added IOSB_GET_T" */
/*  *3    10-AUG-1992 14:20:29 TP "MOVE" */
/*  *2    10-AUG-1992 14:19:45 TP "MOVE" */
/*  *1    10-AUG-1992 14:19:31 TP "VMS data type definitions and macros" */
/*  DEC/CMS REPLACEMENT HISTORY, Element VMS_TYPES.H */
/* VMS_TYPES.H
**=============================================================================
** Copyright (C) 1989 Jym Dyer (jym@wheaties.ai.mit.edu)
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 1, or (at your option)
** any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**-----------------------------------------------------------------------------
** Version:	V1.0-001
**-----------------------------------------------------------------------------
** Facility:	None
**-----------------------------------------------------------------------------
** Prefix:	None
**-----------------------------------------------------------------------------
** Abstract
** ~~~~~~~~
**  These are typedefs and macro functions for various VMS data types.
**-----------------------------------------------------------------------------
** Contents
** ~~~~~~~~
** EXIT_BLOCK_T
** IOSB_T
** IOSB_ACP_T
** IOSB_CR_T
** IOSB_DISK_T
** IOSB_DISK_SENSEMODE_T
** IOSB_LPA_T
** IOSB_LP_WRITE_T
** IOSB_LP_SETMODE_T
** IOSB_MBX_READ_T
** IOSB_MBX_WRITE_T
** IOSB_MBX_SETPROTECTION_T
** IOSB_MT_T
** IOSB_TTY_ITEMLIST_READ_T
** IOSB_TTY_READ_T
** IOSB_TTY_SETSENSE_T
** IOSB_TTY_WRITE_T
** ITEM_2_T
** ITEM_3_T
** ITEM_LIST_2_T()
** ITEM_LIST_3_T()
**-----------------------------------------------------------------------------
** Environment
** ~~~~~~~~~~~
**  Should be portable to any compiler running on VMS.
**-----------------------------------------------------------------------------
** Author:	Jym Dyer - 15-May-1989
**-----------------------------------------------------------------------------
** Modifications
** ~~~~~~~~~~~~~
** 1.0-001 - Original version.  {Jym 15-May-1989}
** 1.0-002 - Added IOSB_GET_T {Terry Poot <tp@mccall.com> 8/10/1992}
**=============================================================================
*/

#ifndef __VMS_TYPES_H__
#define __VMS_TYPES_H__

/* -=- MACRO FUNCTIONS AND TYPEDEFS -=- */

/* --- Exit Handler Block --- */

/* The exit handler block is a variable-length structure.  What we provide
** here is a header for that structure.  For the simplest uses (exit
** handlers that don't take arguments) the typedef alone will suffice:
**
**	extern void  exit_function(unsigned int *  status_p);
**	unsigned int  exh_status;
**		. . .
**	  EXIT_BLOCK_T  exit_block =
**	   {NULL,exit_function,0,{0,0,0},&exh_status};
**		. . .
**	void
**	exit_function(status_p)
**	 unsigned int *  status_p;
**	{
**		. . .
**
** For more complicated uses (when you want to pass several arguments to
** the exit handler) the typedef can be used as a header in a structure:
**
**	extern void  exit_function(
**	  unsigned int *  status_p,int *  yin_p,int *  yang_p
**	);
**	unsigned int  exh_status;
**	int  that;
**	int  this;
**		. . .
**	  struct
**	  {
**	    EXIT_BLOCK_T  header;
**	    int *  this_p;
**	    int *  that_p;
**	  } = {{NULL,exit_function,0,{0,0,0},&exh_status},&this,&that};
**		. . .
**	void
**	exit_function(status_p,yin_p,yang_p)
**	 unsigned int *  status_p;
**	 int *  yin_p;
**	 int *  yang_p;
**	{
**		. . .
*/

typedef struct
{
  void *  flink_p;
  void  (*exit_handler_p)();
  unsigned char  arg_count;
  unsigned char  must_be_zero[3];
  unsigned int *  status_p;
}  EXIT_BLOCK_T;

/* --- All-Purpose IOSB --- */

/* This all-purpose IOSB can be used for any IO function (though it could
** be a bit of a hassle with terminal set and sense functions).  Just be
** careful with the device dependent data, remembering to use casts where
** appropriate.  Use of the other IOSB typedefs is recommended over use of
** this one, as their fields have more relevant names.
*/

typedef struct
{
  unsigned short int  status;
  unsigned short int  count;
  unsigned char  device_dependent_data[4];
}  IOSB_T;    

/* --- Ancillary Control Process (ACP) IOSB --- */

typedef struct
{
  unsigned short int  status;
  unsigned short int  not_used_0;
  unsigned long int  not_used_1;
}  IOSB_ACP_T;

/* --- CR11 Card Reader IOSB --- */

/* Identical to the all-purpose IOSB.
*/

#define IOSB_CR_T IOSB_T

/* --- Disk Device IOSBs --- */

/* IOSB_DISK_T is for all disk device functions except for sense mode;
** IOSB_DISK_SENSEMODE_T is for sense mode.
*/

typedef struct
{
  unsigned short int  status;
  unsigned short int  byte_count_low_order;
  unsigned short int  byte_count_high_order;
  unsigned short int  zero;
}  IOSB_DISK_T;

typedef struct
{
  unsigned short int  status;
  unsigned short int  zero;
  unsigned char  sectors;
  unsigned char  tracks;
  unsigned short int  cylinders;
}  IOSB_DISK_SENSEMODE_T;

/* --- Laboratory Peripheral Accelarator (LPA) IOSB --- */

typedef struct
{
  unsigned short int  status;
  unsigned short int  byte_count;
  unsigned short int  ready_out;
  unsigned short int  maintenance_status;
}  IOSB_LPA_T;

/* --- Line Printer IOSBs --- */

/* IOSB_LP_WRITE_T is for write functions; IOSB_LP_SETMODE_T is for
** set mode functions.  IOSB_LP_SETMODE_T is identical to IOSB_ACP_T.
*/

typedef struct
{
  unsigned short int  status;
  unsigned short int  byte_count;
  unsigned long int  num_lines_paper_moved;
}  IOSB_LP_WRITE_T;

#define IOSB_LP_SETMODE_T IOSB_ACP_T

/* --- Magnetic Tape IOSB --- */

/* Identical to the all-purpose IOSB.
*/

#define IOSB_MT_T IOSB_T

/* --- Mailbox (MBX) IOSBs --- */

/* IOSB_MBX_READ_T is for the read function; IOSB_MBX_WRITE_T
** is for the write function; IOSB_MBX_SETPROTECTION_T is for
** the set protection function.
*/

typedef struct
{
  unsigned short int  status;
  unsigned short int  byte_count;
  unsigned long int  sender_pid;
}  IOSB_MBX_READ_T;

typedef struct
{
  unsigned short int  status;
  unsigned short int  byte_count;
  unsigned long int  receiver_pid;
}  IOSB_MBX_WRITE_T;

typedef struct
{
  unsigned short int  status;
  unsigned short int  zero;
  unsigned long int  protection_mask_value;
}  IOSB_MBX_SETPROTECTION_T;

/* --- Terminal (TTY) IOSBs --- */

/* IOSB_TTY_READ_T is for the read function; IOSB_TTY_ITEMLIST_READ_T
** is for the itemlist read function; IOSB_TTY_WRITE_T is for the
** write function; IOSB_TTY_SETSENSE_T is for the set mode, set
** characteristscs, sense mode, and sense characteristics functions.
*/

typedef struct
{
  unsigned short int  status;
  unsigned short int  offset_to_terminator;
  unsigned short int  terminator;
  unsigned short int  terminator_size;
}  IOSB_TTY_READ_T;

typedef struct
{
  unsigned short int  status;
  unsigned short int  offset_to_terminator;
  unsigned char  terminator_character;
  unsigned char  reserved;
  unsigned char  terminator_length;
  unsigned char  cursor_position_from_eol;
}  IOSB_TTY_ITEMLIST_READ_T;

typedef struct
{
  unsigned short int  status;
  unsigned short int  byte_count;
  unsigned short int  zero_0;
  unsigned short int  zero_1;
}  IOSB_TTY_WRITE_T;

typedef struct
{
  unsigned short int  status;
  unsigned char  transmit_speed;
  unsigned char  receive_speed;
  unsigned char  cr_fill_count;
  unsigned char  lf_fill_count;
  unsigned char  parity_flags;
  unsigned char  zero;
}  IOSB_TTY_SETSENSE_T;

/* Many of the VMS GETxxx system services also use IOSB's, but they are laid
** out differently. IOSB_GET_T is such a structure. The first longword (not
** word) is the status code, and the second word is reserved to DEC.
*/ 

typedef struct
{
  unsigned long  int  status;
  unsigned long  int  reserved;
}  IOSB_GET_T;    

/* --- Item Lists --- */

/* The item list structures change dynamically according to the number
** of items in them.  For this reason, typedefs (ITEM_2_T and ITEM_3_T)
** are provided for the items, and macro functions (ITEM_LIST_2_T() and
** ITEM_LIST_3_T()) are provided for the item lists.  Here is an example
** showing the usage of an item list macro function:
**
**	static const ITEM_LIST_3_T(item_list,2) =
**	{
**	  {
**	    {sizeof pid,JPI$_PID,&pid,NULL},
**	    {sizeof username,JPI$_USERNAME,&username,&username_length}
**	  },
**	  0
**	};
**
** The number 2 means, of course, that there are two items in the
** itemlist (i.e., the PID and the username).
*/

typedef struct
{
  unsigned short int  component_size;
  unsigned short int  item_code;
  void *  component_p;
}  ITEM_2_T;

typedef struct
{
  unsigned short int  buffer_size;
  unsigned short int  item_code;
  void *  buffer_p;
  unsigned short int *  buffer_length_p;
}  ITEM_3_T;

#define ITEM_LIST_2_T(variable_name,num_items) \
  struct \
  { \
    ITEM_2_T  item[num_items]; \
    int  terminating_zero; \
  }  variable_name

#define ITEM_LIST_3_T(variable_name,num_items) \
  struct \
  { \
    ITEM_3_T  item[num_items]; \
    int  terminating_zero; \
  }  variable_name

#endif /* !__VMS_TYPES_H__ */
