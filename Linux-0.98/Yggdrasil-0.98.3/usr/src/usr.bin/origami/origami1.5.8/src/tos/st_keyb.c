/*
   keyboard.c - Keyboard interpreter for Origami
   ATARI ST version by Michael Schwingen
*/

/*{{{  #includes*/
#include <stdio.h>
#include <tos.h>
#include <vdi.h>
#include <aes.h>
#include "ORI_RSC.H"

#define KEYBOARD_C
#include "local\bool.h"
#include "..\origami\origami.h"
/*}}}  */
/*{{{  meta #defines*/
#define META 255   /* Meta-character for normal keys */
#define SMETA 254  /* Meta-Character when Shift is pressed */
#define CMETA 253  /* Meta-Character when Ctrl is pressed */
#define AMETA 252  /* Meta-Character when ALT is pressed */
#define MMETA 251  /* Meta-Character for GEM menu handling */
/*}}}  */

/*{{{  extern ressource structures and init routine, defined in ORI_RSC.C*/
/* Ressource structures and init routine, defined in ORI_RSC.C */
extern OBJECT rs_object[];
extern OBJECT *rs_trindex[];
void rsrc_init(void);
/*}}}  */
 
/*{{{  extern GEM variables*/
extern int phys_handle,handle,ap_id,
    wind_handle,
    gl_hchar,gl_wchar,gl_hbox,gl_wbox,
    wind_x,wind_y,wind_w,wind_h,
    _x,_y;
/*}}}  */

/*{{{  global variables*/
static int sys_kbd_save = 0; /* needed in get_raw_key */
static int sys_kbd_flag = 0; /* needed in get_raw_key */
PRIVATE bool echo;
bool echo_disabled=FALSE;
PRIVATE char command[PROMPT_LEN+1];
/*}}}  */

/*{{{  enable_echo*/
#ifdef STD_C
void enable_echo(bool yes)
#else
void enable_echo(yes) bool yes;
#endif
{
  echo=(!echo_disabled && yes);
}
/*}}}  */
/*{{{  switch_echo*/
#ifdef STD_C
void switch_echo(bool on)
#else
void switch_echo(on)
bool on;
#endif
{
  echo_disabled=!(echo=on);
}
/*}}}  */

/*{{{  enable_abort*/
void enable_abort(void)
{
/*
  On UN*X, this function enables an asynchronous abort function via signals.
  This is done if the keyboard is not active (eg during macro execution)
 
  On the ST, the routine abort_scanner (below) is called in the main macro
  loop and calls ori_abort if both shift keys are pressed.
*/
}
/*}}}  */
/*{{{  disable_abort*/
void disable_abort(void)
{
}
/*}}}  */
/*{{{  set_abort*/
void set_abort(key) char key;
{
  general_abort_key=key;
  /* this is not needes on the ST, too */
}
/*}}}  */
/*{{{  void abort_scanner(void) - replacement for signal-based abort on UN*X*/
void abort_scanner(void)
{
  if ((Kbshift(-1) & 3) == 3)
    ori_abort();
}
/*}}}  */

static unsigned long sys_alarmtime = 0;
/*{{{  void alarm(unsigned long seconds) - set alarm time*/
void alarm(unsigned long seconds)
{
  long   ssp;
 
  ssp = Super(NULL);         /* switch to supervisor mode */
  if (seconds != 0)
    sys_alarmtime = (*(unsigned long *)0x4BA)+200L*seconds;
  else
    sys_alarmtime = 0L;
  Super ((void *) ssp);      /* return to normal mode */
}
/*}}}  */

/*{{{  void init_keyboard(void)*/
#ifdef STD_C
void init_keyboard(void)
#else
void init_keyboard()
#endif
{
  long   ssp;

  /*
     setup system variable conterm to report state of shift/ctrl/alt keys in
     bits 24-31 of Bconin(2) result
  */
  ssp = Super(NULL);         /* switch to supervisor mode */
/*  * (char *) 0x484 |= 0x0f;*/  /* set flag */
  Super ((void *) ssp);      /* return to normal mode */
 
  sys_kbd_save = 0;
  sys_kbd_flag = 0;
  disable_abort();
  enable_echo(!echo_disabled);
}
/*}}}  */
/*{{{  void reset_keyboard(void)*/
#ifdef STD_C
void reset_keyboard(void)
#else
void reset_keyboard()
#endif
{
  long   ssp;
 
  /* switch of shift state report (see init_keyboard)*/
  ssp = Super(NULL) ;
/*  * (char *) 0x484 &= ~8;*/
  Super ((void *) ssp);
}
/*}}}  */
 
/*{{{  keyboard and menu translation tables*/
/*{{{  static char norm_key_tab[]*/
static char norm_key_tab[] =
{
  59,10, /* F1 */
  60,11,
  61,12,
  62,14,
  63,15,
  64,16,
  65,17,
  66,18,
  67,19,
  68,20,  /* F10 */
  71,1,   /* ClrHome */
  72,2,   /* arrow up */
  74,'-', /* - numeric keypad */
  75,3,   /* arrow left */
  77,4,   /* arrow right */
  78,'+', /* + numeric keypad */
  80,5,   /* arrow down */
  82,6,   /* Insert */
  83,7,   /* Delete */

  97,21,  /* Undo */
  98,22,  /* Help */

  99,'(',  /* from now on numeric keypad */
  100,')',
  101,'/',
  102,'*',
  103,'7',
  104,'8',
  105,'9',
  106,'4',
  107,'5',
  108,'6',
  109,'1',
  110,'2',
  111,'3',
  112,'0',
  113,'.',
  114,13,  /* Enter */
  0,0
};
/*}}}  */
/*{{{  static char shift_key_tab[]*/
static char shift_key_tab[] =
{
  1,27,
  14,8,   /* Backspace */
  15,9,   /* Tab */
  71,1,   /* ClrHome */
  72,2,   /* arrow up */
  74,'-', /* - numeric keypad */
  75,3,   /* arrow left */
  77,4,   /* arrow right */
  78,'+', /* + numeric keypad */
  80,5,   /* arrow down */
  82,6,   /* Insert */
  83,7,   /* Delete */

  84,10,  /* Shift-F1 */
  85,11,
  86,12,
  87,14,
  88,15,
  89,16,
  90,17,
  91,18,
  92,19,
  93,20,  /* Shift-F10 */

  97,21, /* Undo */
  98,22, /* Help */

  99,'(',  /* from now on numeric keypad */
  100,')',
  101,'/',
  102,'*',
  103,'7',
  104,'8',
  105,'9',
  106,'4',
  107,'5',
  108,'6',
  109,'1',
  110,'2',
  111,'3',
  112,'0',
  113,'.', /* '.' on numeric keypad*/
  114,13,  /* Enter */
  0,0
};
/*}}}  */
/*{{{  static char ctrl_key_tab[]*/
static char ctrl_key_tab[] =
{
  1,27,
  2,'1',
  3,'2',
  4,'3',
  5,'4',
  6,'5',
  7,'6',
  8,'7',
  9,'8',
  10,'9',
  11,'0',
  12,'?',
  13,'`',
  14,8,
  15,9,
  23,'I',  /* distinguish CTRL-I and TAB */
  26,'@',  /* š */
  27,'+',
  28,13,   /* RETURN */
  35,'H',  /* distinguish CTRL-H and Backspace */
  39,'[',  /* ™ */
  40,']',  /* Ž */
  41,'#',
  43,'~',
  50,'M',  /* distinguish CTRL-M and RETURN */
  51,',',
  52,'.',
  53,'-',
  57,' ',
  59,'a', /* F1 */
  60,'b',
  61,'c',
  62,'d',
  63,'e',
  64,'f',
  65,'g',
  66,'h',
  67,'i',
  68,'j', /* F10 */
  71,'k', /* ClrHome */
  72,'l', /* arrow up */
  74,'_', /* - numeric keypad */
  75,'m', /* arrow left */
  77,'n', /* arrow right */
  78,'=', /* + numeric keypad */
  80,'o', /* arrow down */
  82,'p', /* Insert */
  83,'q', /* Delete */

  84,'a', /* Shift-F1 */
  85,'b',
  86,'c',
  87,'d',
  88,'e',
  89,'f',
  90,'g',
  91,'h',
  92,'i',
  93,'j',

  96,'<',
  97,'r', /* Undo */
  98,'s', /* Help */

  99,'(',  /* from now on numeric keypad */
  100,')',
  101,'/',
  102,'*',
  103,'H', /* 7 Ziff. */
  104,'I', /* 8 on mumeric keypad */
  105,'J', /* 9 on mumeric keypad */
  106,'E', /* 4 on mumeric keypad */
  107,'F', /* 5 on mumeric keypad */
  108,'G', /* 6 on mumeric keypad */
  109,'B', /* 1 on mumeric keypad */
  110,'C', /* 2 on mumeric keypad */
  111,'D', /* 3 on mumeric keypad */
  112,'A', /* 0 on mumeric keypad */
  113,':',
  114,';', /* Enter */
  115,'m', /* arrow left */
  116,'n', /* arrow right */
  119,'k', /* ClrHome */
  0,0
};
/*}}}  */
/*{{{  static char alt_key_tab[]*/
static char alt_key_tab[] =
{
  1,27,
  2,'1',
  3,'2',
  4,'3',
  5,'4',
  6,'5',
  7,'6',
  8,'7',
  9,'8',
  10,'9',
  11,'0',
  12,'?',
  13,'`',
  14,8,
  15,9,
  16,'Q',
  17,'W',
  18,'E',
  19,'R',
  20,'T',
  21,'Z',
  22,'U',
  23,'I',
  24,'O',
  25,'P',
  27,'+',
  28,13,  /* RETURN */
  30,'A',
  31,'S',
  32,'D',
  33,'F',
  34,'G',
  35,'H',
  36,'J',
  37,'K',
  38,'L',
  41,'#',
  43,'~',
  44,'Y',
  45,'X',
  46,'C',
  47,'V',
  48,'B',
  49,'N',
  50,'M',
  51,',',
  52,'.',
  53,'-',
  57,' ',
  59,'a', /* F1 */
  60,'b',
  61,'c',
  62,'d',
  63,'e',
  64,'f',
  65,'g',
  66,'h',
  67,'i',
  68,'j', /* F10 */
  74,'_', /* - on mumeric keypad */
  78,'=', /* + on mumeric keypad */
  83,'q', /* Delete */

  84,'a', /* Shift-F1 */
  85,'b',
  86,'c',
  87,'d',
  88,'e',
  89,'f',
  90,'g',
  91,'h',
  92,'i',
  93,'j',

  96,'<',
  97,'r', /* Undo */
  98,'s', /* Help */

  99,'(',  /* from now on numeric Keypad */
  100,')',
  101,'/',
  102,'*',
  103,10, /* 7 */
  104,11, /* 8 */
  105,12, /* 9 */
  106,5, /* 4 */
  107,6, /* 5 */
  108,7, /* 6 */
  109,2, /* 1 */
  110,3, /* 2 */
  111,4, /* 3 */
  112,1, /* 0 */
  113,':',
  114,';', /* Enter */
  120,'1',
  121,'2',
  122,'3',
  123,'4',
  124,'5',
  125,'6',
  126,'7',
  127,'8',
  128,'9',
  129,'0',
  130,'?',
  131,'`',
  0,0
};
/*}}}  */
/*{{{  static char menu_tab[]*/
static char menu_tab[]=
{
  M_QUIT,'b',   /* quit */
  M_FASTX,'c',  /* fast exit */
  M_READF,'d',  /* read file */
  M_WRITEF,'e', /* write file */
  M_SAVE,'f',   /* save current file */
  M_INSERT,'g', /* insert file */
  M_SHELL,'h',  /* TOS shell */
  M_SHCMD,'i',  /* shell command */
  M_DOUBLE,'j', /* double line */
  M_MOVE,'k',   /* move line */
  M_PICK,'l',   /* pick line */
  M_CPPICK,'m', /* copy-pick line */
  M_PASTE,'n',  /* paste pick buffer */
  M_DELLIN,'o', /* delete line */
  M_UNDLIN,'p', /* undo delete line */
  M_GOTOLI,'q', /* goto line */
  M_TOPFOL,'r', /* goto top of fold */
  M_BOTFOL,'s', /* goto bottom of fold */
  M_FIND,'t',   /* search */
/*  M_FINDSA,'u',   not used any more */
  M_FINDRE,'v', /* search reverse */
  M_REPLAC,'w', /* replace string */
  M_QUREPL,'x', /* query-replace string */
  M_OPFOLD,'y', /* open fold */
  M_CLFOLD,'z', /* close fold */
  M_ENTERF,'A', /* enter fold */
  M_EXITFO,'B', /* exit fold */
  M_UNFOLD,'C', /* unfold fold */
  M_CREAFO,'D', /* create fold */
  M_CRAUFO,'E', /* auto-create fold */
  M_AHEADR,'F', /* auto-header */
  M_FILEFO,'G', /* toggle create file-fold */
  M_ATTFIL,'H', /* toggle attach file to fold */
  M_DESFOL,'I', /* describe fold */
  M_DEFMAC,'J', /* define macro */
  M_EXEMAC,'K', /* execute macro */
  M_FIXMAC,'a', /* define fix macro */
  M_EXE1,'P',   /* execute fix macro 1 */
  M_EXE2,'Q',   /* execute fix macro 2 */
  M_EXE3,'R',   /* execute fix macro 3 */
  M_EXE4,'S',   /* execute fix macro 4 */
  M_EXE5,'T',   /* execute fix macro 5 */
  M_HELP,'L',   /* help */
  M_DESBIN,'M', /* describe bindings */
  M_SETLAN,'N', /* set language */
/*  M_VIEWMO,'O', not used any more */
  0,0  /* end of list */
};
/*}}}  */
/*}}}  */
 
/*{{{  void about_origami(void) - display 'ABOUT' dialog box*/
void about_origami(void)
{
  int x,y,w,h;
  int result;
 
  form_center(rs_trindex[ABOUT],&x,&y,&w,&h);
  form_dial(FMD_START,x,y,w,h,x,y,w,h);
  objc_draw(rs_trindex[ABOUT],0,10,x,y,w,h);
  result = form_do(rs_trindex[ABOUT],0);
  form_dial(FMD_FINISH,x,y,w,h,x,y,w,h);
  rs_trindex[ABOUT][result&0x7FFF].ob_state&=~SELECTED; /* de-select exit object */
}
/*}}}  */
 
/*{{{  void cursor(unsigned int xy[]) - display or remove cursor*/
void cursor(unsigned int xy[])
{
  vswr_mode(handle,MD_XOR);
  vsf_color(handle,1);
  vr_recfl(handle,(int *)xy);
  vswr_mode(handle,MD_REPLACE);
  vsf_color(handle,0);
}
/*}}}  */
 
/*{{{  int get_raw_key(void) handle GEM menu events, keyboard, alarm, cursor ...*/
/*{{{  notes about this function and the ST-specific key binding*/
/*
   This function returns raw ST keyboard events. There are two types of
   results:
   - 'normal' key codes - characters, RETURN, TAB, normal CTRL-Codes etc.
     These are the same as Bconin(2) delivers.
   - Meta-sequences. These consist of a Meta-character (#defined at the
     beginning of this file) and a second character, describing the key.
     There are four Meta-characters which are used depending on the state
     of the Shift, Ctrl and ALT keys. They are checked in the following
     order: ALT, then CTRL, then SHIFT, then normal. No combinations are
     allowed, this means CTRL-ALT-A returns the same sequence as ALT-A.
     The xxx_key_tabs above are used to translate Bios scancodes to meta
     codes. They are set up so that nearly every ST key gives a different
     code with SHIFT, CTRL and ALT - so there should be no need to change
     anything in order to implement your custom keybinding.
     The second character after the META character can be either of two
     types: CTRL-code or normal ASCII character.
     Keys with a normal letter on them (e.g. the numeric keypad) return
     a ASCII character, so if the combination META-Key is not used in the
     keybinding, the META is ignored and you get the normal character
     (otherwise, the key would have no function - not so useful).
     Keys like the function keys F1-F10, cursor control etc. return
     control characters after the META. If those keys are not used in the
     keybinding, both the META and the control code are suppressed (other-
     wise, the control code would result in some function which does not
     stand in any aparent correlation to the key. HELP for example would
     result in a single CTRL-V if only the META would be suppressed. CTRL-V
     might cause a Page Down command (EMACS-like keybinding) or something
     worse)

     Note that most combinations of CTRL+character result in the normal CTRL-
     code, e.g. 0x04 for CTRL-D. Some keys are reported as CMETA,character,
     e.g. CTRL-I results in CMETA,'I' so they can be distinguished from the
     TAB key which also generates 0x09=CTRL-I code. Same is true for 'H'
     (Backspace), 'M' (RETURN). '1'-'0' and all special keys (cursor, numeric
     keypad etc.) also create CMETA-sequences.

     Note also that some keys can not be used together with ALT:
     HELP (because of the hardcopy function), the cursor keys , INSERT and
     CLR/HOME (GEM mouse emulation),
*/
/*}}}  */
unsigned int get_raw_key(void)
{
  /*{{{  variables*/
    char *c;
    int state,scancode;
    int mbuf[8],mx,my,mbutton,key,numclicks,result;
    unsigned int xyarray[4];
    bool cursor_flag;
    static unsigned int last_menu_title = 0;
    static unsigned int first_redraw_request = TRUE;
  /*}}}  */
 
  /*{{{  escape code: return second character*/
    if (sys_kbd_flag)     /* escape - return second character */
    {
      sys_kbd_flag = 0;
      return sys_kbd_save;
    }
  /*}}}  */
 
  /*{{{  set up xyarray[] for cursor area*/
    xyarray[0] = _x*gl_wchar+wind_x;
    xyarray[1] = _y*gl_hchar+wind_y;
    xyarray[2] = xyarray[0]+gl_wchar-1;
    xyarray[3] = xyarray[1]+gl_hchar-1;
  /*}}}  */
 
  /*{{{  display cursor*/
  cursor(xyarray);
  cursor_flag = TRUE;
  /*}}}  */
  /*{{{  if menu title is still inverted from previous selection: normal it*/
  if (last_menu_title)
  {
    menu_tnormal(rs_trindex[MENU],last_menu_title,1);
    last_menu_title = 0;
  }
  /*}}}  */
  /*{{{  main event loop: handle Keyboard/message events, alarm, abort, cursor*/
  evnt_loop:
  do
  {
    graf_mouse(M_ON,NULL);
    wind_update(END_UPDATE);
    /*{{{  result = evnt_multi(MU_KEYBD|MU_MESAG|MU_TIMER, ...*/
    result = evnt_multi(MU_KEYBD|MU_MESAG|MU_TIMER, 0,
                         0,0,
                         0,0,
                         0,0,
                         0,0,
                         0,0,
                         0,0,
                         mbuf, 400,
                         0, &mx,
                         &my, &mbutton,
                         &state, &key,
                         &numclicks );
    /*}}}  */
    wind_update(BEG_UPDATE);
    graf_mouse(M_OFF,NULL);
    /*{{{  timer event: blink cursor, check for alarm*/
    if (result & MU_TIMER)
    {
      long   ssp;
    
      cursor(xyarray);
      cursor_flag =!cursor_flag;
    
      ssp = Super(NULL);         /* switch to supervisor mode */
      if (sys_alarmtime != 0L && sys_alarmtime < (*(unsigned long *)0x4BA)) /* alarm time has come */
      {
        got_alarm();
        sys_alarmtime = 0L;
      }
      Super ((void *) ssp);      /* return to normal mode */
    }
    /*}}}  */
  } while (((result & (MU_MESAG|MU_KEYBD)) == 0) && (state & 3) != 3); /* abort if both shift keys are pressed */
  /*{{{  if necessary, remove cursor*/
  if (cursor_flag)
  {
    cursor(xyarray);
    cursor_flag = FALSE;
  }
  /*}}}  */
  if (result & MU_MESAG)
  /*{{{  handle message events*/
  {
    switch(mbuf[0])
    {
      case MN_SELECTED:
      /*{{{  handle menu events, display ABOUT dialog box*/
      {
        if(mbuf[4] == M_ABOUT)
        {
          wind_update(END_UPDATE);
          graf_mouse(M_ON,NULL);
          about_origami();
          wind_update(BEG_UPDATE);
          graf_mouse(M_OFF,NULL);
          menu_tnormal(rs_trindex[MENU],mbuf[3],1);
          goto evnt_loop;
        }
        last_menu_title = mbuf[3];
        c = menu_tab;
        while (*c != 0)
        {
          if (mbuf[4] == *c)
          {
            sys_kbd_flag = 1;
            sys_kbd_save = *++c;
            return MMETA;
          }
          c += 2;
        }
        break;
      }
      /*}}}  */
      case WM_REDRAW:
      /*{{{  handle window redraw request*/
        if (first_redraw_request)
        {
          first_redraw_request = FALSE;
          goto evnt_loop;
        }
        else
        {
          sys_kbd_flag = 1;
          sys_kbd_save = '1';
          return MMETA;
        }
      /*}}}  */
    }
  }
  /*}}}  */
  if ((state & 3) == 3) return 0x7FFF;  /* both SHIFT keys pressed -> ABORT */
  
  if (!result & MU_KEYBD) /* only non-Keyboard events -> wait */
    goto evnt_loop;
  /*}}}  */
  scancode = (key>>8) & 0xFF;

  /*{{{  ALT pressed -> search alt_key_tab*/
  if (state & 8)  /* ALT ? */
  {
    c = alt_key_tab;
    while (*c != 0)
    {
      if (scancode == *c)
      {
        sys_kbd_flag = 1;
        sys_kbd_save = *++c;
        return AMETA;
      }
      c += 2;
    }
  }
  /*}}}  */
  /*{{{  CTRL pressed -> search ctrl_key_tab*/
  if (state & 4)  /* CTRL ? */
  {
    c = ctrl_key_tab; /* same tab as ALT */
    while (*c != 0)
    {
      if (scancode == *c)
      {
        sys_kbd_flag = 1;
        sys_kbd_save = *++c;
        return CMETA;
      }
      c += 2;
    }
  }
  /*}}}  */
  /*{{{  SHIFT pressed -> search shift_key_tab*/
  if (state & 3)  /* SHIFT ? */
  {
    c = shift_key_tab;
    while (*c != 0)
    {
      if (scancode == *c)
      {
        sys_kbd_flag = 1;
        sys_kbd_save = *++c;
        return SMETA;
      }
      c += 2;
    }
  }
  /*}}}  */
  /*{{{  search norm_key_tab for special keys*/
  c = norm_key_tab;
  while (*c != 0)
  {
    if (scancode == *c)
    {
      sys_kbd_flag = 1;
      sys_kbd_save = *++c;
      return META;
    }
    c += 2;
  }
  /*}}}  */
 
/* nothing special found - return normal ASCII code */
  return (int) (key & 0xFF);
}
/*}}}  */
 
/*{{{  get_key (called by ORIGAMI) - get a keyboard token*/
#ifdef STD_C
TOKEN get_key (void)
#else
TOKEN get_key ()
#endif
{
  /*{{{  variables*/
  int result=1;
  unsigned int ch, lastch;
  int lg=0;
  int cp=0;
  /*}}}  */
 
  sys_kbd_flag = 0;
  ch = 0;
  /*{{{  scan a keyboard-sequence 'til token is complete*/
  while (result==1 && !aborted)
  {
    lastch = ch;
    ch = get_raw_key();
    lg++;
    result=find_key(ch);
    /*{{{  echo-handling*/
    if (echo && result==1) {
      /*{{{  append code to string*/
      if (ch>=' ' && ch<MMETA) {
        /*{{{  x*/
        command[cp++]=ch;
        command[cp++]=' ';
        /*}}}  */
      } else if (ch=='\033') {
        /*{{{  M-*/
        command[cp++]='M';
        command[cp++]='-';
        /*}}}  */
      } else if (ch>0 && ch<' ') {
        /*{{{  C-x*/
        command[cp++]='C';
        command[cp++]='-';
        command[cp++]='@'+ch;
        command[cp++]=' ';
        /*}}}  */
      } else if (ch>=MMETA && ch<=META) {
        /*{{{  X-*/
        command[cp++]='X';
        command[cp++]='-';
        /*}}}  */
      } else {
        /*{{{  ?*/
        command[cp++]='?';
        command[cp++]=' ';
        /*}}}  */
      }
      command[cp]='\0';
      /*}}}  */
      message(command,FALSE);
    }
    /*}}}  */
  }
  /*}}}  */
  /*{{{  if aborted -> return O_BREAK*/
  if (aborted)
  {
    sys_kbd_flag = 0;
    return O_BREAK;
  }
  /*}}}  */
  if (result) return (result);
  /*{{{  handle abort key*/
  if (ch==general_abort_key || ch==0x7FFF) {
    aborted=TRUE;
    return(O_BREAK);
  }
  /*}}}  */
  if (lg==1 && ch>=' ' && ch < MMETA)
    return(ch);
  /* normal META sequence: return second character (for numeric keypad) */
  if (lg == 2 && lastch == META && ch>=' ' && ch<=MMETA)
    return ch;
  message(M_NOT_BOUND, TRUE);
  return(O_NOP);
}
/*}}}  */
