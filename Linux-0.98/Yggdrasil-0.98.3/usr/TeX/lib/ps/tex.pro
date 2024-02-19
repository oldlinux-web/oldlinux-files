%!
/GlSave save def /TeXDict 200 dict def TeXDict begin /Resolution 300 def /Inch
{Resolution mul}def /Mtrx 6 array def /letter where{pop}{/letter{}def}ifelse
/legal where{pop}{/legal{}def}ifelse /note where{pop}{/note{}def}ifelse
/@letter{72 Resolution div dup neg scale 310 -3005 translate Mtrx
currentmatrix pop}def /@landscape{letter initmatrix 72 Resolution div dup neg
scale Mtrx currentmatrix 0 0.0 put Mtrx 1 -1.0 put Mtrx 2 1.0 put Mtrx 3 0.0
put Mtrx setmatrix 300 310 translate Mtrx currentmatrix pop}def /@legal{legal
initmatrix 72 Resolution div dup neg scale 295 -3880 translate Mtrx
currentmatrix pop}def /@copies{/#copies exch def}def /@restore /restore load
def /restore{vmstatus pop dup @VMused lt{pop @VMused}if exch pop exch @restore
/@VMused exch def}def /@pri{( )print(                                       )
cvs print}def /@FontMatrix[1 0 0 -1 0 0]def /@FontBBox[0 0 1 1]def /dmystr
(ZZf@@)def /newname{dmystr cvn}def /df{/fontname exch def dmystr 2 fontname
cvx(@@@)cvs putinterval newname 7 dict def newname load begin /FontType 3 def
/FontMatrix @FontMatrix def /FontBBox @FontBBox def /BitMaps 256 array def
/BuildChar{CharBuilder}def /Encoding 256 array def 0 1 255{Encoding exch
/.undef put}for end fontname{/foo setfont}2 array copy cvx def fontname load 0
dmystr 5 string copy cvn cvx put}def /dfend{newname newname load definefont
setfont}def /ch-image{ch-data 0 get}def /ch-width{ch-data 1 get}def /ch-height
{ch-data 2 get}def /ch-xoff{ch-data 3 get}def /ch-yoff{ch-data 4 get}def
/ch-tfmw{ch-data 5 get}def /CharBuilder{save 3 1 roll exch /BitMaps get exch
get /ch-data exch def ch-data null ne{ch-tfmw 0 ch-xoff neg ch-yoff neg
ch-width ch-xoff sub ch-height ch-yoff sub setcachedevice ch-width ch-height
true[1 0 0 1 ch-xoff ch-yoff]{ch-image}imagemask}if restore}def /dc{/ch-code
exch def dup 0 get length 2 lt{pop[<00>1 1 0 0 8.00]}if /ch-data exch def
newname load /BitMaps get ch-code ch-data put newname load /Encoding get
ch-code dup(   )cvs cvn put}def /bop{erasepage initgraphics Mtrx setmatrix
/SaveImage save def 0 0 moveto}def /eop{showpage SaveImage restore}def /@start
{@letter vmstatus pop /@VMused exch def pop}def /@end{(VM used: )print @VMused
@pri(. Unused: )print vmstatus @VMused sub @pri pop pop(\n)print flush end
GlSave restore}def /p{show}def /v{/dy exch neg def /dx exch def /x1
currentpoint /y1 exch def def newpath x1 y1 moveto dx 0 rlineto 0 dy rlineto
dx neg 0 rlineto closepath fill x1 y1 moveto}def /li{lineto}def /rl{rlineto}
def /rc{rcurveto}def /np{/SaveX currentpoint /SaveY exch def def newpath}def
/st{stroke SaveX SaveY moveto}def /fi{fill SaveX SaveY moveto}def /dot{/myY
exch def /myX exch def /deltax currentlinewidth 2 div def np myX deltax sub
myY moveto myX deltax add myY lineto st}def /ellipse{/endangle exch def
/startangle exch def /yrad exch def /xrad exch def /myY exch def /myX exch def
/savematrix matrix currentmatrix def myX myY translate xrad yrad scale 0 0 1
startangle endangle arc savematrix setmatrix}def /a{moveto}def /delta 0 def /b
{exch p dup /delta exch def 0 rmoveto}def /c{p delta 4 sub dup /delta exch def
0 rmoveto}def /d{p delta 3 sub dup /delta exch def 0 rmoveto}def /e{p delta 2
sub dup /delta exch def 0 rmoveto}def /f{p delta 1 sub dup /delta exch def 0
rmoveto}def /g{p delta 0 rmoveto}def /h{p delta 1 add dup /delta exch def 0
rmoveto}def /i{p delta 2 add dup /delta exch def 0 rmoveto}def /j{p delta 3
add dup /delta exch def 0 rmoveto}def /k{p delta 4 add dup /delta exch def 0
rmoveto}def /l{p -4 0 rmoveto}def /m{p -3 0 rmoveto}def /n{p -2 0 rmoveto}def
/o{p -1 0 rmoveto}def /q{p 1 0 rmoveto}def /r{p 2 0 rmoveto}def /s{p 3 0
rmoveto}def /t{p 4 0 rmoveto}def /w{0 rmoveto}def /x{0 exch rmoveto}def /y{3
-1 roll p moveto}def /bos{/section save def}def /eos{section restore}def
/@SpecialDefaults{/hs 8.5 Inch def /vs 11 Inch def /ho 0 def /vo 0 def /hsc 1
def /vsc 1 def /CLIP false def}def /@hsize{/hs exch def /CLIP true def}def
/@vsize{/vs exch def /CLIP true def}def /@hoffset{/ho exch def}def /@voffset{
/vo exch def}def /@hscale{/hsc exch def}def /@vscale{/vsc exch def}def
/@setclipper{hsc vsc scale CLIP{newpath 0 0 moveto hs 0 rlineto 0 vs rlineto
hs neg 0 rlineto closepath clip}if}def /@beginspecial{gsave /SpecialSave save
def currentpoint transform initgraphics itransform translate @SpecialDefaults
@MacSetUp}def /@setspecial{MacDrwgs{md begin /pxt ho def /pyt vo neg def end}{
ho vo translate @setclipper}ifelse}def /@endspecial{SpecialSave restore
grestore}def /MacDrwgs false def /@MacSetUp{userdict /md known{userdict /md
get type /dicttype eq{md begin /letter{}def /note{}def /legal{}def /od{txpose
1 0 mtx defaultmatrix dtransform exch atan/pa exch def newpath clippath mark{
transform{itransform moveto}}{transform{itransform lineto}}{6 -2 roll
transform 6 -2 roll transform 6 -2 roll transform{itransform 6 2 roll
itransform 6 2 roll itransform 6 2 roll curveto}}{{closepath}}pathforall
newpath counttomark array astore /gc xdf pop ct 39 0 put 10 fz 0 fs 2
F/|______Courier fnt invertflag{PaintBlack}if}def /txpose{pxs pys scale ppr
aload pop por{noflips{pop exch neg exch translate pop 1 -1 scale}if xflip
yflip and{pop exch neg exch translate 180 rotate 1 -1 scale ppr 3 get ppr 1
get neg sub neg ppr 2 get ppr 0 get neg sub neg translate}if xflip yflip not
and{pop exch neg exch translate pop 180 rotate ppr 3 get ppr 1 get neg sub neg
0 translate}if yflip xflip not and{ppr 1 get neg ppr 0 get neg translate}if}{
noflips{translate pop pop 270 rotate 1 -1 scale}if xflip yflip and{translate
pop pop 90 rotate 1 -1 scale ppr 3 get ppr 1 get neg sub neg ppr 2 get ppr 0
get neg sub neg translate}if xflip yflip not and{translate pop pop 90 rotate
ppr 3 get ppr 1 get neg sub neg 0 translate}if yflip xflip not and{translate
pop pop 270 rotate ppr 2 get ppr 0 get neg sub neg 0 exch translate}if}ifelse
scaleby96{ppr aload pop 4 -1 roll add 2 div 3 1 roll add 2 div 2 copy
translate .96 dup scale neg exch neg exch translate}if}def /cp{pop pop
showpage pm restore}def end}if}if}def /psf$TeXscale{65536 div}def
/DocumentInitState[matrix currentmatrix currentlinewidth currentlinecap
currentlinejoin currentdash currentgray currentmiterlimit]cvx def /startTexFig
{/psf$SavedState save def userdict maxlength dict begin currentpoint transform
DocumentInitState setmiterlimit setgray setdash setlinejoin setlinecap
setlinewidth setmatrix itransform moveto /psf$ury exch psf$TeXscale def
/psf$urx exch psf$TeXscale def /psf$lly exch psf$TeXscale def /psf$llx exch
psf$TeXscale def /psf$y exch psf$TeXscale def /psf$x exch psf$TeXscale def
currentpoint /psf$cy exch def /psf$cx exch def /psf$sx psf$x psf$urx psf$llx
sub div def /psf$sy psf$y psf$ury psf$lly sub div def psf$sx psf$sy scale
psf$cx psf$sx div psf$llx sub psf$cy psf$sy div psf$ury sub translate
/DefFigCTM matrix currentmatrix def /initmatrix{DefFigCTM setmatrix}def
/defaultmatrix{DefFigCTM exch copy}def /initgraphics{DocumentInitState
setmiterlimit setgray setdash setlinejoin setlinecap setlinewidth setmatrix
DefFigCTM setmatrix}def /showpage{initgraphics}def /erasepage{initgraphics}
def /copypage{}def}def /clipFig{currentpoint 6 2 roll newpath 4 copy 4 2 roll
moveto 6 -1 roll exch lineto exch lineto exch lineto closepath clip newpath
moveto}def /doclip{psf$llx psf$lly psf$urx psf$ury clipFig}def /endTexFig{end
psf$SavedState restore}def end statusdict /waittimeout 300 put
