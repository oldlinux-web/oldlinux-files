/* @(#) errors.i 2.4 88/01/31 12:32:46 */

/*
The contents of this file are hereby released to the public domain.

                           -- Rahul Dhesi 1986/11/14
*/

/* defines all the errors as externs.  Declarations must be
equivalent to those in prterror.c */

/* These declarations must be equivalent to those in prterror.c */
extern char no_match[];
extern char failed_consistency[];
extern char invalid_header[];
extern char internal_error[];
extern char disk_full[];
extern char bad_directory[];
extern char no_memory[];
extern char too_many_files[];
extern char packfirst[];
extern char garbled[];
extern char start_ofs[];

#ifndef OOZ
extern char wrong_version[];
extern char cant_process[];
extern char option_ignored[];
extern char inv_option[];
extern char bad_crc[];
#endif

extern char could_not_open[];

