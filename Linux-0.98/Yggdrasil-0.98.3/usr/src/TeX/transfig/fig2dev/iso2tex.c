/*
 * TransFig: Facility for Translating Fig code
 * Copyright (c) 1991 Micah Beck, Cornell University
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Cornell University not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Cornell University makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * CORNELL UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL CORNELL UNIVERSITY BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

/* map ISO-Font Symbols to appropriate sequences in TeX */
/* Herbert Bauer 22.11.1991 */

char *ISOtoTeX[] =   /* starts at octal 240 */
{
  "{}",
  "!`{}",	/* inverse ! */
  "{}",		/* cent sign (?) */
  "\\pounds{}",
  "{}",		/* circle with x mark */
  "{}",		/* Yen */
  "{}",		/* some sort of space - doen't work under mwm */
  "\\S ",	/* paragraph sign */
  "{}",		/* diaresis points */
  "\\copyright{}",
  "\\b{a}",
  "\\mbox{$\\ll$}",		/* << */
  "--", 	/* longer dash - doesn't work with mwm */
  "-",		/* short dash */
  "{}",		/* trademark */
  "{}",		/* overscore */
  "{}",		/* degree */
  "\\mbox{$\\pm$}",	/* plus minus - math mode */
  "^2",		/* squared  - math mode */
  "^3",		/* cubed  - math mode */
  "{}",		/* accent egue */
  "\\mbox{$\\mu$}",	/* greek letter mu - math mode */
  "\\P ",	/* paragraph */
  "\\mbox{$\\cdot$}",	/* centered dot  - math mode */
  "",
  "^1",		/* superscript 1  - math mode */
  "\\b{o}",
  "\\mbox{$\\gg$}",		/* >> */
  "\\mbox{$1\\over 4$}",	/* 1/4 - math mode */
  "\\mbox{$1\\over 2$}",	/* 1/2 - math mode */
  "\\mbox{$3\\over 4$}",	/* 3/4 - math mode */
  "?`{}",		/* inverse ? */
  "\\`A{}",
  "\\'A{}",
  "\\^A{}",
  "\\~A{}",
  "\\\"A{}",
  "\\AA{}",
  "\\AE{}",
  "\\c{C}",
  "\\`E{}",
  "\\'E{}",
  "\\^E{}",
  "\\\"E{}",
  "\\`I{}",
  "\\'I{}",
  "\\^I{}",
  "\\\"I{}",
  "{}",		/* combination of D and - */
  "\\i~N{}",
  "\\`O{}",
  "\\'O{}",
  "\\^O{}",
  "\\~O{}",
  "\\\"O{}",
  "\\mbox{$\\times$}",	/* math mode */
  "\\O{}",
  "\\`U{}",
  "\\'U{}",
  "\\^U{}",
  "\\\"U{}",
  "\\'Y{}",
  "{}",		/* letter P wide-spaced */
  "\\ss{}",
  "\\`a{}",
  "\\'a{}",
  "\\^a{}",
  "\\~a{}",
  "\\\"a{}",
  "\\aa{}",
  "\\ae{}",
  "\\c{c}",
  "\\`e{}",
  "\\'e{}",
  "\\^e{}",
  "\\\"e{}",
  "\\`i{}",
  "\\'i{}",
  "\\^i{}",
  "\\\"i{}",
  "\\mbox{$\\partial$}",	/* correct?  - math mode */
  "\\~n{}",
  "\\`o{}",
  "\\'o{}",
  "\\^o{}",
  "\\~o{}",
  "\\\"o{}",
  "\\mbox{$\\div$}",	/* math mode */
  "\\o{}",
  "\\`u{}",
  "\\'u{}",
  "\\^u{}",
  "\\\"u{}",
  "\\'y{}",
  "{}",		/* letter p wide-spaced */
  "\\\"y{}"
};

