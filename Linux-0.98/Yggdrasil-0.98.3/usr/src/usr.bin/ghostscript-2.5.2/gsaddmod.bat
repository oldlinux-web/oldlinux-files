@echo off
if '%3'=='' goto done
goto %2
rem ---------------- dev ----------------
:-dev
echo device_(gs_%3_device)>>%1.d_n
gsaddmod %1 %2 %4 %5 %6 %7 %8 %9
rem ---------------- font ----------------
:-font
echo font_("0.font_%3",gsf_%3,zf_%3)>>%1.d_f
gsaddmod %1 %2 %4 %5 %6 %7 %8 %9
rem ---------------- include ----------------
:-include
echo copy %3.dev _temp_i.bat>>%1.dev
echo command /c _temp_i.bat>>%1.dev
gsaddmod %1 %2 %4 %5 %6 %7 %8 %9
rem ---------------- lib ----------------
:-lib
echogs -a %1.dev -q echo -s -q %3+ -x 3e3e _temp_.dlb
gsaddmod %1 %2 %4 %5 %6 %7 %8 %9
rem ---------------- obj ----------------
:-obj
for %%x in (%3 %4 %5 %6 %7 %8 %9) do echo %%x+>>%1.d_o
for %%x in (%3 %4 %5 %6 %7 %8 %9) do echo FILE %%x>>%1.d_w
goto done
rem ---------------- oper ----------------
:-oper
for %%x in (%3 %4 %5 %6 %7 %8 %9) do echo oper_(%%x_op_defs)>>%1.d_v
goto done
rem ---------------- ps ----------------
:-ps
for %%x in (%3 %4 %5 %6 %7 %8 %9) do echo psfile_("%%x.ps")>>%1.d_v
goto done
rem - - - - - - - - done - - - - - - - -
:done
