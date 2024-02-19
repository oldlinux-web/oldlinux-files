#    Copyright (C) 1991, 1992 Aladdin Enterprises.  All rights reserved.
#    Distributed by Free Software Foundation, Inc.
#
# This file is part of Ghostscript.
#
# Ghostscript is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
# to anyone for the consequences of using it or for whether it serves any
# particular purpose or works at all, unless he says so in writing.  Refer
# to the Ghostscript General Public License for full details.
#
# Everyone is granted permission to copy, modify and redistribute
# Ghostscript, but only under the conditions described in the Ghostscript
# General Public License.  A copy of this license is supposed to have been
# given to you along with Ghostscript so you can know your rights and
# responsibilities.  It should be in a file named COPYING.  Among other
# things, the copyright notice and this notice must be preserved on all
# copies.

# Auxiliary MS-DOS makefile for maintenance operations.

# This file pertains to Aladdin Enterprises maintenance operations,
# and is unlikely to be useful to users.

# Targets:
#	xmit -- make the .BE files for Kermit transmission
#	xfonts -- make the .BE files for the fonts
#	xfer -- make a diskette for transferring files to the Sun
#	release -- make the .BE files for a release
# Remember to erase *.zip / *.z / *.be, if needed, before running these.

# ---------------- Convert Windows icons ----------------

gsgraph.icx: gsgraph.uue
	eadcode / gsgraph.uue gsgraph.ico
	od -o gsgraph.icx -H gsgraph.ico

gstext.icx: gstext.uue
	eadcode / gstext.uue gstext.ico
	od -o gstext.icx -H gstext.ico

# ---------------- Make files for Kermit transmission ----------------

TARS=tar1.be tar2.be tar3a.be tar3b.be tar3c.be tar4a.be tar4b.be tar5.be tar6.be tar7.be

xmit: $(TARS)

xexe: tar0.be

xfonts: tar8.be

xfer: tar1.z tar2.z tar3a.z tar3b.z tar3c.z tar4a.z tar4b.z \
  tar5.z tar6.z tar7.z
	@echo ---------------- Insert diskette:
	@command /c pause
	xcopy tar_x.* a:
	xcopy tar*.z a:

release: xmit xexe

# The dependency lists for the .be files should be much longer!

.z.be:
	bed $*.z $*.be
	erase $*.z

tar0.z: gs.exe
	@if not exist gs386.exe echo gs386.exe does not exist, do you want to proceed?
	@if not exist gs386.exe pause
	@if not exist gswin.exe echo gswin.exe does not exist, do you want to proceed?
	@if not exist gswin.exe pause
	tar -b1 -cf _temp_.t -uexe gs*.exe
	compress -i _temp_.t
	if exist tar0.z erase tar0.z
	rename _temp_.txz tar0.z

tar1.z: bdftops.bat font2c.bat
	tar -b1 -cf _temp_.t -ubat g*.asm i*.asm gs*.bat bdftops.bat font2c.bat

	compress -i _temp_.t
	if exist tar1.z erase tar1.z
	rename _temp_.txz tar1.z

tar2.z: \cp.bat \mv.bat \rm.bat ansi2knr.c echogs.c ega.c uniq.c
	copy \cp.bat
	copy \mv.bat
	copy \rm.bat
	tar -b1 -cf _temp_.t -ubat cp.bat mv.bat rm.bat ansi2knr.c echogs.c ega.c uniq.c
	compress -i _temp_.t
	if exist tar2.z erase tar2.z
	rename _temp_.txz tar2.z

tar3a.z: gs.c
	tar -b1 -cf _temp_.t gs*.c
	compress -i _temp_.t
	if exist tar3a.z erase tar3a.z
	rename _temp_.txz tar3a.z

tar3b.z: gxfill.c
	tar -b1 -cf _temp_.t gx*.c
	compress -i _temp_.t
	if exist tar3b.z erase tar3b.z
	rename _temp_.txz tar3b.z

tar3c.z: gconfig.c genarch.c
	tar -b1 -cf _temp_.t gdev*.c gp_*.c gconfig.c genarch.c
	compress -i _temp_.t
	if exist tar3c.z erase tar3c.z
	rename _temp_.txz tar3c.z

tar4a.z: interp.c stream.c turboc.cfg gs.def history.doc gstext.icx gs.rc
	tar -b1 -cf _temp_.t i*.c s*.c turboc.cfg gs.def *.doc *.icx gs.rc
	compress -i _temp_.t
	if exist tar4a.z erase tar4a.z
	rename _temp_.txz tar4a.z

tar4b.z: zarith.c
	tar -b1 -cf _temp_.t z*.c
	compress -i _temp_.t
	if exist tar4b.z erase tar4b.z
	rename _temp_.txz tar4b.z

tar5.z: gs.h gs.mak
	tar -b1 -cf _temp_.t *.h *.mak *.man *.sh *.tr tar_*.
	compress -i _temp_.t
	if exist tar5.z erase tar5.z
	rename _temp_.txz tar5.z

tar6.z: gs_init.ps uglyr.gsf
	tar -b1 -cf _temp_.t *.ps ugly*.*
	compress -i _temp_.t
	if exist tar6.z erase tar6.z
	rename _temp_.txz tar6.z

tar7.z: fontmap copying news readme bdftops font2c ccgs
	tar -b1 -cf _temp_.t fontmap copying news readme bdftops font2c gs*. ccgs
	compress -i _temp_.t
	if exist tar7.z erase tar7.z
	rename _temp_.txz tar7.z

tar8.z: fonts\bchr.gsf fonts\hrsy_r.gsf
	tar -b1 -cf _temp_.t fonts/*.gsf
	compress -i _temp_.t
	if exist tar8.z erase tar8.z
	rename _temp_.txz tar8.z

# ---------------- Make MS-DOS diskette sets ----------------

allzips: gsexe.zip gsfiles.zip gssrc1.zip gssrc2.zip \
  gsfonts1.zip gsfonts2.zip gsfonts3.zip gsfonts4.zip
	@echo ---------------- Done. ----------------

srczips: gsfiles.zip gssrc1.zip gssrc2.zip
	@echo ---------------- Done. ----------------

zips: gsexe.zip gsfiles.zip gssrc1.zip gssrc2.zip
	@echo ---------------- Done. ----------------

# Here are the ZIP files that go onto the diskettes.

gsexe.zip: gs.exe
	@if not exist gs386.exe echo gs386.exe does not exist, do you want to proceed?
	@if not exist gs386.exe pause
	@if not exist gswin.exe echo gswin.exe does not exist, do you want to proceed?
	@if not exist gswin.exe pause
	if exist gsexe.zip erase gsexe.zip
	if exist gs386.exe pkzip gsexe.zip gs*.exe \watc\bin\dos4gw.exe
	if not exist gs386.exe pkzip gsexe.zip gs*.exe

gsfiles.zip: bdftops.bat
	if exist _temp_.zip erase _temp_.zip
	pkzip _temp_.zip gs*.bat bdftops.bat font2c.bat *.doc
	pkzip -a _temp_.zip *.ps fontmap copying news readme bdftops font2c
	pkzip -d _temp_.zip q* q*.* t.* comp1.*
	if exist gsfiles.zip erase gsfiles.zip
	rename _temp_.zip gsfiles.zip

gssrc1.zip: ansi2knr.c
	if exist _temp_.zip erase _temp_.zip
	pkzip _temp_.zip \rm.bat \cp.bat \mv.bat
	pkzip -a _temp_.zip g*.asm i*.asm
	pkzip -a _temp_.zip ansi2knr.c echogs.c ega.c uniq.c turboc.cfg gs.def *.icx gs.rc
	pkzip -a _temp_.zip *.h *.mak *.man *.sh *.tr tar_*. ugly*.*
	pkzip -a _temp_.zip gs*. ccgs
	pkzip -d _temp_.zip arch.h gconfig*.h obj*.tr lib*.tr _temp*.*
	pkzip -d _temp_.zip ugly*.bdf libc*.tr q* q*.* t.* comp1.*
	if exist gssrc1.zip erase gssrc1.zip
	rename _temp_.zip gssrc1.zip

gssrc2.zip: gs.c
	if exist _temp_.zip erase _temp_.zip
	pkzip _temp_.zip g*.c i*.c s*.c z*.c
	pkzip -d _temp_.zip comp1.*
	if exist gssrc2.zip erase gssrc2.zip
	rename _temp_.zip gssrc2.zip

gsfonts1.zip: fonts\phvr.gsf fonts\pncr.gsf fonts\pplr.gsf
	if exist _temp_.zip erase _temp_.zip
	pkzip _temp_.zip fonts\ph*.* fonts\pn*.* fonts\pp*.*
	if exist gsfonts1.zip erase gsfonts1.zip
	rename _temp_.zip gsfonts1.zip

gsfonts2.zip: fonts\bchr.gsf fonts\cyr.gsf fonts\pagk.gsf fonts\pbkd.gsf
	if exist _temp_.zip erase _temp_.zip
	pkzip _temp_.zip fonts\b*.* fonts\cy*.* fonts\pa*.* fonts\pb*.*
	if exist gsfonts2.zip erase gsfonts2.zip
	rename _temp_.zip gsfonts2.zip

gsfonts3.zip: fonts\psyr.gsf fonts\ptmr.gsf fonts\pzdr.gsf fonts\zcr.gsf
	if exist _temp_.zip erase _temp_.zip
	pkzip _temp_.zip fonts\ps*.* fonts\pt*.* fonts\pz*.* fonts\z*.*
	if exist gsfonts3.zip erase gsfonts3.zip
	rename _temp_.zip gsfonts3.zip

gsfonts4.zip: fonts\ncrr.gsf fonts\putr.gsf fonts\hrsy_r.gsf
	if exist _temp_.zip erase _temp_.zip
	pkzip _temp_.zip fonts\n*.gsf fonts\pu*.* fonts\h*.gsf
	if exist gsfonts4.zip erase gsfonts4.zip
	rename _temp_.zip gsfonts4.zip
