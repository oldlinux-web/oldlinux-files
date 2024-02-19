ifdef(`comment',`Copyright (C) 1991, 1992 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the GNU C Library; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
')

define(IGNORE,)
define(STARTCOMMENT,`/*')
define(ENDCOMMENT,`*/')
define(INCLUDE,`include('$1`)')

ifdef(`ansi',`
define(PTR,void *)
define(LONG_DOUBLE,long double)

define(AND,`,')
define(NOARGS,void)
define(CONST,const)
define(VOLATILE,volatile)
define(SIGNED,signed)
define(DOTS,`, ...')

define(EXFUN,$1$2)
define(DEFUN,$1($3))
define(DEFUN_VOID,$1(NOARGS))

define(IFANSI,$1)
define(IFNANSI,)
define(IFTRAD,)
define(IFNTRAD,$1)
')

ifdef(`trad',`
define(PTR,char *)
define(LONG_DOUBLE,double)

define(AND,;
)
define(NOARGS,)
define(CONST,)
define(VOLATILE,)
define(SIGNED,)
define(DOTS,)

define(EXFUN,$1())
define(DEFUN,$1$2
$3;)
define(DEFUN_VOID,$1())

define(IFTRAD,$1)
define(IFNTRAD,)
define(IFANSI,)
define(IFNANSI,$1)
')

dnl Undefine all the standard m4 functions.
changequote($, $)
undefine($changequote$)
undefine($regexp$)
undefine($define$)
undefine($divert$)
undefine($divnum$)
dnl undefine($include$)
undefine($dnl$)
undefine($dumpdef$)
undefine($errprint$)
undefine($eval$)
undefine($ifdef$)
undefine($incr$)
undefine($index$)
undefine($len$)
undefine($maketemp$)
undefine($shift$)
undefine($sinclude$)
undefine($substr$)
undefine($syscmd$)
undefine($translit$)
undefine($undivert$)
undefine($unix$)
undefine($ifelse$)
undefine($undefine$)
