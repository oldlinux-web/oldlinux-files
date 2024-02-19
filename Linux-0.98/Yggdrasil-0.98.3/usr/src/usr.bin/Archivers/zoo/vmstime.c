/* vmstime.c */
#ifndef LINT
static char sccsid[]="$Source: /usr/home/dhesi/zoo/RCS/vmstime.c,v $\n\
$Id: vmstime.c,v 1.6 91/07/06 12:20:26 dhesi Exp $";
#endif
/*
This file was graciously supplied by Randal Barnes to support preservation
of file timestamps under VAX/VMS.  I claim no copyright on the contents of
this file.  I assume that neither do its authors.  However, if you adapt
this code for your own use, I recommend preserving author attributions.

                                -- Rahul Dhesi  1991/07/04
*/

/*
*  This module sets a VAX/VMS file's creation and revision date/time to a
*  specified date/time.
*
*  Inputs       Type            Description
*  ------       ----            -----------
*  path         char *          Name of file to be modified
*  date         int             Binary formatted date to be applied to the file
*  time         int             Binary formatted time to be applied to the file
*
*  Outputs      Type            Description
*  -------      ----            -----------
*  Modified file
*
*  Randy Magnuson - (612) 542-5052
*  Randal Barnes  - (612) 542-5021
*  Honeywell Inc. - Military Avionics Division
*  April 12, 1990
*/

#include <stdio.h>
#include <rms.h>
#include <fibdef.h>
#include <atrdef.h>
#include <descrip.h>
#include <iodef.h>
#include <libdtdef.h>

int setutime (char *path, unsigned int date, unsigned int time)
{
   char EName [NAM$C_MAXRSS],   /*  Expanded String Area                */
        RName [NAM$C_MAXRSS],   /*  Resultant String Area               */
        date_str [50];          /*  Holds intermediate ASCII date/time  */

   short iosb [4];              /*  I/O status block for sys calls      */

   int  status,                 /*  Condition code for sys calls, etc.  */
        i,                      /*  Temp index for looping thru arrays  */
        chan,                   /*  Channel to device containing file   */
        Cdate [2],              /*  VMS binary time - creation date     */
        Rdate [2],              /*  VMS binary time - revision date     */
        datetimecontext = 0,    /*  Context for time conv. lib calls    */
        intime = LIB$K_INPUT_FORMAT,  /*  Constant for time lib calls   */
        intdate [2];            /*  VMS binary time - temp              */

   struct FAB Fab;              /*  RMS File Access Block               */
   struct NAM Nam;              /*  RMS Name Block                      */
   static struct fibdef Fib;    /*  RMS File Information Block          */
   struct atrdef Atr [] =       /*  File attribute struct               */
      {
         { sizeof (Cdate), ATR$C_CREDATE, &Cdate [0] }, /*  Creation date  */
         { sizeof (Rdate), ATR$C_REVDATE, &Rdate [0] }, /*  Revision date  */
         {              0,             0,         0 }
      };
   struct dsc$descriptor devnam =       /*  Device name descriptor      */
      { 0, DSC$K_DTYPE_T, DSC$K_CLASS_S, &Nam.nam$t_dvi [1] };
   struct dsc$descriptor FibDesc =      /*  File ID descriptor          */
      { sizeof (Fib), 0, 0, &Fib };
   struct dsc$descriptor_s FileName =   /*  File name descriptor        */
      { 0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0 };

   /*  Time conversion format specification  */
   $DESCRIPTOR (datetimeformat, "|!Y4!MN0!D0|!H04!M0!S0!C2|");

   /*  String descriptor for intermediate date/time string  */
   $DESCRIPTOR (date_desc, date_str);


/*
*  Fill out our File Access Block, Name Block, and Extended Attribute Block so
*  we can parse the file name.
*/
   Fab = cc$rms_fab;
   Nam = cc$rms_nam;

   Fab.fab$l_fna = path;
   Fab.fab$b_fns = strlen (path);
   Fab.fab$l_nam = &Nam;

   Nam.nam$l_esa = &EName;
   Nam.nam$b_ess = sizeof (EName);
   Nam.nam$l_rsa = &RName;
   Nam.nam$b_rss = sizeof (RName);


/*
*  Do a parse and search to fill out the NAM block.
*/
   status = sys$parse(&Fab);
   if (!(status & 1))
      return 0;
   status = sys$search(&Fab);
   if (!(status & 1))
      return 0;


/*
*  Open a channel to the device that the file resides on.
*/
   devnam.dsc$w_length = Nam.nam$t_dvi [0];
   status = SYS$ASSIGN (&devnam, &chan, 0, 0);
   if (!(status & 1))
      return 0;


/*
*  Initialize the FIB
*/
   Fib.fib$r_acctl_overlay.fib$l_acctl = FIB$M_NORECORD;
   for (i = 0; i < 3; i++)
   {
      Fib.fib$r_fid_overlay.fib$w_fid [i] = Nam.nam$w_fid [i];
      Fib.fib$r_did_overlay.fib$w_did [i] = Nam.nam$w_did [i];
   }


/*
*  Set up the file name descriptor for the QIO
*/
   FileName.dsc$a_pointer = Nam.nam$l_name;
   FileName.dsc$w_length = Nam.nam$b_name + Nam.nam$b_type + Nam.nam$b_ver;


/*
*  Use the IO$_ACCESS function to return info about the file.
*/
   status = SYS$QIOW (0, chan, IO$_ACCESS, &iosb, 0, 0,
                      &FibDesc, &FileName, 0, 0, 0, 0);
   if (!(status & 1))
      return 0;
   status = iosb [0];
   if (!(status & 1))
      return 0;


/*
*  Set up a date/time format that we can easily convert to - "YYMMDD HHMMSS"
*/
   status = LIB$INIT_DATE_TIME_CONTEXT (&datetimecontext, &intime,
                                        &datetimeformat);
   if (!(status & 1))
      return 0;


/*
*  Convert the MS-DOS time ints to our ASCII format.
*/
   date_desc.dsc$w_length = sprintf (date_str, "%04d%02d%02d %02d%02d%02d00",
                                      ((date >> 9) & 0x7f) + 1980, /* year */
                                      (date >> 5) & 0x0f,          /* month */
                                      date & 0x1f,                 /* day */
                                      (time >> 11)& 0x1f,          /* hour */
                                      (time >> 5) & 0x3f,          /* min */
                                      (time & 0x1f) * 2);          /* sec */


/*
*  Convert our ASCII formatted date/time to VMS internal time format
*/
   status = LIB$CONVERT_DATE_STRING (&date_desc, &intdate, &datetimecontext);
   if (!(status & 1))
      return 0;


/*
*  Fill in the creation date and revision date fields in the Extended Attribute
*  Block with the date and time from the zoo file.
*/
   Cdate [0] = Rdate [0] = intdate [0];
   Cdate [1] = Rdate [1] = intdate [1];

/*
*  Modify the file
*/
   status = SYS$QIOW (0, chan, IO$_MODIFY, &iosb, 0, 0,
                      &FibDesc, &FileName, 0, 0, &Atr, 0);
   if (!(status & 1))
      return 0;
   status = iosb [0];
   if (!(status & 1))
      return 0;


/*
*  Okee dokee.
*/
   return 1;
}
