%!
/ReEncodeForTeX{/newfontname exch def /basefontname exch def /TeXstr 30 string
def /basefontdict basefontname findfont def /newfont basefontdict maxlength
dict def basefontdict{exch dup /FID ne{dup /Encoding eq{exch dup length array
copy newfont 3 1 roll put}{exch newfont 3 1 roll put}ifelse}{pop pop}ifelse}
forall basefontname TeXstr cvs(Dingbat)search{pop pop pop}{pop /TeXvec
basefontname TeXstr cvs(Courier)search{pop pop pop TeXcourvec}{pop
TeXnormalvec}ifelse def TeXvec aload pop TeXvec length 2 idiv{newfont
/Encoding get 3 1 roll put}repeat}ifelse newfontname newfont definefont pop}
def /TeXnormalvec[8#014 /fi 8#015 /fl 8#020 /dotlessi 8#022 /grave 8#023
/acute 8#024 /caron 8#025 /breve 8#026 /macron 8#027 /ring 8#030 /cedilla
8#031 /germandbls 8#032 /ae 8#033 /oe 8#034 /oslash 8#035 /AE 8#036 /OE 8#037
/Oslash 8#042 /quotedblright 8#074 /exclamdown 8#076 /questiondown 8#134
/quotedblleft 8#136 /circumflex 8#137 /dotaccent 8#173 /endash 8#174 /emdash
8#175 /quotedbl 8#177 /dieresis]def /TeXcourvec[8#016 /exclamdown 8#017
/questiondown 8#020 /dotlessi 8#022 /grave 8#023 /acute 8#024 /caron 8#025
/breve 8#026 /macron 8#027 /ring 8#030 /cedilla 8#031 /germandbls 8#032 /ae
8#033 /oe 8#034 /oslash 8#035 /AE 8#036 /OE 8#037 /Oslash 8#074 /less 8#076
/greater 8#134 /backslash 8#136 /circumflex 8#137 /underscore 8#173 /braceleft
8#174 /bar 8#175 /braceright 8#177 /dieresis]def /TeXPSmakefont{/TeXsize exch
def findfont[TeXsize 0 0 TeXsize neg 0 0]makefont}def /ObliqueFont{
/ObliqueAngle exch def /ObliqueBaseName exch def /ObliqueFontName exch def
/ObliqueTransform[1 0 ObliqueAngle sin ObliqueAngle cos div 1 0 0]def
/basefontdict ObliqueBaseName findfont ObliqueTransform makefont def /newfont
basefontdict maxlength dict def basefontdict{exch dup /FID ne{dup /Encoding eq
{exch dup length array copy newfont 3 1 roll put}{exch newfont 3 1 roll put}
ifelse}{pop pop}ifelse}forall newfont /FontName ObliqueFontName put
ObliqueFontName newfont definefont pop}def /Times-Oblique /Times-Roman 15.5
ObliqueFont /Times-BoldOblique /Times-Bold 15 ObliqueFont /Palatino-Oblique
/Palatino-Roman 10 ObliqueFont /Palatino-BoldOblique /Palatino-Bold 10
ObliqueFont /Times-ItalicUnslanted /Times-Italic -15.15 ObliqueFont /pf{
4736286.72 div Resolution mul /ptsize exch def /PSname exch def /TeXname exch
def dmystr 2 TeXname cvx(@@@)cvs putinterval PSname newname ReEncodeForTeX
newname newname ptsize TeXPSmakefont def TeXname{/foo setfont}2 array copy cvx
def TeXname load 0 dmystr 5 string copy cvn cvx put}def
