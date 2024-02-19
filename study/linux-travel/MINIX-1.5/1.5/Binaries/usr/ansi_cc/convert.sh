
Here are the sed-script and a short description of the changes between the
old and the new assembler.  The sed-script does not work under MINIX because
of the \< and \> patterns. Furthermore, it doesn't change ! into ~.


s/|/!/
s/\([ 	]*\.\)byte\>/\1data1/
s/\([ 	]*\.\)word\>/\1data2/
s/\([ 	]*\.\)long\>/\1data4/
s/^\([ 	]*\.\)globl\>/\1extern/
s/\<b\([xp]\)_\([sd]\)i\>/b\1),(\2i/
s/^\([ 	]*\)seg\>[ 	]*\([ecsd]\)s\>/\1\2seg/
s/^\([ 	]*\)in\>/\1inb/
s/^\([ 	]*\)out\>/\1outb/
s/^\([ 	]*\)stob\>/\1stosb/
s/^\([ 	]*\)stow\>/\1stos/
s/^\([ 	]*\)lodb\>/\1lodsb/
s/^\([ 	]*\)lodw\>/\1lods/
s/^\([ 	]*\)scab\>/\1scasb/
s/^\([ 	]*\)scaw\>/\1scas/
s/^\([ 	]*\)jc\>/\1jb/
s/^\([ 	]*\)jnc\>/\1jnb/
s/^\([ 	]*\)calli\>/\1callf/
s/^\([ 	]*\)jmpi\>/\1jmpf/
s/^\([ 	]*\)reti\>/\1retf/
s/^\([ 	]*\)j\>/\1jmp/
s/^\([ 	]*\)br\>/\1jmp/
s/^\([ 	]*\)beq\>/\1je/
s/^\([ 	]*\)bge\>/\1jge/
s/^\([ 	]*\)bgt\>/\1jg/
s/^\([ 	]*\)bhi\>/\1jnbe/
s/^\([ 	]*\)bhis\>/\1jnb/
s/^\([ 	]*\)ble\>/\1jle/
s/^\([ 	]*\)blo\>/\1jb/
s/^\([ 	]*\)blos\>/\1jbe/
s/^\([ 	]*\)blt\>/\1jnge/
s/^\([ 	]*\)bne\>/\1jne/
----
Changes between the old and the new assembler:


+    Files  beginning  with  a  '#'  are  preprocessed.  The
     preprocessor  is  not a  general  preprocessor,  but an
     ANSI-C pre-processor, so some old tricks and some  con-
     structions may not work.  An example is the #, which is
     now an preprocessor-operator.

+    The comment symbol is now '!' instead of '|'. The symbol
     for the not-operator is now '~' instead of '!'.

+    The .byte, .word and .long directives  are  now  called
     .data1, .data2 and .data4 repectively.

+    The  adressing  modes  (bx_si),  (bx_di),  (bp_si)  and
     (bp_si) are now written as (bx)(si), (bx)(di), (bp)(si)
     and (bp)(si) respectively.

+    The mapping of old instructions to new ones is given in
     the following table.

               ___________________________________
               |old instruction   new instruction|
               |in                inb            |
               |out               outb           |
               |stob              stosb          |
               |lodb              lodsb          |
               |lodw              lodsw          |
               |scab              scasb          |
               |scaw              scas           |
               |jc                jb             |
               |jnc               jnb            |
               |calli             callf          |
               |jmpi              jmpf           |
               |reti              retf           |
               |j                 jmp            |
               |br                jmp            |
               |beq               je             |
               |bge               jge            |
               |bgt               jg             |
               |bhi               jnbe           |
               |bhis              jnb            |
               |ble               jle            |
               |blo               jb             |
               |blos              jbe            |
               |blt               jnge           |
               |bne               jne            |
               |_________________________________|


     Be aware that some instructions are  still  valid,  but
     now  have  a different meaning. Examples are the in and
     out intructions, which will work on  words  instead  of
     bytes.

+    The assembler knows i80286 and i80x87 instructions.
----
