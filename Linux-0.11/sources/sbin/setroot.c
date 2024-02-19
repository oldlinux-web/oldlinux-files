From: hns@regent.e-technik.tu-muenchen.dbp.de (Henning Spruth)
Newsgroups: alt.os.linux
Subject: DOS program to set root device
Date: 25 Mar 92 09:06:28 GMT

Here is a small DOS program I hacked using Turbo C to be able to
conveniently set the root partition. It is quite primitive, as it
requires major/minor device numbers and can only access a boot disk in
drive A:, but it works. Using it, installing Linux can be done in
these steps:

      1. boot the Linux boot disk
      2. make HD file system
      3. run install script
      4. sync
      5. boot DOS
      6. run setroot.exe to set root file system to HD
      7. boot the Linux boot disk again

Here's the source:

------------------------- begin setroot.c ------------------------------
#include <stdio.h>
#include <bios.h>
#include <conio.h>

char buffer[512];

void check(int res)
{
  if(res==0) return;
  printf("Disk error status: %x\n",res);
  exit(1);
}

main()
{
  int result,x;
  char in[80];
  unsigned int major, minor;
  printf("\n\nInsert Linux boot disk into drive A: and hit a key ...");
  getch();
  printf("\n");
  printf("Resetting controller ...\n");
  result=biosdisk(0,0,0,0,1,1,buffer);
  check(result);
  printf("Loading boot sector ...\n");
  result=biosdisk(2,0,0,0,1,1,buffer);
  if(result==6) result=biosdisk(2,0,0,0,1,1,buffer);
  check(result);
  printf("Current major boot device: %d\n",buffer[509]);
  printf("Current minor boot device: %d\n",buffer[508]);
  printf("\n");
  printf("New device values:\n");
  printf("Enter new major device: ");
  gets(in);
  sscanf(in,"%d",&major);
  printf("Enter new minor device: ");
  gets(in);
  sscanf(in,"%d",&minor);
  printf("\nCheck: changing root disk to major %d, minor %d\n",major,minor);
  printf("Is this OK (y/n)? ");
  x=getch(); printf("\n");
  if(x!='y')
  { printf("Aborting ...\n");
    exit(0);
  }
  buffer[508]=minor;
  buffer[509]=major;
  printf("Writing boot sector ...\n");
  result=biosdisk(3,0,0,0,1,1,buffer);
  check(result);
  printf("Root device changed!\n");
}
------------------------- end setroot.c ------------------------------


Here is the uuencoded executable:

------------------------- begin setroot.uue ----------------------------
begin 664 setroot.exe
M35KP 1D  P @    ___W H          /@    $ ^S!J<@              
M                       !    W@   ! !                        
M                                                            
M                                                            
M                                                            
M                                                            
M                                                            
M                                                            
M                                                            
M                                                            
M                                                            
M                      "Z;0(NB18U K0PS2&++@( BQXL ([:HY  C :.
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
MU1(             5'5R8F\@0RLK("T@0V]P>7)I9VAT(#$Y.3 @0F]R;&%N
M9"!);G1L+@!.=6QL('!O:6YT97(@87-S:6=N;65N= T*1&EV:61E(&5R<F]R
M#0I!8FYO<FUA;"!P<F]G<F%M('1E<FUI;F%T:6]N#0H                 
M                                 )X(G@@               !$:7-K
M(&5R<F]R('-T871U<SH@)7@*  H*26YS97)T($QI;G5X(&)O;W0@9&ES:R!I
M;G1O(&1R:79E($$Z(&%N9"!H:70@82!K97D@+BXN  H 4F5S971T:6YG(&-O
M;G1R;VQL97(@+BXN"@!,;V%D:6YG(&)O;W0@<V5C=&]R("XN+@H 0W5R<F5N
M="!M86IO<B!B;V]T(&1E=FEC93H@)60* $-U<G)E;G0@;6EN;W(@8F]O="!D
M979I8V4Z("5D"@ * $YE=R!D979I8V4@=F%L=65S.@H 16YT97(@;F5W(&UA
M:F]R(&1E=FEC93H@ "5D $5N=&5R(&YE=R!M:6YO<B!D979I8V4Z(  E9  *
M0VAE8VLZ(&-H86YG:6YG(')O;W0@9&ES:R!T;R!M86IO<B E9"P@;6EN;W(@
M)60* $ES('1H:7,@3TL@*'DO;BD_(  * $%B;W)T:6YG("XN+@H 5W)I=&EN
M9R!B;V]T('-E8W1O<B N+BX* %)O;W0@9&5V:6-E(&-H86YG960A"@      
M(" @(" @(" @(2$A(2$@(" @(" @(" @(" @(" @(" !0$! 0$! 0$! 0$! 
M0$!  @(" @(" @(" D! 0$! 0$ 4%!04%!0$! 0$! 0$! 0$! 0$! 0$! 0$
M!$! 0$! 0!@8&!@8& @(" @(" @(" @(" @(" @(" @(0$! 0"          
M                                                            
M                                                            
M                                          !4!50%5 4   D"    
M         $@#   * @$           !8 P   @("            : ,  $,"
M P           '@#  !" @0           "( P    #_            F , 
M    _P           *@#     /\           "X P    #_            
MR ,     _P           -@#     /\           #H P    #_        
M    ^ ,     _P            @$     /\            8!     #_    
M        * 0     _P           #@$     /\           !(!     #_
M            6 0     _P           &@$     /\           !X! $@
M B "( 2@ J#_______________________________________\   T     
M   0 0                     ! $-/35!!40       !," @0%!@@("!05
M!1/_%@41 O________________\%!?____________________\/_R,"_P__
M____$___ @(%#P+___\3__________\C_____R/_$_\   (" @(" @(" 0$!
M 0$" @(" @(" @(" @(" @(" @(! @(" @," @("! (" @("!04%!04%!04%
M!0(" @(" @(" @('"A4*# D" @L"% X" @(" @@" A(" A "$ (" @("!@<*
M"@H,"0("#0(1#A," @\"" ("$@(" @(" @(H;G5L;"D  !04 105%!04% ( 
M% ,$% D%!04%!04%!044%!04%!04%!04% \7#P@4%!0'%!84%!04%!04%!0-
M%!04%!04%!04%! *#P\/" H4% 84$@L.%!01% P4% T4%!04%!04 '!R:6YT
M('-C86YF(#H@9FQO871I;F<@<&]I;G0@9F]R;6%T<R!N;W0@;&EN:V5D#0H 
M        $"05)!4D%20                                         
M                                                            
M                                                            
M                                                            
M                                                            
M                                                            
M                                                            
M                                                            
M                                                            
M                                                            
M                                                            
M                                                            
M                                                            
M                                                            
M                                                            
M                                                            
M     /M2"0(E!@  O  B    B !_ #,    $ "<  0 !  $ \#$        !
M          $    / 0    (    N 0    ,   "H 0    0    @ @    4 
M   U @    8    W @    < (@ Y @  & @ (0!: @  & D   #G P    H 
M   +!     L    ^!0    P '@!5!0    T   !X!0    X   #Z!0    \ 
M  "6!@   !    #X!@   !$ &P Y!P   !(   "@!P   !,   #O!P   !0 
M   G"    !4 &0#-"    !8   #D"    !<   #^"    !@     "@   !D 
M   4"@   !H   !&"P   !L   ""#    !P   "_#    !T    @#0   !X 
M  "*#@   !\   #O#@   "  'P#D#P   "$    "$    "(    ?$    ", 
M   W$    "0   !!$    "4   !0$    "8   !C$    "<   !2$0   "@ 
M&@!S$0   "D   "J$0   "H   #,$0   "L   #]$0   "P    )$@   "T 
M  !1$@   "X   #5$@   "\   #U$@   #    #/$P   #$   "U%    #( 
M  #)%    #,   #)%    #0   !L%0   #4 '0!U%0   #8   ".%0   #< 
M  ".%0   #@   "K%0   #D   "K%0   #H   #'%0   #L    $%@   #P 
M   5%@   #T    ^%@   #X   "[%@   #\   #B%@   $    #\%@   $$ 
M   A%P   $(   !"%P   $,   #W%P   $0    1&    $4   #/'    $8 
M  !F'@   $<    J(P   $@   !0(P   $D   #)(P   $H    O)    $L 
M   S)    $P    W)    $T    [)    $X   ! )    $\    /)0   %  
M  !F)@   %$      &T" %(   !R &T" %,   !V &T" %0   !Z &T" %4 
M  !^ &T" %8   "" &T" %<   "$ &T" %@   "& &T" %D   "( &T" %H 
M  "* &T" %L   ", &T" %P   ". &T" %T   "0 &T" %X   "0 &T" %\ 
M  "1 &T" &    "2 &T" &$   "4 &T" &(   "8 &T" &,   ": &T" &0 
M  "< &T" &4   "@ &T" &8   "D &T" &<    ^ FT" &@   !  FT" &D 
M  !" VT" &H   !$ VT" &L   !& VT" &P   !( VT" &T   "(!&T" &X 
M  "P!&T" &\   "X!&T" '    "Z!&T" '$   "\!&T" '(   #+!&T" ', 
M  #6!&T" '0   #8!&T" '4   !,!FT" '8   !.!FT" '<   !0!FT" '@ 
M  !2!FT" 'D   !4!FT" 'H ( !:!FT" 'L   !:"&T" 'P         !7T 
M        !7X    ,    !7\     $   !80 !  $    "H0 !  $    "H4 
M"@#\_P   H8 "@#^_P   H< & "L_P   H@ !  '    !(D !  &    !(H 
M"@    $ !HL !@    $ !H$                   "#  $)AP "  $  0 !
M  $ C                    (T                   ".            
M        CP                   )                    "1        
M            D@                   ),                   "4    
M                E0                   )8                   "7
M                    F                    )D                 
M  ":                    FP                   )P             
M      "=                    G@                   )\         
M          "@                    H0                   *(     
M              "C                    I                    *4 
M                  "F                    IP                  
M *@                   "I                    J@              
M     *L                   "L                    K0          
M         *X                   "O                    L       
M             +$                   "R                    LP  
M                 +0                   "U                    
MM@                   +<                   "X                
M    N0                   +H                   "[            
M        O                    (( JAIR& < .0() #P""@!$ @L 4 (,
M %@"#@!: A, 8@(4 &H"%0!M A8 =0(7 'T"& "= AD H@(: *H"&P#+ AP 
M\0(= /8"'@ $ Q\ $@,@ !H#(0 B R( *@,C #(#) !$ R4 3 ,F %0#)P!F
M R@ =@,I 'X#*@"+ RL D ,L )@#+@"? R\ I0,P *L#,0"S S( U ,S -D#
M- #A X   0      .0(A ($  0 !    / (>            6@*- 8( !0 #
M    8@*% 0(    Y JX! 0 $  $  0 !  $  0 G            !    0  
M @" ____?P    4   (   0  (#___]_   &   $   &     (#___]_"   
M 0  "       _P    D   (   H      /__   *   $   ,      #_____
M#0  !      /   (     !    H     #@  !@     H   !      P   $ 
M !,      /\    '   (      L   @     *P  "@     :  !0   " ", 
M    @ 0 (P      !  C       < !4   ( ! ( (P      !  C       $
M ",     @ 0 &@    (  @ C       $ ",       $ 7U]E>&ET8VQE86X 
M7U]E>&ET %]?<F5S=&]R97IE<F\ 7V%B;W)T $1'4D]54$  7U]-34]$14P 
M7V-H96-K %]M86EN %]A=&5X:70 7U]#4%543@!?8W!R:6YT9@!?97AI= !?
M9F9L=7-H %]F;'5S:&%L; !?9G-E96L 7V9T96QL %]G971S %]G971T97AT
M %]P=71T97AT %]M;W9E=&5X= !?<')I;G1F %]?9G!U=&, 7V9P=71C %]F
M<'5T8VAA<@!?7T90551. %]P=71C: !?7U-#4D5%3DE/ %]?5D%,241!5$58
M60!?7U-#4D],3 !?7W-E='5P:6\ 7W-E='9B=68 7W-S8V%N9@!?=G-S8V%N
M9@!?=&5L; !?7W=H97)E>'D 7W=H97)E> !?=VAE<F5Y %]W<FET90!?7WAF
M9FQU<V@ 7V)I;W-D:7-K %]?7V)R:P!?7U]S8G)K %]B<FL 7W-B<FL 7U]6
M:61E;TEN= !?7V,P8W)T:6YI= !?7V-R=&EN:70 7V5O9@!?7V9G971C %]?
M3F9G971C %]F9V5T8P!?9F=E=&-H87( 7V=E=&-H %]?3F=E=&-H90!?9V5T
M8VAE %]?3G5N9V5T8V@ 7W5N9V5T8V@ 7U])3T524D]2 %]I<V%T='D 7VQS
M965K %]?3$].1U1/00!?:71O80!?=6QT;V$ 7VQT;V$ 7VUE;6-P>0!?<F5A
M9 !?7W)E860 7U]S8V%N;F5R %]?<V-A;G1O; !?7U904DE.5$52 %]?5E!4
M4@!?7U9204T 7U]W<FET90!?7U)%04Q#5E0 7U]S8V%N=&]D %]?<V-A;G)S
M;'0 7U]S8V%N<&]P %]F<F5E %]M86QL;V, 7W)E86QL;V, 1$%405-%1T  
M7U]);G0P5F5C=&]R %]?26YT-%9E8W1O<@!?7TEN=#5696-T;W( 7U]);G0V
M5F5C=&]R %]?0S!A<F=C %]?0S!A<F=V %]?0S!E;G9I<F]N %]?96YV3&YG
M %]?96YV<V5G %]?96YV4VEZ90!?7W!S< !?7V]S;6%J;W( 7U]V97)S:6]N
M %]?;W-M:6YO<@!?97)R;F\ 7U]3=&%R=%1I;64 7U]?:&5A<&)A<V4 7U]?
M8G)K;'9L %]?:&5A<&)A<V4 7U]B<FML=FP 7U]H96%P=&]P %]?871E>&ET
M8VYT %]?8W1Y<&4 7U]E>&ET8G5F %]?97AI=&9O<&5N %]?97AI=&]P96X 
M7U]S=')E86US %]?;W!E;F9D %]?:&5A<&QE;@!?7W-T:VQE;@!?7W=S8W)O
M;&P 7U]V:61E;P!?9&ER96-T=FED96\ 7U]D;W-E<G)N;P!?7V1O<T5R<F]R
M5&]35@!?7V9I<G-T %]?;&%S= !?7W)O=F5R %]?4F5A;$-V=%9E8W1O<@!?
M7U-C86Y4;V1696-T;W( 7V)U9F9E<@!?7V%T97AI='1B; !?7W1U<F)O0W)T
M %]?8W9T9F%K %]?04A32$E&5 !?7T%(24Y#4@!#,"Y!4TT 0S!3 %-%5%)/
M3U0N0P!315123T]4 ')E<P!M:6YO<@!M86IO<@!I;@!X ')E<W5L= !S:7IE
M7W0 9G!O<U]T $%415A)5 !#4%))3E1& $-465!% $58250 1D9,55-( $9)
M3$53 $9)3$53,@!&3%532$%,3 !&4T5%2P!'1513 $=05$585 !(14%03$5.
M $U/5D5415A4 %!224Y41@!0551# %!55$-( %-#4D5%3@!30U)/3$P 4T54
M55!)3P!315160E5& %-30T%.1@!35$M,14X 5$5,3 !72$5215A9 %=2251%
M %=30U)/3$P 6$9&3%532 !"24]31$E32P!"4DL 0U)424Y)5 !%3T8 1T54
M0P!'151#2 !)3T524D]2 $E3051460!,4T5%2P!,5$]! $U%34-060!214%$
M %)%041! %-#04Y.15( 4T-!3E1/3 !64%))3E1%4@!64D%- %=2251%00!#
?5E1&04L 4D5!3$-65 !30T%.5$]$ $Y%05)(14%0  !6
 
end
------------------------- end setroot.uue ----------------------------


If there is some demand, I can extend the program to use 
standard HD device names instead of major/minor device numbers.

Enjoy,

Henning


------------------------------------------------------------------------
Henning Spruth		 	  spruth@regent.e-technik.tu-muenchen.de
Institute of Electronic Design Automation
Technical University of Munich, Germany
--
------------------------------------------------------------------------
Henning Spruth		 	  spruth@regent.e-technik.tu-muenchen.de
Institute of Electronic Design Automation
Technical University of Munich, Germany

