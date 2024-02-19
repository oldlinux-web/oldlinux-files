/*
  "E:\TC\ORIGAMI.158\SRC\TOS\ORI_RSC.RSH"
  this file was created by GEM ressource converter for Turbo-C
  ½ 1990/91 by Michael Schwingen
*/
#include <aes.h>




BITBLK rs_bitblk[] = {
0};


ICONBLK rs_iconblk[] = {
0};

TEDINFO rs_tedinfo[] = {
" ORIGAMI folding editor","","", 3, 6, 2, 0x1061, 0x0, -1, 24,1};

OBJECT rs_object[] = {
-1, 1, 10, G_IBOX, NONE, NORMAL, 0x0L, 0,0, 90,25,
10, 2, 2, G_BOX, NONE, NORMAL, 0x1111L, 0,0, 90,513,
1, 3, 9, G_IBOX, NONE, NORMAL, 0x0L, 2,0, 53,769,
4, -1, -1, G_TITLE, NONE, NORMAL," ORIGAMI ", 0,0, 9,769,
5, -1, -1, G_TITLE, NONE, NORMAL," File ", 9,0, 6,769,
6, -1, -1, G_TITLE, NONE, NORMAL," Edit ", 15,0, 6,769,
7, -1, -1, G_TITLE, NONE, NORMAL," Goto ", 21,0, 6,769,
8, -1, -1, G_TITLE, NONE, NORMAL," Folding ", 27,0, 9,769,
9, -1, -1, G_TITLE, NONE, NORMAL," Macros ", 36,0, 8,769,
2, -1, -1, G_TITLE, NONE, NORMAL," Special ", 44,0, 9,769,
0, 11, 80, G_IBOX, NONE, NORMAL, 0x0L, 0,769, 80,19,
20, 12, 19, G_BOX, NONE, NORMAL, 0xFF1100L, 2,0, 20,8,
13, -1, -1, G_STRING, NONE, NORMAL,"  About ORIGAMI     ", 0,0, 20,1,
14, -1, -1, G_STRING, NONE, DISABLED,"--------------------", 0,1, 20,1,
15, -1, -1, G_STRING, NONE, NORMAL,"1", 0,2, 20,1,
16, -1, -1, G_STRING, NONE, NORMAL,"2", 0,3, 20,1,
17, -1, -1, G_STRING, NONE, NORMAL,"3", 0,4, 20,1,
18, -1, -1, G_STRING, NONE, NORMAL,"4", 0,5, 20,1,
19, -1, -1, G_STRING, NONE, NORMAL,"5", 0,6, 20,1,
11, -1, -1, G_STRING, NONE, NORMAL,"6", 0,7, 20,1,
32, 21, 31, G_BOX, NONE, NORMAL, 0xFF1100L, 11,0, 17,11,
22, -1, -1, G_STRING, NONE, NORMAL,"  Quit           ", 0,9, 17,1,
23, -1, -1, G_STRING, NONE, NORMAL,"  Save & Quit    ", 0,10, 17,1,
24, -1, -1, G_STRING, NONE, DISABLED,"-----------------", 0,3, 17,1,
25, -1, -1, G_STRING, NONE, NORMAL,"  Read new file  ", 0,0, 17,1,
26, -1, -1, G_STRING, NONE, NORMAL,"  Write to file  ", 0,1, 17,1,
27, -1, -1, G_STRING, NONE, NORMAL,"  Save           ", 0,2, 17,1,
28, -1, -1, G_STRING, NONE, NORMAL,"  Insert file    ", 0,4, 17,1,
29, -1, -1, G_STRING, NONE, DISABLED,"-----------------", 0,5, 17,1,
30, -1, -1, G_STRING, NONE, NORMAL,"  TOS Shell      ", 0,6, 17,1,
31, -1, -1, G_STRING, NONE, NORMAL,"  Shell command  ", 0,7, 17,1,
20, -1, -1, G_STRING, NONE, DISABLED,"-----------------", 0,8, 17,1,
43, 33, 42, G_BOX, NONE, NORMAL, 0xFF1100L, 17,0, 20,10,
34, -1, -1, G_STRING, NONE, DISABLED,"--------------------", 0,1, 20,1,
35, -1, -1, G_STRING, NONE, NORMAL,"  Double-line       ", 0,0, 20,1,
36, -1, -1, G_STRING, NONE, NORMAL,"  Move line         ", 0,2, 20,1,
37, -1, -1, G_STRING, NONE, DISABLED,"-- Pick buffer -----", 0,3, 20,1,
38, -1, -1, G_STRING, NONE, NORMAL,"  Pick line         ", 0,4, 20,1,
39, -1, -1, G_STRING, NONE, NORMAL,"  Copy-pick line    ", 0,5, 20,1,
40, -1, -1, G_STRING, NONE, NORMAL,"  Fold pick buffer  ", 0,6, 20,1,
41, -1, -1, G_STRING, NONE, DISABLED,"-- Delete buffer ---", 0,7, 20,1,
42, -1, -1, G_STRING, NONE, NORMAL,"  Delete line       ", 0,8, 20,1,
32, -1, -1, G_STRING, NONE, NORMAL,"  Undelete Line     ", 0,9, 20,1,
54, 44, 53, G_BOX, NONE, NORMAL, 0xFF1100L, 23,0, 18,10,
45, -1, -1, G_STRING, NONE, DISABLED,"------------------", 0,1, 18,1,
46, -1, -1, G_STRING, NONE, NORMAL,"  Line            ", 0,0, 18,1,
47, -1, -1, G_STRING, NONE, NORMAL,"  Top of fold     ", 0,2, 18,1,
48, -1, -1, G_STRING, NONE, NORMAL,"  Bottom of fold  ", 0,3, 18,1,
49, -1, -1, G_STRING, NONE, DISABLED,"------------------", 0,4, 18,1,
50, -1, -1, G_STRING, NONE, NORMAL,"  Search          ", 0,5, 18,1,
51, -1, -1, G_STRING, NONE, NORMAL,"  Search reverse  ", 0,6, 18,1,
52, -1, -1, G_STRING, NONE, NORMAL,"  Replace         ", 0,8, 18,1,
53, -1, -1, G_STRING, NONE, NORMAL,"  Query-replace   ", 0,9, 18,1,
43, -1, -1, G_STRING, NONE, DISABLED,"------------------", 0,7, 18,1,
70, 55, 69, G_BOX, NONE, NORMAL, 0xFF1100L, 29,0, 20,15,
56, -1, -1, G_STRING, NONE, DISABLED,"--------------------", 0,2, 20,1,
57, -1, -1, G_STRING, NONE, NORMAL,"  Open fold         ", 0,0, 20,1,
58, -1, -1, G_STRING, NONE, NORMAL,"  Close fold        ", 0,1, 20,1,
59, -1, -1, G_STRING, NONE, NORMAL,"  Enter fold        ", 0,3, 20,1,
60, -1, -1, G_STRING, NONE, NORMAL,"  Exit fold         ", 0,4, 20,1,
61, -1, -1, G_STRING, NONE, DISABLED,"--------------------", 0,5, 20,1,
62, -1, -1, G_STRING, NONE, NORMAL,"  Unfold fold       ", 0,6, 20,1,
63, -1, -1, G_STRING, NONE, NORMAL,"  Create fold       ", 0,7, 20,1,
64, -1, -1, G_STRING, NONE, NORMAL,"  Create auto fold  ", 0,8, 20,1,
65, -1, -1, G_STRING, NONE, DISABLED,"-- Toggle: ---------", 0,10, 20,1,
66, -1, -1, G_STRING, NONE, NORMAL,"  Auto header       ", 0,9, 20,1,
67, -1, -1, G_STRING, NONE, NORMAL,"  File fold         ", 0,11, 20,1,
68, -1, -1, G_STRING, NONE, NORMAL,"  Attach file       ", 0,12, 20,1,
69, -1, -1, G_STRING, NONE, NORMAL,"  Describe fold     ", 0,14, 20,1,
54, -1, -1, G_STRING, NONE, DISABLED,"--------------------", 0,13, 20,1,
80, 71, 79, G_BOX, NONE, NORMAL, 0xFF1100L, 38,0, 19,9,
72, -1, -1, G_STRING, NONE, NORMAL,"  Define macro     ", 0,0, 19,1,
73, -1, -1, G_STRING, NONE, NORMAL,"  Execute macro    ", 0,1, 19,1,
74, -1, -1, G_STRING, NONE, NORMAL,"  Define fix mac   ", 0,2, 19,1,
75, -1, -1, G_STRING, NONE, DISABLED,"-------------------", 0,3, 19,1,
76, -1, -1, G_STRING, NONE, NORMAL,"  Exec fix mac #1  ", 0,4, 19,1,
77, -1, -1, G_STRING, NONE, NORMAL,"  Exec fix mac #2  ", 0,5, 19,1,
78, -1, -1, G_STRING, NONE, NORMAL,"  Exec fix mac #3  ", 0,6, 19,1,
79, -1, -1, G_STRING, NONE, NORMAL,"  Exec fix mac #4  ", 0,7, 19,1,
70, -1, -1, G_STRING, NONE, NORMAL,"  Exec fix mac #5  ", 0,8, 19,1,
10, 81, 84, G_BOX, NONE, NORMAL, 0xFF1100L, 46,0, 21,4,
82, -1, -1, G_STRING, NONE, NORMAL,"  Help               ", 0,0, 21,1,
83, -1, -1, G_STRING, NONE, NORMAL,"  Describe bindings  ", 0,1, 21,1,
84, -1, -1, G_STRING, NONE, DISABLED,"---------------------", 0,2, 21,1,
80, -1, -1, G_STRING, LASTOB, NORMAL,"  Set language       ", 0,3, 21,1,
-1, 1, 8, G_BOX, NONE, OUTLINED, 0x21111L, 0,0, 38,14,
2, -1, -1, G_BOXTEXT, NONE, SHADOWED,&rs_tedinfo[0], 2,1, 34,2,
3, -1, -1, G_STRING, NONE, NORMAL,"Improvements of UN*X version, OCL", 2,4, 33,1,
4, -1, -1, G_STRING, NONE, NORMAL,"and many bug fixes by:", 2,5, 25,1,
5, -1, -1, G_STRING, NONE, NORMAL," Michael Haardt", 2,6, 16,1,
6, -1, -1, G_STRING, NONE, NORMAL," Wolfgang Stumvoll", 2,7, 19,1,
7, -1, -1, G_STRING, NONE, NORMAL,"ATARI ST port & GEM routines by", 2,9, 34,1,
8, -1, -1, G_STRING, NONE, NORMAL,"Michael Schwingen", 2,10, 17,1,
0, -1, -1, G_BUTTON, 0x27, NORMAL,"ORIGAMI is PD - spread it !", 2,12, 34,1};

OBJECT *rs_trindex[] = {
&rs_object[0],
&rs_object[85]};


#define NUM_OBS 94

void rsrc_init(void)
{
	int i;
	for(i=0;i<NUM_OBS;i++)
		rsrc_obfix(rs_object,i);
}

