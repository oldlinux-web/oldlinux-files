/* XRFTTY.PL :  Terminal interaction for XREF

						Dave Bowen
						Updated: 20 November 83

						Jan Wielemaker
						Updated: 11 December 84
*/
/*
 :- public member/2.				% this is Called

 :- mode actionchar(+,+,-).
 :- mode append(?,?,?).
 :- mode complain(+).
 :- mode get_crf(-,-).
 :- mode get_globals_file(-,-).
 :- mode get_title(-).
 :- mode get_width(-).
 :- mode member(?,+).  
 :- mode readtonl(-).  
 :- mode readtonl0(-).
 :- mode xref_readln(+,-).
 :- mode see_chek(+).
 :- mode tell_chek(+).
 :- mode warn(+,+).
 :- mode writepred(+,+).
 :- mode writes(+).
 :- mode yesno(+,?).
*/

				/* Get name for cross-refs file: instantiate
				   both File and Chars */

get_crf(File,Chars) :- 				%  Name specified in DEF file?
	recorded('$define', cross_ref_file(File), Ref), !,
	erase(Ref),				%  Yes, remove from database
	name(File,Chars),
	recorda($crf,file(File,Chars),_).	%  save
get_crf(File,Chars) :-
	repeat, nl,
	write('Filename for Cross-Reference listing: '),
	ttyflush, 
	readtonl(Chars),			% read file name from terminal
	name(File,Chars),
	check_filename(File),			%  Check name is appropriate
	recorda($crf,file(File,Chars),_).	%  save 

				/* Get required page width */
get_width(W) :-
 	recorded('$define',width(W),Ref), !,	% Was it specified in DEF file?
 	erase(Ref).
get_width(W) :-					% No, prompt for it.
	telling(Old,user),
 	repeat,
	  nl, write('Width: '),
   	  ttyflush, readtonl(Chars),
	  name(W,Chars),
	check_width(W), !,
	tell(Old).

				/* Get title for Cross-Reference listing */
get_title(T) :-
	recorded('$define',title(T),Ref), !,	% Specified in a DEF file?
	erase(Ref).
get_title(T) :-					% Not in DEF file, ask for it.
	telling(Old,user),
	nl, write('Title: '),
	ttyflush, readtonl0(T),
	tell(Old).

				/* Gets filename (or "no") for import/export
				   lists. Instantiates both File and Chars */

get_globals_file(File,Chars) :-			% Specified in DEF file?
	recorded('$define',globals_file(File),Ref),
	erase(Ref),
	(File=no, !, fail ; name(File,Chars)).
get_globals_file(File,Chars) :-			% No, ask for it.
	yesno('Do you want a listing of imports/exports',yes),
	repeat,					% Try again if unable to open
   	    nl,					%	file for output.
	    write('Filename for imports/exports: '),
	    ttyflush, 
	    readtonl(Chars),
	    name(File,Chars).

% Utilities for input/output 

/* made comment by Jan Wielemaker, 7/12/'84
member(X,[X|_]).
member(X,[_|L]) :- member(X,L).

append([],X,X).
append([A|B],C,[A|D]) :- append(B,C,D).
*/

complain(L) :- 
	nl, write('% '),
	(member(Text,L), write(Text); nl), fail.

yesno(Question,Answer) :-
	nl, write(Question),
	write('? '), ttyflush,
	repeat,
	   readtonl(Ans),
	   (Ans=[C|_], 
		(C=:="y", !, Answer=yes;
		 C=:="n", !, Answer=no);
	    complain(['Answer y(es) or n(o)'])).

			/* Give a warning about a predicate */
warn(P,State) :-
	telling(File), tell(user),
	warning(P,State),
	warn_on_crf(P,State),
	tell(File).

warn_on_crf(P,State) :-
	recorded($crf,file(Crf,_),_),
	tell(Crf),
	warning(P,State).
warn_on_crf(_,_).	%   don't fail if this file doesn't exist.

warning(P,State) :-
	write('** WARNING: '),
	functor(P,F,N),
	write(F/N is State), nl.
	

writepred('<user>',0) :- !, write('<user>').
writepred(F,N) :- writeq(F/N).


				/* See file or complain if it doesn't exist */
see_chek(File) :-
	( nofileerrors  ;  fileerrors, complain([File, ' not found']) ),
	(   see(File)
	|   concat(File, '.pl', PlFile),
	    see(PlFile)
	), !,
	fileerrors.

				/* Open file for output or complain */
tell_chek('tty:') :- !, tell(user).
tell_chek(File) :-
	( nofileerrors  ;  fileerrors, complain(['Can''t open ', File]) ),
	tell(File), !, fileerrors.

% Low level input routines


				/* Read a line, returning character list */
readtonl(Cs) :-
	xref_readln(ignore_blanks,Cs).		% - for reading file names
readtonl0(Cs) :-
	xref_readln(keep_blanks,Cs).			% - for reading text

xref_readln(Flag,Cs) :- 
	get0(C),
	actionchar(Flag,C,Cs).

actionchar(_,10,[]) :- !.			% newline: return []
actionchar(_,26,_) :- !, fail.			% ^Z: fail
actionchar(ignore_blanks,C,Cs) :-		% ignore layout characters
	C=<32, !, 				%  (incl. space) if reqd
	xref_readln(ignore_blanks,Cs).
actionchar(Flag,C,[C|Crest]) :- !,		% other: construct list
	xref_readln(Flag,Crest).

%	Strip path from filename. i.e. /usr/local/lib/prolog/applic becomes
%	applic.

strippath(Path,Fn) :-
	recorded('$strip',strip(Path,Fn),_), !.	% once before computed

strippath(Path,Fn) :-
	mname(Path,PathL),
	strippath(PathL,Hole ,Hole,FnL),
	mname(Fn,FnL),
	recorda('$strip',strip(Path,Fn),_), !.	% for later use

strippath([],F,[],F).
strippath([47|R],_,Hole,F):- strippath(R,Hole,Hole,F).
strippath([X|R],Sofar,[X|Hole],F):- strippath(R,Sofar,Hole,F).

mname(At ,[]):- var(At),!,fail.
mname(At,Lst):- name(At,Lst).
