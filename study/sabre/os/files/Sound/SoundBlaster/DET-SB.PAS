From : Ryan Nathan Thompson
Subj : SB autodetect
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

  Procedure SndWrite(Register, Data : Byte); assembler;
    Asm
      mov   al, register
      mov   dx, SndPort                { read register }
      out   dx, al
      in    al, dx; in    al, dx; { add a few more for the needed delay time }
      mov   al, data
      inc   dx                         { write register }
      out   dx, al
      dec   dx
      in    al, dx; in    al, dx; { add more for a proper delay }      
    End;

  Function SndRead : Byte; assembler;
    Asm
      mov   dx, SndPort
      in    al, dx
    End;

  Procedure WriteDSP(Command : Byte); assembler;
    Asm
      mov    cx, $ff
      mov    dx, DSPWrite
    @Wait1: 
      in     al, dx
      or     al, al
      jns    @Wait2
      loop   @Wait1
    @Wait2:  
      mov    al, Command
      out    dx, al
    End;

  Procedure CheckIRQ; assembler;
    Var
      Old2, Old3, Old5, Old7, Old9, Old10, Old11, Old12 : Pointer;
    Asm
    @ScanIRQ:
      cli
      in     al, $21                   { save the IMR. PIC1 }
      mov    bl, al
      in     al, $a1                   { save the IMR. PIC2 }
      mov    bh, al
      mov    al, $ff                   { disable all the IRQs. }
      out    $21, al                   { PIC1 }
      out    $a1, al                   { PIC2 }
      xor    ax, ax                    { trap the IRQs 2,3,5,7,10,11,12. }
      mov    es, ax
    @SaveIrqs:
      mov    ax, [es:$28]              { irq2 }      mov    dx, [es:$2A]
      push   ax      push   dx
      mov    ax, [es:$2C]              { irq3 }      mov    dx, [es:$2E]
      push   ax      push   dx
      mov    ax, [es:$34]              { irq5 }      mov    dx, [es:$36]
      push   ax      push   dx
      mov    ax, [es:$3C]              { irq7 }      mov    dx, [es:$3E]
      push   ax      push   dx
      mov    ax, [es:$134]             { irq9 }      mov    dx, [es:$136]
      push   ax      push   dx
      mov    ax, [es:$138]             { irq10 }     mov    dx, [es:$13a]
      push   ax      push   dx
      mov    ax, [es:$13c]             { irq11 }     mov    dx, [es:$13e]
      push   ax      push   dx
      mov    ax, [es:$140]             { irq12 }     mov    dx, [es:$142]
      push   ax      push   dx
    @SetIrqs:
      mov    ax, offset @TrapIrq2      { irq2 }
      mov    [es:$28], ax      mov    [es:$2A], cs
      mov    ax, offset @TrapIrq3      { irq3 }
      mov    [es:$2C], ax      mov    [es:$2E], cs
      mov    ax, offset @TrapIrq5      { irq5 }
      mov    [es:$34], ax      mov    [es:$36], cs
      mov    ax, offset @TrapIrq7      { irq7 }
      mov    [es:$3C], ax      mov    [es:$3E], cs
      mov    ax, offset @TrapIrq9      { irq9 }
      mov    [es:$1c4], ax      mov    [es:$1c6], cs
      mov    ax, offset @TrapIrq10     { irq10 }
      mov    [es:$1c8], ax      mov    [es:$1ca], cs
      mov    ax, offset @TrapIrq11     { irq11 }
      mov    [es:$1cc], ax      mov    [es:$1ce], cs
      mov    ax, offset @TrapIrq12     { irq12 }
      mov    [es:$140], ax      mov    [es:$142], cs
    @EnableIrqs:
      mov    al, bl
      and    al, $53                   { enable IRQs 2/Cascade,3,5,7. }
      out    $21, al                   { PIC1 }
      mov    al, bh
      and    al, $e1                   { enable IRQs 9,10,11,12. }
      out    $a1, al                   { PIC2 }
      sti
      mov    [SndIrq], 0               { clear the IRQ level. }
      mov    dx, [DSPWrite]            { Tell the SB to generate a IRQ! }
    @WaitSb:
      in     al, dx
      or     al, al
      js     @WaitSb
      mov    al, $F2
      out    dx, al
      xor    cx, cx                    { wait until IRQ level }
    @WaitIRQ:
      cmp    [SndIrq], 0               { is changed or timeout. }
      jne    @IrqOk
      loop   @WaitIRQ
    @IrqOk:
      mov    al, bl                    { restore IMR. }
      out    $21, al                   { PIC1 }
      mov    al, bh
      out    $a1, al                   { PIC2 }
      cli                              { restore IRQ vectors. }
      xor    ax, ax
      mov    es, ax                    { point to vector table }
      pop    dx                        { irq12 }      pop    ax
      mov    [es:$140], ax      mov    [es:$142], dx
      pop    dx                        { irq11 }      pop    ax
      mov    [es:$1cc], ax      mov    [es:$1ce], dx
      pop    dx                        { irq10 }      pop    ax
      mov    [es:$1c8], ax      mov    [es:$1ca], dx
      pop    dx                        { irq9 }      pop    ax
      mov    [es:$1c6], ax      mov    [es:$1c4], dx
      pop    dx                        { irq7 }      pop    ax
      mov    [es:$3C], ax      mov    [es:$3E], dx
      pop    dx                        { irq5 }      pop    ax
      mov    [es:$34], ax      mov    [es:$36], dx
      pop    dx                        { irq3 }      pop    ax
      mov    [es:$2C], ax      mov    [es:$2E], dx
      pop    dx                        { irq2 }      pop    ax
      mov    [es:$28], ax      mov    [es:$2A], dx
      cli
      jmp    @Done                     { Exit test }
    @TrapIrq2:
      push   ax      mov    al, 2      jmp    @TrapIrq
    @TrapIrq3:
      push   ax      mov    al, 3      jmp    @TrapIrq
    @TrapIrq5:
      push   ax      mov    al, 5      jmp    @TrapIrq
    @TrapIrq7:
      push   ax      mov    al, 7      jmp    @TrapIrq
    @TrapIrq9:
      push   ax      mov    al, 9      jmp    @TrapIrq
    @TrapIrq10:
      push   ax      mov    al, 10      jmp    @TrapIrq
    @TrapIrq11:
      push   ax      mov    al, 11      jmp    @TrapIrq
    @TrapIrq12:
      push   ax      mov    al, 12
    @TrapIrq:
      push   dx                        { General IRQ trapper }
      push   ds                        { used for IRQ autodetect. }
      mov    dx, seg SndIRQ
      mov    ds, dx
      mov    [SndIRQ], al              { save IRQ level. }
      mov    dx, [DSPStatus]
      in     al, dx                    { SB acknowledge. }
      mov    al, $20
      out    $20, al                   { Hardware acknowledge PIC1 }
      out    $a0, al                   { Hardware acknowledge PIC2 }
      pop    ds      pop    dx      pop    ax
      iret                             { bye! }
    @Done:
    End;

  Function CheckSound : Boolean;
    Var
      Ver : Byte;
     Function CheckPort(TestPort : Word) : Boolean;
      Begin
        SndPort:= TestPort;
        if Port[SndPort] < $ff then begin
          SndWrite(4, $60);    SndWrite(4, $80);    SndWrite(2, $ff);
          If SndRead and $e0 = 0 then begin
            SndWrite(4, $21);   
            if SndRead and $e0 = $c0 then CheckPort:= true
                                     else CheckPort:= false;
            SndWrite(4, $60);     SndWrite(4, 0); DSPStatus:= SndPort + $e;
            DSPRead:= SndPort + $a;   DSPWrite:= SndPort + $c;
          end
          else CheckPort:= false;
        end;
      End;

    Begin
      SndCard:= 'None detected';      If CheckPort($388) then begin
        SndCard:= 'AdLib';  If CheckPort($220) or CheckPort($240) or 
                               CheckPort($260) or CheckPort($280) then begin
          Port[DSPWrite]:= $e1;     Delay(10);    Ver:= Port[DSPRead];
          Case Ver of
            1, 2: SndCard:= 'SoundBlaster v';
            3: SndCard:= 'SB Pro v';
            4: SndCard:= 'SB16 v';
            5: SndCard:= 'AWE32 v';
            else SndCard:= 'SB-compatible v';
          End;          Delay(10);
          SndCard:= SndCard + Stringof(Ver) + '.' + Stringof(Port[DSPRead]);
          CheckIRQ;
        end;
      end
      else SndPort:= 0;
    End;

--- Renegade v10-05 Exp
 * Origin: The BBS at Pooh Corner 707-445-0500/0599 14.4/28.8 (1:125/47)
