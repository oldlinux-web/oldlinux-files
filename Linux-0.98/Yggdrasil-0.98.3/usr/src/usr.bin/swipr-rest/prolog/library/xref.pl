/*  @(#) xref.pl 1.0.0 (UvA SWI) Fri Dec  7 17:48:07 1990

    Copyright (c) 1990 Jan Wielemaker. All rights reserved.
    jan@swi.psy.uva.nl

    Purpose: load xref
*/

% NOTE: Port not yet finished

:- module(xref,
	[ xref_load/1				% load definition file
	, xref/0				% interactive xref loop
	]).

:- use_module(
	[ library('xref.d/xrf')
	, library('xref.d/xrfcol')
	, library('xref.d/xrfdef')
	, library('xref.d/xrfout'),
	, library('xref.d/xrftty')
	]).

:- '$check_file'('xref.d/xref.def', DefFile),
   format('Loading defenitions from ~w ...', DefFile), flush,
   load(DefFil),
   format('done.~n').

