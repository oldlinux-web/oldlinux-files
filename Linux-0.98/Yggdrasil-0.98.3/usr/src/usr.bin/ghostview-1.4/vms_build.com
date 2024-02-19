$ @lsrc:[x11]logicals.com     !<<< change for your system!
$ copy/log x11_xmu:xmushr.psects,x11_xaw:xawshr.psects,x11_xaw:user.opt 		gv.opt
$ get_run = 0
$ get_ad2c = 0
$ if f$search("run-ad2c.").eqs."" then get_run = 1
$ if f$search("ad2c.").eqs."" then get_ad2c = 1
$ if get_run then cms fetch run-ad2c.
$ if get_ad2c then cms fetch ad2c.
$ posix/run posix$bin:sh. run-ad2c
$ if get_run then delete run-ad2c.;*
$ if get_ad2c then delete ad2c.;*
$ CC /NOLIST/OBJECT=MAIN.OBJ MAIN.C
$ If "''F$Search("GV.OLB")'" .EQS. "" Then LIBRARY/Create GV.OLB
$ LIBRARY/REPLACE GV.OLB MAIN.OBJ
$ CC /NOLIST/OBJECT=MISC.OBJ MISC.C
$ If "''F$Search("GV.OLB")'" .EQS. "" Then LIBRARY/Create GV.OLB
$ LIBRARY/REPLACE GV.OLB MISC.OBJ
$ CC /NOLIST/OBJECT=CALLBACKS.OBJ CALLBACKS.C
$ If "''F$Search("GV.OLB")'" .EQS. "" Then LIBRARY/Create GV.OLB
$ LIBRARY/REPLACE GV.OLB CALLBACKS.OBJ
$ CC /NOLIST/OBJECT=ACTIONS.OBJ ACTIONS.C
$ If "''F$Search("GV.OLB")'" .EQS. "" Then LIBRARY/Create GV.OLB
$ LIBRARY/REPLACE GV.OLB ACTIONS.OBJ
$ CC /NOLIST/OBJECT=DIALOGS.OBJ DIALOGS.C
$ If "''F$Search("GV.OLB")'" .EQS. "" Then LIBRARY/Create GV.OLB
$ LIBRARY/REPLACE GV.OLB DIALOGS.OBJ
$ CC /NOLIST/OBJECT=GHOSTVIEW.OBJ GHOSTVIEW.C
$ If "''F$Search("GV.OLB")'" .EQS. "" Then LIBRARY/Create GV.OLB
$ LIBRARY/REPLACE GV.OLB GHOSTVIEW.OBJ
$ CC /NOLIST/OBJECT=PS.OBJ PS.C
$ If "''F$Search("GV.OLB")'" .EQS. "" Then LIBRARY/Create GV.OLB
$ LIBRARY/REPLACE GV.OLB PS.OBJ
$ CC /NOLIST/OBJECT=STRCASECMP.OBJ STRCASECMP.C
$ If "''F$Search("GV.OLB")'" .EQS. "" Then LIBRARY/Create GV.OLB
$ LIBRARY/REPLACE GV.OLB STRCASECMP.OBJ
$ continue
$ LINK /TRACE/NOMAP/EXEC=GV.EXE /exe=gv.exe 		gv.olb/include=main/library,gv.opt/opt
