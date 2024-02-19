/*  @(#) portray.pl 1.0.0 (UvA SWI) Tue Feb 27 14:59:38 1990

    Copyright (c) 1990 Jan Wielemaker. All rights reserved.
    jan@swi.psy.uva.nl

    Purpose: portray long strings
*/

portray(S) :-
	is_string(S, L),
	L >= 2,
	(   L > 25
        ->  put('"'),
	    put_first_n(25, S),
	    put('"')
	;   format('"~s"', [S])
	).

put_first_n(0, _) :- !.
put_first_n(N, [C|R]) :-
	put(C),
	NN is N - 1,
	put_first_n(NN, R).

is_string(0) :- !,
	fail.
is_string([], 0) :- !.
is_string([C|R], N) :-
	is_print(C),
	is_string(R, NN),
	N is NN + 1.

is_print(9)  :- !.
is_print(10) :- !.
is_print(C) :-
	number(C),
	between(32, 126, C).
