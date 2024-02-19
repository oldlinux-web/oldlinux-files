/* XRFDEF.PL :  Handles .DEFinition files for XREF.

						Dave Bowen
						Updated: 20 November 83

						Jan Wielemaker
						Updated: 10 December 86
*/

/*
* 'system' is renamed in 'sys_pred' because system is a system predicate
* itself.
* 
* removed cut within or in check_width/1 and check_filename/1.
*
* Jan Wielemaker, 7/12/84
*/


/*
:- public load/1.		% User entry
:- public load_file/1.		% This is Called by XRF.PL 

:- mode load(+).
:- mode load_file(+).
:- mode note(+).
:- mode check_width(+).
:- mode check_filename(+).
*/

/******************************************************************************
Data for user definitions.

The following terms may be recorded using a predicate as the key:

	$sys_pred		for built-in predicates.
	$known(Where)	for predicates known to be defined in "Where".
	$applies(P,T)	for predicates P which apply one of their arguments T
	$called		for predicates which are called from other places

The following types of term may be recorded under the key $define:

	width(N)	where N is the required page width for the 
			cross-reference listing.
	title(X)	where X is the title for the listing.
	cross_ref_file(F) where F is the filename for the listing.
	globals_file(G)	where G is the filename for the listing of the imports
			and exports of each file. If G='no' then no such 
			listing will be produced.

**************************************************************************** */


				/* Load in definition file containing system or
				   known predicates, or operators */
load([F|L]) :- !,
	load(F), !,
	load(L).
load([]) :- !.
load(File) :- !,
	seeing(Old),		/* Otherwise we get problems (JW) */
	(   see_chek(File),
	    load_file(File),
	    seen
	|   true
	),
	see(Old).
				/* Load given DEFinition file */
load_file(File) :-
	repeat,
	  read(T),
	(T=end_of_file, !; note(T), fail),
	nl, write('Definition file '), 
	write(File), write(' loaded'),
	nl, nl.

				/* Process terms in definition file. */
note(sys_pred(P)) :- !,
	crecord(P,'$sys_pred').		% P is a system (built-in) predicate
note(known(P,Where)) :- !,
	crecord(P, '$known'(Where)).	% P is known to be defined in Where
note(op(Prec,Assoc,Name)) :- !,
	call(op(Prec,Assoc,Name)).	% operator defns handled as normally
note(applies( _,List )) :-		% handle lists as well (forall etc )
	nonvar( List ),			% Jan Wielemaker
	List = [], !.
note(applies( P,List )) :-
	nonvar( List ), List = [H|T], !,
	note(applies( P,H )),
	note(applies( P,T )), !.
note(applies(P,T)) :-  !,
	recorda(P, '$applies'(P,T),_).	% P must contain T which it Calls
note(called(P,Pred,Arity,File)) :- !,	% P is called by Pred, arity Arity
	caller(P,Pred,Arity,File).	% and file File
note(called(P)) :- !,			% P is called by the user
	caller(P,'<user>',0,undefined).	

				/* The following clauses deal with answers
				   to questions about layout of output etc */
note(width(N)) :- !,
	check_width(N),			% Paper width for cross-ref listing
	recorda('$define',width(N),_).
note(title(T)) :- !,			% Title of cross-ref listing
	recorda('$define',title(T),_).
note(cross_ref_file(F)) :- !, 		% File name for cross-ref listing
	check_filename(F),
	recorda('$define',cross_ref_file(F),_).
note(globals_file(G)) :- !,		% File name for imports/exports listing
	check_filename(G),		% - may be 'no' meaning no listing reqd
	recorda('$define',globals_file(G),_).


% Routines for checking validity of arguments 

				/* Check Width is in range */
check_width( Width ) :-  
	integer( Width ),
	Width >= 50,
	Width =< 150, !.
check_width( Width ) :-
	complain( ['Width (',Width,') must be between 50 and 150'] ).

				/* Check legal file name */
check_filename( F ) :-
	atom( F ), !.
check_filename( F ) :-
	complain( ['Illegal file name ',F] ).

