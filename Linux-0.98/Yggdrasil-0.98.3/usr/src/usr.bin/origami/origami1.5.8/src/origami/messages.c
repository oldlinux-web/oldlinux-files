/*{{{  #includes*/
#include <limits.h>
#include <stdio.h>

#include <local/bool.h>
 
#define MESSAGES_C
 
#include "origami.h"
/*}}}  */
 
/*{{{  formats*/
char F_F_FAILED[]       ="open filed-fold %s failed";
char F_LOADING[]        ="loading %s";
char F_NO_OPEN[]        ="You are not allowed to open %s";
char F_SUBSTITUTIONS[]  ="%d substitutions";
char F_TCFF[]           ="%s - Overwrite %s";
char F_TERMA[]          ="No %s (%s) capability for this terminal - aborted.\n";
char F_TERMB[]          ="Cannot find %s in termcap.\n";
char F_UNABLE[]         ="Unable to open %s.\n";
char F_WRITING[]        ="Writing %s ...";
char F_WRITTEN[]        ="%s written";
/*}}}  */
/*{{{  messages*/
/*{{{  general*/
char M_ABORTED[]        ="aborted";
char M_AUTO_SAVED[]     ="auto-saved";
char M_BYE[]            ="Bye";
char M_ERR_ARG[]        ="wrong parameter list\n";
char M_FAILED[]         ="[failed]";
char M_MAIL_S[]         ="Abnormal termination of Origami";
char M_MAIL_1[]         ="Origami was killed while a unsaved file exists and tried to save it in:";
char M_MAIL_2[]         ="Please check if its contents are valid before using it.";
char M_NOT_IF_TDS[]     ="using TDS!";
char M_NO_MEMORY[]      ="not enough memory";
char M_SHELL_CREATED[]  ="sub shell created";
char M_SH_COMMAND[]     ="shell command";
char M_VIEW[]           ="view mode!";
char M_WIN_CHANGED[]    ="window changed";
/*}}}  */
/*{{{  keyboard*/
char M_DEF_EXE[]        ="no define macro during execute macro";
char M_EXE_EXE[]        ="exe macro not allowed during define macro";
char M_FIX_EXE[]        ="no define-fix during define/execute macro";
char M_KEY_BUFF_FULL[]  ="keyboard-macro-buffer full";
char M_NOFOLDKEY[]      ="key disabled during create fold";
char M_NOT_BOUND[]      ="key not bound";
char M_NO_DEFINE[]      ="macro is not defineable";
/*}}}  */
/*{{{  rc /term / env*/
char M_ERR_DEPTH[]      ="invalid macrostackdepth\n";
char M_NO_AUTO[]        ="no automacro given.\n";
char M_NO_HOME[]        ="Environment variable HOME not set.\n";
char M_NO_KTAB[]        ="bad data in rcfile\n";
char M_NO_MARKS[]       ="no matching marks given in rc-file.\n";
char M_NO_SHELL_VAR[]   ="no SHELL environment variable";
char M_NO_TERMCAP[]     ="Cannot open termcap.\n";
char M_NO_TERM[]        ="Environment variable TERM not set.\n";
char M_ERR_MACROPLACE[] ="invalid macroplace";
char M_NO_PROG[]        ="no macro_programm_ops allowed";
char M_STACK_OVER[]     ="macro-stack overflow, macro stopped";
/*}}}  */
/*{{{  prompt*/
char M_ANY[]            ="press any key";
char M_ANY_Q[]          ="press any key or q to quit";
char M_ARG[]            ="arg:";
char M_CHANGED_LEAVE[]  ="File modified. Leave anyway";
char M_CHANGED_SAVE[]   ="File not saved. Save";
char M_CO_END[]         ="End of comment";
char M_CO_START[]       ="Start of comment";
char M_END[]            ="End";
char M_GOTO[]           ="goto-line";
char M_NO_SAVE_CANCEL[] ="cannot save - cancel changes";
char M_Q_R_PROMPT[]     ="(Y)es, (N)o, (!)Do rest, (A)bort, (.)Do and abort:";
char M_TABSIZE[]        ="Tab size";
char M_WHICH_FIX[]      ="Which fix_macro_buffer";
char M_WHICH_MODE[]     ="Which mode";
/*}}}  */
/*{{{  folds*/
char M_EX_NFF[]         ="exiting not-filed-fold";
char M_FOLDERR[]        ="cannot fold";
char M_NO_BACK_FOLDING[]="cannot fold backwards";
char M_NO_ENTER_LIST_FF[]="cannot enter listing of filed fold";
char M_NO_FF_COPY[]     ="cannot copy filed folds";
char M_NO_SIMPLE_FOLD[] ="line is not a simple fold!";
char M_NO_TEXT[]        ="not text fold";
char M_USE_EXIT[]       ="use exit";
/*}}}  */
/*{{{  editing*/
char M_LONG_LINE[]      ="long_line";
char M_NO_PICK[]        ="no pick";
char M_NO_UNDEL[]       ="nothing to undelete";
char M_REPLACE[]        ="Replace with";
char M_SEARCH[]         ="Search for";
/*}}}  */
/*{{{  position*/
char M_BOTTOM[]         ="BOTTOM of LIST";
char M_ERR_PO[]         ="invalid edit-position";
char M_NO_TOP[]         ="not at top";
char M_TOP[]            ="TOP of LIST";
/*}}}  */
/*{{{  files/pipes*/
char M_CANTOPEN[]       ="cant open";
char M_CANTWRITE[]      ="can't write";
char M_CLOSE_FAILED[]   ="close failed";
char M_FILENAME[]       ="filename";
char M_FILE_NOT_FOUND[] ="file not found";
char M_NO_BIND[]        ="Bindingfile is not online";
char M_NO_CHANGE[]      ="no changes need to be written";
char M_NO_HELP[]        ="Helpfile is not online";
char M_NO_NEXT[]        ="no next file";
char M_NO_OUTFILE[]     ="can't open outputfile";
char M_NO_PREV[]        ="no previous file";
char M_NO_TOP_AUTO_SAVE[]="not at top for auto-save";
char M_READING[]        ="Reading...";
char M_READ[]           ="Read";
char M_WRITING[]        ="Writing...";
char M_WRITTEN[]        ="Written";
/*}}}  */
/*}}}  */
/*{{{  inputs*/
char NO='N';
char YES='Y';
char AB='A';
char DO_AB='.';
char DO_REST='!';
/*}}}  */
