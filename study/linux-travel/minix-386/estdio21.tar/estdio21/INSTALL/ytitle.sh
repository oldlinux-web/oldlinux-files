#!/bin/sh
FILE="$1"
BASENAME=`basename $FILE .c`
NAME=`echo $BASENAME | sed -e 's/[a-zA-Z0-9_]/& /g' -e 's/ *$//'`
BYTES=`echo $NAME | wc -c`
LPAD=`expr \( 74 - $BYTES \) / 2`
RPAD=`expr 74 - $BYTES - $LPAD`
(echo $LPAD $RPAD; echo "$NAME") | awk '
BEGIN		{ line = 0; }
		{ if (line == 0) {
		    lpad = 0 + $1;
		    rpad = 0 + $2;
		    line++;
		    next;
		  }
		  else {
		    printf("/*");
		    for (i = 0; i < lpad; i++)
		      printf(" ");
		    printf("%s", $0);
		    for (i = 0; i < rpad; i++)
		      printf(" ");
		    printf("*/\n");
		    next;
		  }
		}'
