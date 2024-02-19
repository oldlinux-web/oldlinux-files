.first
	@lsrc:[x11]logicals.com     !<<< change for your system!
	copy/log x11_xmu:xmushr.psects,x11_xaw:xawshr.psects,x11_xaw:user.opt -
		gv.opt
GV.EXE : GV.OLB XMUSHR XAWSHR
	$(link) $(linkflags) /exe=gv.exe 		gv.olb/include=main/library,gv.opt/opt
GV.OLB : GV.OLB(MAIN=MAIN.OBJ) GV.OLB(MISC=MISC.OBJ) GV.OLB(CALLBACKS=CALLBACKS.OBJ) -
	 GV.OLB(ACTIONS=ACTIONS.OBJ) GV.OLB(DIALOGS=DIALOGS.OBJ) GV.OLB(GHOSTVIEW=GHOSTVIEW.OBJ) -
	 GV.OLB(PS=PS.OBJ) GV.OLB(STRCASECMP=STRCASECMP.OBJ)
	@ continue
GV.OLB(MAIN=MAIN.OBJ) : MAIN.OBJ
	If "''F$Search("$@")'" .EQS. "" Then $(LIBR)/Create $@
	$(LIBR)$(LIBRFLAGS) $@ MAIN.OBJ
MAIN.OBJ : MAIN.C GHOSTVIEW.H GV.H -
	 PS.H APP-DEFAULTS.H
	$(CC) $(CFLAGS) MAIN.C
APP-DEFAULTS.H : GHOSTVIEW.AD
	get_run = 0
	get_ad2c = 0
	if f$search("run-ad2c.").eqs."" then get_run = 1
	if f$search("ad2c.").eqs."" then get_ad2c = 1
	- if get_run then cms fetch run-ad2c.
	- if get_ad2c then cms fetch ad2c.
	- posix/run posix$bin:sh. run-ad2c
	if get_run then delete run-ad2c.;*
	if get_ad2c then delete ad2c.;*
GV.OLB(MISC=MISC.OBJ) : MISC.OBJ
	If "''F$Search("$@")'" .EQS. "" Then $(LIBR)/Create $@
	$(LIBR)$(LIBRFLAGS) $@ MISC.OBJ
MISC.OBJ : MISC.C GHOSTVIEW.H GV.H -
	 PS.H
	$(CC) $(CFLAGS) MISC.C
GV.OLB(CALLBACKS=CALLBACKS.OBJ) : CALLBACKS.OBJ
	If "''F$Search("$@")'" .EQS. "" Then $(LIBR)/Create $@
	$(LIBR)$(LIBRFLAGS) $@ CALLBACKS.OBJ
CALLBACKS.OBJ : CALLBACKS.C GHOSTVIEW.H GV.H -
	 PS.H
	$(CC) $(CFLAGS) CALLBACKS.C
GV.OLB(ACTIONS=ACTIONS.OBJ) : ACTIONS.OBJ
	If "''F$Search("$@")'" .EQS. "" Then $(LIBR)/Create $@
	$(LIBR)$(LIBRFLAGS) $@ ACTIONS.OBJ
ACTIONS.OBJ : ACTIONS.C GV.H GHOSTVIEW.H -
	 PS.H
	$(CC) $(CFLAGS) ACTIONS.C
GV.OLB(DIALOGS=DIALOGS.OBJ) : DIALOGS.OBJ
	If "''F$Search("$@")'" .EQS. "" Then $(LIBR)/Create $@
	$(LIBR)$(LIBRFLAGS) $@ DIALOGS.OBJ
DIALOGS.OBJ : DIALOGS.C GV.H GHOSTVIEW.H
	$(CC) $(CFLAGS) DIALOGS.C
GV.OLB(GHOSTVIEW=GHOSTVIEW.OBJ) : GHOSTVIEW.OBJ
	If "''F$Search("$@")'" .EQS. "" Then $(LIBR)/Create $@
	$(LIBR)$(LIBRFLAGS) $@ GHOSTVIEW.OBJ
GHOSTVIEW.OBJ : GHOSTVIEW.C GHOSTVIEWP.H GHOSTVIEW.H -
	 VMS_TYPES.H
	$(CC) $(CFLAGS) GHOSTVIEW.C
GV.OLB(PS=PS.OBJ) : PS.OBJ
	If "''F$Search("$@")'" .EQS. "" Then $(LIBR)/Create $@
	$(LIBR)$(LIBRFLAGS) $@ PS.OBJ
PS.OBJ : PS.C PS.H
	$(CC) $(CFLAGS) PS.C
GV.OLB(STRCASECMP=STRCASECMP.OBJ) : STRCASECMP.OBJ
	If "''F$Search("$@")'" .EQS. "" Then $(LIBR)/Create $@
	$(LIBR)$(LIBRFLAGS) $@ STRCASECMP.OBJ
STRCASECMP.OBJ : STRCASECMP.C STDC.H
	$(CC) $(CFLAGS) STRCASECMP.C
