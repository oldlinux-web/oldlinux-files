/*  @(#) whereis.pl 1.0.0 (UvA SWI) Fri Mar  2 14:32:07 1990

    Copyright (c) 1990 Jan Wielemaker. All rights reserved.
    jan@swi.psy.uva.nl

    Purpose: Find predicates
*/

:- module(whereis,
	[ whereis/1
	]).

:- style_check(+dollar).

%	whereis(+Spec)
%	Find predicate definition.

whereis(Name/Arity) :- !,
	whereis(Name, Arity).
whereis(Name) :-
	atom(Name), !,
	whereis(Name, _).
whereis(Head) :-
	functor(Head, Name, Arity),
	whereis(Name, Arity).

whereis(Name, Arity) :-
	(   nonvar(Arity)
	->  functor(Head, Name, Arity)
	;   true
	),
	findall(Module:Head - 0,
		current_predicate(Name, Module:Head),
		Loaded),
	findall(Head - Where,
		find_library_predicate(Name, Head, Where),
		Library),
	append(Loaded, Library, Places),
	sort(Places, Sorted),
	output(Places).

find_library_predicate(Name, Head, Where) :-
	(   nonvar(Head)
	->  functor(Head, Name, Arity)
	;   true
	),
	$find_library(Name, Arity, Where).

output(Places) :-
	

	
