# Copyright (C) 1991 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License
# as published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

# errno.texinfo contains lines like:
# @comment ###errno: ENOSYS	78	Function not implemented
# These lines must be sorted!

BEGIN {
    print "/* This file is generated from errno.texinfo by sys_errlist.awk. */"
    print "";
    maxerr = 0;
  }
$1 == "@comment" && $2 == "###errno:" {
    n = $4 + 0;
    if (n != 0)
      {
	d = $5;
	for (i = 6; i <= NF; ++i)
	  d = d " " $i;
	desc[n] = d;
	err[n] = $3;
	if (n > maxerr)
	  maxerr = n;
      }
  }
END {
    printf "const int _sys_nerr = %d;\n", maxerr + 1;
    print "";
    print "const char *const _sys_errlist[] =";
    print "  {";
    for (i = 0; i <= maxerr; ++i)
      {
        if (desc[i] == "")
	  printf "    \"Error %d\",\n", i;
	else
	  {
            s = "    \"" desc[i] "\",";
	    l = 50 - length (s);
	    while (l-- > 0)
	      s = s " ";
	    printf "%s/* %s %d */\n", s, err[i], i;
	  }
      }
    print "  };";
  }
