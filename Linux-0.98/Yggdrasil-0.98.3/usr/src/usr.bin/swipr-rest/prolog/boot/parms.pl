/*  @(#) parms.cpp 1.4.0 (UvA SWI) Mon Jan 1 18:02:00 MET 1990

    Copyright (c) 1990 Jan Wielemaker. All rights reserved.
    jan@swi.psy.uva.nl

    Purpose: Installation dependant parts of the prolog code
*/

:- user:assert(library_directory('.')).
:- user:assert(library_directory(lib)).
:- user:assert(library_directory('~/lib/prolog')).
:- user:assert(library_directory('/usr/prolog/swi-prolog/library')).		% cpp expanded
:- user:assert((
	library_directory(Lib) :-
		'$home'(Home),
		concat(Home, '/library', Lib))).

$vi('/usr/ucb/vi').						% cpp expanded
