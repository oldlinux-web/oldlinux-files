Unit SBDSP;
{$G+}           {Enable 286 instructions}
{$N-}           {Disable Numeric Coprocessor}
{$E-}           {Shut off FPU emulation}
{$A+}           {Word Alignment}
{$S-}           {No Stack Checking}
{$R-}           {No Range Checking}

(***************************************************************************)
(*              Unit for driving an SB in DMA or DIRECT MODE               *)
(*               Note: Only for playing Digital samples!!!                 *)
(*            By: Romesh Prakashpalan (hacscb93@huey.csun.edu)             *)
(***************************************************************************)
(* Feature List (Version 1.31B):                                           *)
(*  - Support for Creative Lab's Sound Blaster DSP chip.                   *)
(*  - DMA driven, no POLLING of the Sound Card required.                   *)
(*  - Play from disk, with small buffer in memory (files can be of any size*)
(*    but a buffer of <= 64K is required in memory).                       *)
(*  - Play from memory (up to 64K).                                        *)
(*  - Software mixing of samples.                                          *)
(*  - Stereo output for the SB-PRO or higher.                              *)
(*  - Full support for the SB-PRO mixer chip.                              *)
(*  - Conversion of a VOC file to the RPD file format (via the attached    *)
(*    VOC2RPD program)                                                     *)
(*  - Conversion of a WAV file to the RPD file format (via the attached    *)
(*    WAV2RPD program)                                                     *)
(***************************************************************************)
(* So far, I've only been able to test this code on my machine, and its    *)
(* configuration follows:                                                  *)
(*  Pentium Processor @60Mhz                                               *)
(*  Tested on a Sound Blaster Pro 2.0 AND Sound Blaster AWE 32 :) (Not at  *)
(*  the same time of course ;-) ).                                         *)
(*                                                                         *)
(*  It has also been tested on a friend's machine:                         *)
(*  80386-40Mhz                                                            *)
(*  Sound Blaster 2.0 AND Sound Blaster 16 Value Edition                   *)
(*                                                                         *)
(*  80486-33Mhz                                                            *)
(*  Sound Blaster Pro 2.0, Sound Blaster 1.5, AND Pro Audio Spectrum 16    *)
(*                                                                         *)
(* Code was compiled under the following environment:                      *)
(*   DOS IDE for Turbo Pascal 7.0 (Real Mode), and also tested under the   *)
(* DOS Protected mode IDE -> TPX.EXE. Sorry, no BP 7.0 :( but the code     *)
(* should also run fine on TP 6.0.                                         *)
(*                                                                         *)
(*   Also, DMA transfers aren't supposed to work in protected mode, so if  *)
(* anyone is using Borland Pascal 7.0+, could you tell me if my code works *)
(* for a protected mode program?                                           *)
(***************************************************************************)
interface

const
  (* All types of possible DMA xfers available (includes compressed modes)*)
  EightBitDMA     = $14;      (* You'll normally need this *)
  TwoBitDMA       = $16;      (* YUKK! (In most cases) *)
  TwoBitRefDMA    = $17;
  FourBitDMA      = $74;      (* Actually, this doesn't sound too bad! *)
  FourBitRefDMA   = $75;
  TwoSixBitDMA    = $76;
  TwoSixBitRefDMA = $77;
  EightBitDMAADC  = $24;      (* Record at a resolution of 8 bits/sample *)
  HighSpeedDMAADC = $99;      (* Record at high speeds > 22Khz.          *)
  HighSpeedDMA    = $91;      (* This allows playback of samples > 22kHz *)

type
  (* This holds a "chunk" of up to 64K of the sample.                    *)
  SoundType = Record
                SampleChunk: PChar;
                Frequency: Word;     (* Frequency of the sample          *)
                Size: Word;          (* Size of the Sample               *)
                DACType: Byte;       (* ADPCM Method used for compression*)
              end;

  ProVolumeType = 0..15;             (* Volume setting Resolution on an  *)
                                     (* SB-PRO                           *)
var
  CurrentSound: SoundType;
  Playing: Boolean;                  (* If there is sound playing...     *)

(* The following Initializes the DSP chip for data, and should always be *)
(* called before you use ANY of the routines. The following are          *)
(* descriptions of what the parameters require:                          *)
(*                                                                       *)
(* Base should be:                                                       *)
(*  1 for Base Address 210h                                              *)
(*  2 for Base Address 220h (Default on most cards)                      *)
(*  3 for Base Address 230h                                              *)
(*  etc..                                                                *)
(*                                                                       *)
(* IRQ should be the IRQ level of your card (usually 5 or 7)             *)
(* Valid IRQs are now in the range of: 0..15 (or, in Hex: $0..$F)        *)
(* DMAChannel is the DMA Channel of your sound card (usually 1)          *)
(*                                                                       *)
(* Returns True if DSP was detected, else returns False                  *)

Function ResetDSP(Base : Word; IRQ, DMAChannel: Byte) : Boolean;

(* Outputs a Byte to the Sound Card by writing directly to it...         *)
Procedure WriteDAC(Level : Byte);

(* Reads a Byte from the Sound Card by reading directly to it...         *)
Function ReadDAC : Byte;

(* Turns on the speaker (use before outputting sound to the card)        *)
Function SpeakerOn: Byte;

(* Turns off the speaker (does not affect DMA transfers, e.g: DMA xfers  *)
(* will still occur, but you will not HEAR them)                         *)
Function SpeakerOff: Byte;

(* Pauses a DMA xfer                                                     *)
Procedure DMAStop;

(* Continues a paused DMA xfer                                           *)
Procedure DMAContinue;

(* Use the following to play a sound up to 64K long                      *)
Procedure PlaySound (Sound: SoundType);

(* This will play a sound file from DISK (using a 64K buffer), with the  *)
(* specified frequency and compression algorithm in SoundType            *)
Procedure PlaySoundDSK (Filename: String; Frequency: Word; SoundType: Byte);

(* This will play a sound file (RPD) from disk, so all you have to do is *)
(* specify the Filename (no other settings are required)                 *)
Procedure PlaySoundRPD (Filename: String);

(* Installs the SB Interrupt Hook                                        *)
Procedure InstallHandler;

(* Uninstalls the SB Interrupt Hook                                      *)
Procedure UninstallHandler;

(* Read a Byte from the DSP chip                                         *)
Function  ReadDSP : Byte;

(* Write a Byte to the DSP chip                                          *)
Procedure WriteDSP (Value : Byte);

(* Gets the DSP chip version number                                      *)
Function GetDSPVersion: String;

(* This mixes the two sound Buffers: Buffer1, and Buffer2, and stores    *)
(* them into the CurrentSound variable. Note: The 2 sounds have to be of *)
(* the same frequency and DACType, or else you will get weird results!   *)
Procedure MixSamples (Buffer1, Buffer2: SoundType);

(* Changes the size of the sound buffer (used for Disk xfers). A higher  *)
(* value provides smoother playback (albeit at the cost of memory), and  *)
(* a smaller one takes up less memory (but sounds choppier).             *)
(* Returns:                                                              *)
(*    True if Buffer size was changed (no sound was playing)             *)
(*    False if Buffer size wasn't changed (sound was playing)            *)
Function ChangeBufferSize (Size: Word): Boolean;

(* Returns the size of the allocated sound buffer.                       *)
Function CheckBufferSize: Word;

(*+---------------------------------------------------------------------+*)
(*|The Following routines should only be used with an SB PRO or higher: |*)
(*+---------------------------------------------------------------------+*)

(* Sets the Mixer to Mono mode                                           *)
Procedure PlayMono;

(* Sets the Mixer to Stereo mode                                         *)
(* VOC files are arranged differently in Stereo modes (odd bytes -> left *)
(* channel, even bytes -> right channel)                                 *)
Procedure PlayStereo;

(* Resets the CT 1345 Mixer chip (call before using any other functions) *)
Procedure ResetMixer;

(* Sets the Voice Level on the Sound Card *)
Procedure SetVocVolume (Left, Right: ProVolumeType);

(* Sets the FM Level on the Sound Card *)
Procedure SetFMVolume (Left, Right: ProVolumeType);

(* Sets the CD Level on the Sound Card *)
Procedure SetCDVolume (Left, Right: ProVolumeType);

(* Sets the Master Volume on the Sound Card *)
Procedure SetMasterVolume (Left, Right: ProVolumeType);

implementation
Uses Crt, DOS;

const
  (* Notes on the Buffer Size:                                            *)
  (* I generally find that values in the range of 16K to 64K are the best *)
  (* to use, (16K saves memory, and is still allows decent performance,   *)
  (* especially if the sample rate <= 11Khz, and 64K allows the BEST      *)
  (* performance possible, but takes away some precious memory space). I  *)
  (* usually prefer 32K ($7FFF) => Nice balance.                          *)

  MaxXferSize: Word = $7000;     (* Maximum size of 1 "chunk" of a sample *)

  (* Note: In previous versions of the program, the user was able to      *)
  (*       change the size of the buffer as the program goes along,       *)
  (*       without having to check if the program was playing sound at    *)
  (*       that current moment. Now, this variable is isolated, and to    *)
  (*       change/check on the size of the currently allocated buffer, one*)
  (*       has to make the following calls: ChangeBufferSize, and         *)
  (*       CheckBufferSize. This way, the user doesn't "kill" the system  *)
  (*       in some weird way!                                             *)

type
 (* Voice type incorported in my programs, (I use the VOC2RPD program to     *)
 (* convert VOC files, and the WAV2RPD program to convert WAV files)         *)
 (* Channel method: 0 - Layed down Byte 1 - Channel 0, Byte 2 - Channel 1... *)
 (*                 1 - First Channel continuously for Size Bytes, then comes*)
 (*                     the Second Channel, etc...                           *)
  RPDHeader = Record
                Sig: Array [0..2] of Char; (* "RPD" *)
                Version: Word;             (* Version # *)
                DAC: Byte;                 (* 8/16/4/4.6/2/2.6, etc...*)
                Phase: Byte;               (* Mono=0, Stereo=1, Surround=2 *)
                Freq: Word;                (* Sample Frequency *)
                Channels: Byte;            (* # of DIGITAL Channels *)
                ChannelMethod: Byte;       (* Method for laying down channels *)
                Size: LongInt;             (* Size of Sample *)
                Reserved: Array [0..31] of Byte;
              end;
var
  DSP_RESET : Word;
  DSP_READ_DATA : Word;
  DSP_WRITE_DATA : Word;
  DSP_WRITE_STATUS : Word;
  DSP_DATA_AVAIL : Word;
  DSPPRO_READWRITE: Word;
  DSPPRO_INDEX: Word;
  DMAStartMask : Byte;
  DMAStopMask  : Byte;
  DMAModeReg   : Byte;
  IntHandler: Byte;
  OldIntHandler: Procedure;
  PicPort: Byte;
  IRQStopMask, IRQStartMask: Byte;
  CurPos: LongInt;
  CurPageEnd: LongInt;
  Length: Word;
  LeftToPlay: LongInt;
  VoiceEnd: LongInt;
  DSKFile: File;
  FileXferLeft: LongInt;
  PlayFromDisk: Boolean;
  OldExitProc: Pointer;

const
  IRQHandlerInstalled: Boolean = False;

Procedure PlayBack; forward;

Function GetAbsoluteAddress (P: Pointer): LongInt;
(* Faster when implemented in assembly (and easier!)    *)
(* Uses the only 32-bit code in this unit               *)
Begin
  asm
    DB 66h; XOR AX, AX                 {; Make sure that no "junk" is in the}
    DB 66h; XOR BX, BX                 {; extended Registers}
    MOV AX, WORD PTR [P+2]             {; AX := SEG (P^) }
    MOV BX, WORD PTR [P]               {; BX := OFS (P^) }
    DB 66h; SHL AX, 4                  {; EAX := SEG (P^) * 16}
    DB 66h; ADD AX, BX                 {; EAX := EAX + OFS (P^)}
    DB 66h; MOV WORD PTR @RESULT, AX   {; GetAbsoluteAddress := EAX }
  end;
End;

Procedure MixSamples (Buffer1, Buffer2: SoundType);
(* Mixes the two buffers Buffer1 and Buffer2, and mixes them to CurrentSound *)
(* Note: Sounds must have the same frequency, but DON'T have to have the same*)
(*       length!                                                             *)
var
  Temp1, Temp2, TempC: PChar;
  CurrentByte: Word;
Begin
  (* Figure out the size of the resulting sample (Max {Buffer1, Buffer2}) *)
  If Buffer1.Size > Buffer2.Size then
    CurrentSound.Size := Buffer1.Size
  else CurrentSound.Size := Buffer2.Size;
  (* Assuming that both frequencies are the same *)
  CurrentSound.Frequency := Buffer1.Frequency;

  (* Assuming that both DAC types are the same *)
  CurrentSound.DACType := Buffer1.DACType;
  CurrentByte := 0;
  Temp1 := Buffer1.SampleChunk;
  Temp2 := Buffer2.SampleChunk;
  TempC := CurrentSound.SampleChunk;
  Repeat
    (* Simple scaling function to "mix" the samples *)
    TempC^ := Char (Round ((((Byte (Temp2^) * 2) - Byte (Temp1^))*2)/2));
    Inc (TempC);
    If (CurrentByte < Buffer1.Size) then
      Inc (Temp1)
    Else
      Temp1^ := #0;
    If (CurrentByte < Buffer2.Size) then
      Inc (Temp2)
    Else
      Temp2^ := #0;
    Inc (CurrentByte);
  Until (CurrentByte >= CurrentSound.Size);
End;

Function GetDSPVersion: String;
var
  MajorByte, MinorByte: Byte;
  MajorStr, MinorStr: String;
Begin
  WriteDSP ($E1);
  MajorByte := ReadDSP;
  Str (MajorByte, MajorStr);
  MinorByte := ReadDSP;
  Str(MinorByte, MinorStr);
  If MinorByte < 10 then MinorStr := '0' + MinorStr;
  GetDSPVersion := MajorStr + '.' + MinorStr;
End;

Procedure WriteDSP(Value : Byte);
Begin
  While Port[DSP_WRITE_STATUS] and $80 <> 0 do;
  Port[DSP_WRITE_DATA] := Value;
End;

Function ReadDSP : Byte;
Begin
  While Port[DSP_DATA_AVAIL] and $80 = 0 do;
  ReadDSP := Port[DSP_READ_DATA];
End;

Procedure WriteDAC(Level : Byte);
Begin
  WriteDSP($10);
  WriteDSP(Level);
End;

Function ReadDAC : Byte;
Begin
  WriteDSP($20);
  ReadDAC := ReadDSP;
End;

Function SpeakerOn: Byte;
Begin
  WriteDSP($D1);
End;

Function SpeakerOff: Byte;
Begin
  WriteDSP($D3);
End;

Procedure DMAContinue;
Begin
  WriteDSP($D4);
End;

Procedure DMAStop;
Begin
  WriteDSP($D0);
End;

Procedure PlaySoundDSK (Filename: String; Frequency: Word; SoundType: Byte);
Begin
  Assign (DSKFile, Filename);
  Reset (DSKFile, 1);
  GetMem (CurrentSound.SampleChunk, MaxXferSize);
  CurrentSound.Frequency := Frequency;
  CurrentSound.DACType := SoundType;
  FileXferLeft := FileSize (DSKFile);
  PlayFromDisk := True;
  If FileSize (DSKFile) > MaxXferSize then
  Begin
    CurrentSound.Size := MaxXferSize;
    BlockRead (DSKFile, CurrentSound.SampleChunk^, MaxXferSize);
    Dec (FileXferLeft, MaxXferSize);
  End
  Else
  Begin
    CurrentSound.Size := FileSize (DSKFile);
    BlockRead (DSKFile, CurrentSound.SampleChunk^, Filesize (DSKFile));
    Dec (FileXferLeft, FileSize (DSKFile));
  End;
  LeftToPlay := CurrentSound.Size - 6;
  CurPos := GetAbsoluteAddress (CurrentSound.SampleChunk) + 6;
  CurPageEnd := ((CurPos shr 16) shl 16) + 65536 - 1;
  Length := CurPageEnd - CurPos;
  VoiceEnd := CurPos + LeftToPlay;
  Playing := True;
  PlayFromDisk := True;
  SpeakerOn;
  PlayBack;
End;

Procedure PlaySoundRPD (Filename: String);
var
  TempHead: RPDHeader;
Begin
  Assign (DSKFile, Filename);
  Reset (DSKFile, 1);
  BlockRead (DSKFile, TempHead, Sizeof (TempHead));
  GetMem (CurrentSound.SampleChunk, MaxXferSize);
  CurrentSound.Frequency := TempHead.Freq;
  CurrentSound.DACType := TempHead.DAC;
  If CurrentSound.Frequency >= 23000 then CurrentSound.DACType := HighSpeedDMA;
  ResetMixer;
  (* Then we have a stereo file, and for the SB-PRO, output in stereo... *)
  If TempHead.Phase = 1 then
    PlayStereo   (* Initialize the SB-PRO to play in stereo  *)
  Else PlayMono; (* We have a mono file, and reset the mixer *)
  FileXferLeft := FileSize (DSKFile) - SizeOf (TempHead);
  PlayFromDisk := True;
  If FileSize (DSKFile) > MaxXferSize then
  Begin
    CurrentSound.Size := MaxXferSize;
    BlockRead (DSKFile, CurrentSound.SampleChunk^, MaxXferSize);
    Dec (FileXferLeft, MaxXferSize);
  End
  Else
  Begin
    CurrentSound.Size := FileSize (DSKFile);
    BlockRead (DSKFile, CurrentSound.SampleChunk^, FileXferLeft);
    Dec (FileXferLeft, FileSize (DSKFile));
  End;
  LeftToPlay := CurrentSound.Size - 6;
  CurPos := GetAbsoluteAddress (CurrentSound.SampleChunk) + 6;
  CurPageEnd := ((CurPos shr 16) shl 16) + 65536 - 1;
  Length := CurPageEnd - CurPos;
  VoiceEnd := CurPos + LeftToPlay;
  Playing := True;
  PlayFromDisk := True;
  SpeakerOn;
  PlayBack;
End;

Procedure PlaySound (Sound: SoundType);
Begin
  LeftToPlay := Sound.Size - 6;
  CurPos := GetAbsoluteAddress (Sound.SampleChunk) + 6;
  CurPageEnd := ((CurPos shr 16) shl 16) + 65536 - 1;
  Length := CurPageEnd - CurPos;
  VoiceEnd := CurPos + LeftToPlay;
  Playing := True;
  PlayFromDisk := False;
  SpeakerOn;
  PlayBack;
End;

Function ChangeBufferSize (Size: Word): Boolean;
Begin
  ChangeBufferSize := False;
  If not Playing then
  Begin
    MaxXferSize := Size;
    ChangeBufferSize := True;
  End;
End;

Function CheckBufferSize: Word;
Begin
  CheckBufferSize := MaxXferSize;
End;

Procedure ClearInterrupt;
var
  Temp: Byte;
Begin
  Temp := Port[DSP_DATA_AVAIL];
  Port[$20] := $20;
End;

Procedure Playback;
(* This procedure should NOT be called by anyone outside of this unit, as  *)
(* its prime purpose is to start a DMA xfer, or get called by the Interrupt*)
(* handler when a xfer has to be continued!                                *)
var
  Time_Constant : Word;
  Page, Offset : Word;
begin

  { Set up the DMA chip, by setting the Page and Offsets }
  Page := CurPos shr 16;
  Offset := CurPos mod 65536;
  if VoiceEnd < CurPageEnd
  then Length := LeftToPlay-1
  else Length := CurPageEnd - CurPos;

  Inc(CurPos, LongInt(Length)+1);
  Dec(LeftToPlay, LongInt(Length)+1);
  Inc(CurPageEnd, 65536);

  { Set the playback frequency }
  Time_Constant := 256 - (1000000 div CurrentSound.Frequency);
  WriteDSP($40);
  WriteDSP(Time_constant);

  {Now, we must program the DMA chip for another xfer}
  Port[$0A] := DMAStopMask;     (* Stop any DMA activity so far *)
  Port[$0C] := 0;
  Port[$0B] := DMAModeReg;
  Port[$02] := Lo(Offset);
  Port[$02] := Hi(Offset);
  Port[$03] := Lo(Length);
  Port[$03] := Hi(Length);
  Port[$83] := Page;
  Port[$0A] := DMAStartMask;    (* Start the DMA transfer *)

  If CurrentSound.DACType = HighSpeedDMA then
  Begin
    WriteDSP ($48);
    WriteDSP (Lo (Length));
    WriteDSP (Hi (Length));
    WriteDSP ($91);
  End
  Else
  Begin
    WriteDSP(CurrentSound.DACType);
    WriteDSP(Lo(Length));
    WriteDSP(Hi(Length));
  End;
end;

Procedure SBIntHandler; interrupt;
(* This procedure handles interrupt calls from the DMA chip when a xfer is *)
(* complete. As of version 1.01, this procedure has been expanded to read  *)
(* in a block off of a disk file as well. I have to try to make this code  *)
(* as fast as possible in the future, as it will "Interrupt" the main      *)
(* program, to do its own thing. And, in a game that can slow things down  *)
(* considerably. (Depending on sample rates, after all, sound that is      *)
(* recorded at 44Khz at 8 bits will xfer 44K to the DMA chip/second, that  *)
(* means with a 32K buffer, we won't even get 1 SECOND before the next     *)
(* interrupt!!!                                                            *)
Begin
  asm CLI end;
  If LeftToPlay > 0 then
    PlayBack
  Else if PlayFromDisk then
  Begin
    If FileXferLeft > 0 then
    Begin
      If FileXferLeft > MaxXferSize then
      Begin
        CurrentSound.Size := MaxXferSize;
        BlockRead (DSKFile, CurrentSound.SampleChunk^, MaxXferSize);
        Dec (FileXferLeft, MaxXferSize);
      End
      Else
      Begin
        CurrentSound.Size := FileXferLeft;
        BlockRead (DSKFile, CurrentSound.SampleChunk^, FileXferLeft);
        FileXferLeft := 0;
      End;
      LeftToPlay := CurrentSound.Size - 6;
      CurPos := GetAbsoluteAddress (CurrentSound.SampleChunk) + 6;
      CurPageEnd := ((CurPos shr 16) shl 16) + 65536 - 1;
      Length := CurPageEnd - CurPos;
      VoiceEnd := CurPos + LeftToPlay;
      PlayBack;
    End
    Else
    Begin
      (* Say that we have nothing more to say :) *)
      Playing := False;
      PlayFromDisk := False;
      (* Turn off the speaker *)
      SpeakerOff;
      (* Close the file that we used, and free our memory space *)
      Close (DSKFile);
      FreeMem (CurrentSound.SampleChunk, MaxXferSize);
    End;
  End
  Else
    Playing := False;
  asm STI end;
  ClearInterrupt;
End;

Procedure StopSBIRQ;
Begin
  Port[PICPort] := Port[PICPort] OR IRQStopMask;
End;

Procedure StartSBIRQ;
Begin
  Port[PICPort] := Port[PICPort] AND IRQStartMask;
End;

Procedure SetMixerReg(Index, Value : Byte);
Begin
  Port[DSPPRO_INDEX] := Index;
  Port[DSPPRO_READWRITE] := Value;
End;

Function GetMixerReg(Index : Byte) : Byte;
Begin
  Port[DSPPRO_INDEX] := Index;
  GetMixerReg := Port[DSPPRO_READWRITE];
End;

Procedure ResetMixer;
Begin
  SetMixerReg (0, 0);
End;

Procedure PlayStereo;
Begin
  SetMixerReg ($E, $2);
End;

Procedure PlayMono;
Begin
  SetMixerReg ($E, $0);
End;

Procedure SetFMVolume (Left, Right: ProVolumeType);
Begin
  SetMixerReg ($26, Left SHL 4 + Right);
End;

Procedure SetCDVolume (Left, Right: ProVolumeType);
Begin
  SetMixerReg ($28, Left SHL 4 + Right);
End;

Procedure SetMasterVolume (Left, Right: ProVolumeType);
Begin
  SetMixerReg ($22, Left SHL 4 + Right);
End;

Procedure SetVocVolume (Left, Right: ProVolumeType);
Begin
  SetMixerReg ($4, Left SHL 4 + Right);
End;

Procedure InstallHandler;
Begin
  StopSBIRQ;
  GetIntVec(IntHandler, @OldIntHandler);
  SetIntVec(IntHandler, @SBIntHandler);
  StartSBIRQ;
  IRQHandlerInstalled := True;
End;

Procedure UninstallHandler;
Begin
  StopSBIRQ;
  SetIntVec(IntHandler, @OldIntHandler);
  IRQHandlerInstalled := False;
End;

Function ResetDSP(Base : Word; IRQ, DMAChannel: Byte) : Boolean;
   (* Returns TRUE if a DSP chip was found, else returns FALSE *)
const
   (* The IRQ Interrupt numbers available *)
   IRQIntNums : Array[0..15] of Byte =
                ($08, $09, $0A, $0B, $0C, $0D, $0E, $0F,
                 $70, $71, $72, $73, $74, $75, $76, $77);

Begin
  {Figure out the DMA channel info...}
  DMAStartMask := DMAChannel + $00;
  DMAStopMask  := DMAChannel + $04;
  DMAModeReg   := DMAChannel + $48;

  If IRQ <= 7 then PICPort := $21 else PICPort := $A1;

  IRQStopMask  := 1 SHL (IRQ MOD 8);
  IRQStartMask := Not (IRQStopMask);

  Base := Base * $10;

  { Calculate the port addresses }
  DSP_RESET        := Base + $206;
  DSP_READ_DATA    := Base + $20A;
  DSP_WRITE_DATA   := Base + $20C;
  DSP_WRITE_STATUS := Base + $20C;
  DSP_DATA_AVAIL   := Base + $20E;
  DSPPRO_READWRITE := Base + $205;
  DSPPRO_INDEX     := Base + $204;

  { Reset the DSP, and give some nice long delays just to be safe }
  Port[DSP_RESET] := 1;
  Delay(10);
  Port[DSP_RESET] := 0;
  Delay(10);
  if (Port[DSP_DATA_AVAIL] and $80 = $80) and (Port[DSP_READ_DATA] = $AA) then
    ResetDSP := True
  else
    ResetDSP := False;
  IntHandler := IRQIntNums[IRQ];
  InstallHandler;
end;

{$F+}
Procedure SBExitProc;
Begin
  ExitProc := OldExitProc;
  If IRQHandlerInstalled then
    UninstallHandler;
End;
{$F-}

Begin
  OldExitProc := ExitProc;
  ExitProc := @SBExitProc;
  Playing := False;
End.

(***************************************************************************)


Program ConvertVOCRPD;

(**************************************************************************)
(*                            Program VOC2RPD                             *)
(*                       By: Romesh Prakashpalan                          *)
(**************************************************************************)
(*  This program will convert a VOC file to my RPD format (the specs have *)
(* been given out with my SBDSP unit). This will eliminate any "noise"    *)
(* you might get due to the fact that you are probably reading in VOC     *)
(* data structures instead of pure RAW data.                              *)
(*  Any questions or comments (or donations ;-) ) should be directed to:  *)
(*    Romesh Prakashpalan (hacscb93@huey.csun.edu) <- Until 01/31/94.     *)
(*     Note: After 01/31/94, my account at CSUN expires, so look in the   *)
(*           comp.lang.pascal or alt.sb.programmer newsgroups, for my     *)
(*           current address.                                             *)
(**************************************************************************)

Uses SBDSP, Crt;

(**************************************************************************)
(*  Revisions:                                                            *)
(*    Version 1.0à: Just strips off the VOC file header, to reduce a bit  *)
(*                  of the "popping" that occurs when played back as a RAW*)
(*                  file. This was pretty lame!                           *)
(*    Version 1.10: Converts the file pretty much with all VOC specific   *)
(*                  information removed. However, repeat loops will only  *)
(*                  iterate ONCE, and Silence blocks will be REMOVED from *)
(*                  the VOC file. In future versions, I plan on adding    *)
(*                  repeats manually, digital silence (if user specifies  *)
(*                  it), but these are minor things.                      *)
(*    Version 1.11: STOOPID MISTAKE FIXED!!!! Would not write header to   *)
(*                  the beginning of the file, but rather to the END!!    *)
(*    Version 1.12: Added Input/Output checking on files, instead of      *)
(*                  the program giving a "run-time error", it gives a     *)
(*                  suitable error message.                               *)
(*    Version 1.13: Fixed a Block 9 error in conversion (Previously I     *)
(*                  did not have any VOC files to test Block 9 conversion *)
(*                  on, so I apologize!)                                  *)
(**************************************************************************)
(* Future Revisions:                                                      *)
(*  I hope to create support for 16-bit sound in the next versions of my  *)
(* code, but this is dependant on when I will be able to get code to      *)
(* program the Sound Blaster 16! Only then will I be able to implement    *)
(* such a conversion, as my program is the only one which will read, or   *)
(* even recognise an RPD file! (As far as I know)                         *)
(**************************************************************************)
(*  Note: To all of those who are using this file for the SBDSP unit that *)
(*        I have given out: What do you think of my unit???, do you think *)
(*        that I should create support for the Sound Blaster 16? I do not *)
(*        think that I should charge anything for it, as that would make  *)
(*        me as bad as Creative Labs, but I would willingly take a small  *)
(*        donation ;-). That's all right, just as long as you are able to *)
(*        use the code, I'm happy. Please let me know what you think about*)
(*        the unit though!                                                *)
(**************************************************************************)

type

(****************************************************************************)
(* Channel method: 0 - Layed down Byte 1 - Channel 0, Byte 2 - Channel 1... *)
(*                 1 - First Channel continuously for Size Bytes, then comes*)
(*                     the Second Channel, etc...                           *)
(****************************************************************************)
  RPDHeader = Record
                Sig: Array [0..2] of Char; (* "RPD" *)
                Version: Word;             (* Version # *)
                DAC: Byte;                 (* 8/16/4/4.6/2/2.6, etc...*)
                Phase: Byte;               (* Mono=0, Stereo=1, Surround=2*)
                Freq: Word;                (* Sample Frequency *)
                Channels: Byte;            (* # of DIGITAL Channels *)
                ChannelMethod: Byte;       (* Method for laying down channels *)
                Size: LongInt;             (* Size of Sample *)
                Reserved: Array [0..31] of Byte;
              end;

  VOCHeader = Record
                Sig:Array [0..$13] of Char;(* "Creative Voice File" *)
                Offset: Word;              (* Offset of first datablock in the*)
                                           (* .VOC file.                      *)
                Version: Word;             (* Version # *)
                Version2s: Word;           (* 2's complement of version # plus*)
                                           (* 1234h ex: 1.10 = 1129           *)
              end;
  ThreeByte = Array [1..3] of Byte;

  Block1Type = Record                      (* Voice Data Block *)
                Length: ThreeByte;
                TimeConstant: Byte;        (* = 256 - 1000000/Sample Rate *)
                PackType: Byte;            (* 0 = 8-bit unpacked *)
                                           (* 1 = 4-bit packed   *)
                                           (* 2 = 2.6 bit packed *)
                                           (* 3 = 2-bit packed   *)
               end;
  Block2Type = Record                      (* Voice Continuation *)
                 Length: ThreeByte;
               end;

  Block3Type = Record                      (* Silence block *)
                 Length: ThreeByte;        (* Always 3 *)
                 Pause: Word;              (* Pause period in sample bytes *)
                 TimeConstant: Byte;
               end;

  Block4Type = Record                      (* Marker Block *)
                 Length: ThreeByte;
                 MarkerValue: Word;
               end;

  Block5Type = Record                      (* Ascii Text (null-terminated) *)
                 Length: ThreeByte;
               end;

  Block6Type = Record                      (* Repeat Loop *)
                 Length: ThreeByte;        (* Always 2 *)
                 Count: Word;              (* 1 to $FFFE, $FFFF = endless loop*)
               end;

  Block7Type = Record                      (* End Repeat Loop *)
                 Length: ThreeByte;        (* Always 0 *)
               end;

  Block8Type = Record                      (* Extended Block *)
                 Length: ThreeByte;        (* Always 4 *)
                 TimeConstant: Word;       (* For Mono:  *)
                                           (* 65535-(256,000,000/sample rate)*)
                                           (* For Stereo: *)
                                           (* 65535-(256000000/sample rate*2) *)
                 PackType: Byte;           (* Same as Block 1 *)
                 Mode: Byte;               (* 0 = Mono, 1 = Stereo *)
               end;

  Block9Type = Record                      (* NEW Extended VOC Block *)
                 Length: ThreeByte;        (* Length of Sample + 12 (Bytes) *)
                 SamplesPerSec: LongInt;   (* ACTUAL Samples/Second *)
                 BitsPerSample: Byte;      (* Bits/Sample after compression *)
                 Channels: Byte;           (* 1 for mono, 2 for stereo *)
                 FormatTag: Word;          (* Format Tags follow: *)
                                           (*   $000 - 8 bit unsigned PCM *)
                                           (*   $001 - 4 Bit ADPCM *)
                                           (*   $002 - 2.6 Bit ADPCM *)
                                           (*   $003 - 2 Bit ADPCM *)
                                           (*   $004 - 16 Bit Signed PCM *)
                                           (*   $006 - CCITT a-Law *)
                                           (*   $007 - CCITT u-Law *)
                                           (*   $200 - 16 bit -> 4 Bit ADPCM *)
                 Reserved: LongInt;        (* Reserved by Creative Labs *)
               end;
var
  Source, Destination: String;
  Ch: Char;

Function ThreeByte2Long (TheData: ThreeByte): LongInt;
Begin
  ThreeByte2Long := LongInt (TheData[1]) + LongInt (TheData[2]) shl 8 +
                    LongInt (TheData[3]) shl 16;
End;

Procedure ConvertVOC2RPD (Source, Destination: String);
var
  SourceF: File;
  DestF: File;
  TempRPDHead: RPDHeader;
  TempVOCHead: VOCHeader;
  TempBuffer: Pointer;
  BlockType: Byte;
  BytesConverted: LongInt;
  Done: Boolean;
  Block1: Block1Type;
  Block2: Block2Type;
  Block3: Block3Type;
  Block4: Block4Type;
  Block5: Block5Type;
  Block6: Block6Type;
  Block7: Block7Type;
  Block8: Block8Type;
  Block9: Block9Type;
  LeftToGo: LongInt;
  SkipNextBlock1: Boolean;

Begin
  SkipNextBlock1 := False;
  BytesConverted := 0;
  Assign (SourceF, Source);
  Assign (DestF, Destination);
  Reset (SourceF, 1);
  Rewrite (DestF, 1);
  BlockRead (SourceF, TempVOCHead, SizeOf (TempVOCHead));
  GetMem (TempBuffer, $FFFF);
  TempRPDHead.Sig := 'RPD';
  TempRPDHead.Version := 1;
  TempRPDHead.Channels := 1;
  TempRPDHead.ChannelMethod := 0;
  (* Blank out the reserved field with 0's, so that future software won't *)
  (* be confused!                                                         *)
  FillChar (TempRPDHead.Reserved, SizeOf (TempRPDHead.Reserved), 0);
  (* Write the incomplete header to the file, we shall update it later... *)
  BlockWrite (DestF, TempRPDHead, SizeOf (TempRPDHead));
  Done := False;
  Repeat
    BlockRead (SourceF, BlockType, SizeOf (BlockType));
    Case BlockType of
       0: Begin         (* Terminator Block *)
            WriteLn ('Block Type 0 encountered, conversion complete...');
            Done := True;
          End;
       1: Begin         (* Data Block *)
            Write ('Converting Block Type 1.');
            BlockRead (SourceF, Block1, SizeOf (Block1));
            If not SkipNextBlock1 then
            Begin
              Case Block1.PackType of
                0: TempRPDHead.DAC := EightBitDMA;
                1: TempRPDHead.DAC := FourBitDMA;
                2: TempRPDHead.DAC := TwoSixBitDMA;
                3: TempRPDHead.DAC := TwoBitDMA;
              end;
              TempRPDHead.Freq := Round (1000000/(256 - Block1.TimeConstant));
              TempRPDHead.Phase := 0;   (* Block Type 1 is ALWAYS MONO *)
            End
            Else SkipNextBlock1 := False;
            Inc (BytesConverted, ThreeByte2Long (Block1.Length)-2);
            LeftToGo := ThreeByte2Long (Block1.Length) - 2;
            Repeat
              If LeftToGo > $FFFF then
              Begin
                Dec (LeftToGo, $FFFF);
                BlockRead (SourceF, TempBuffer^, $FFFF);
                BlockWrite (DestF, TempBuffer^, $FFFF);
              End
              Else
              Begin
                BlockRead (SourceF, TempBuffer^, LeftToGo);
                BlockWrite (DestF, TempBuffer^, LeftToGo);
                LeftToGo := 0;
              End;
              Write ('.');
            Until LeftToGo = 0;
            WriteLn;
            WriteLn ('Block Type 1: ', ThreeByte2Long (Block1.Length),
                     ' bytes converted.');
          end;
          2: Begin      (* Voice Continuation *)
               WriteLn ('Converting Block 2, Voice Continuation.');
               BlockRead (SourceF, Block2, SizeOf (Block2));
               Inc (BytesConverted, ThreeByte2Long (Block2.Length));
               LeftToGo := ThreeByte2Long (Block2.Length);
               Repeat
                If LeftToGo > $FFFF then
                Begin
                  Dec (LeftToGo, $FFFF);
                  BlockRead (SourceF, TempBuffer^, $FFFF);
                  BlockWrite (DestF, TempBuffer^, $FFFF);
                End
                Else
                Begin
                  BlockRead (SourceF, TempBuffer^, LeftToGo);
                  BlockWrite (DestF, TempBuffer^, LeftToGo);
                  LeftToGo := 0;
                End;
                Write ('.');
              Until LeftToGo = 0;
              WriteLn;
              WriteLn ('Block Type 2: ', ThreeByte2Long (Block1.Length),
                       ' bytes converted.');
             end;
          3: Begin      (* Silence Block, will be skipped *)
               WriteLn ('Skipping a Silence Block (Block Type 3)');
               BlockRead (SourceF, Block3, SizeOf (Block3));
             end;
          4: Begin      (* Marker Block *)
               WriteLn ('Skipping a Marker Block (Block Type 4)');
               BlockRead (SourceF, Block4, SizeOf (Block4));
             End;
          5: Begin      (* ASCII text *)
               WriteLn ('Skipping embedded ASCII text (Block Type 5)');
               BlockRead (SourceF, Block5, SizeOf (Block5));
             End;
          6: Begin      (* Repeat Loop, in version 1.10, they are skipped *)
               WriteLn ('Skipping the repeat loop, loop will play ONCE');
               BlockRead (SourceF, Block6, SizeOf (Block6));
             End;
          7: Begin      (* Signal the end of a repeat loop *)
               WriteLn ('End of Repeat Loop found...');
               BlockRead (SourceF, Block7, SizeOf (Block7));
             End;
          8: Begin      (* Extended Block (Stereo, and other 8-bit goodies) *)
               WriteLn ('An extended block was found (8), reading data...');
               SkipNextBlock1 := True;
               BlockRead (SourceF, Block8, SizeOf (Block8));
               Case Block8.PackType of
                 0: TempRPDHead.DAC := EightBitDMA;
                 1: TempRPDHead.DAC := FourBitDMA;
                 2: TempRPDHead.DAC := TwoSixBitDMA;
                 3: TempRPDHead.DAC := TwoBitDMA;
               end;
               If Block8.Mode = 0 then
               Begin
                 TempRPDHead.Phase := 0;
                 TempRPDHead.Freq := (-256000000 div (Block8.TimeConstant
                                      - 65536));
               End
               Else
               Begin
                 TempRPDHead.Phase := 1;
                 TempRPDHead.Freq := (-256000000 div (Block8.TimeConstant
                                      - 65536));
               End;
             End;
          9: Begin     (* NEW Extended Block (16-Bit/other NEW stuff) *)
               Write ('Converting Block 9.');
               BlockRead (SourceF, Block9, SizeOf (Block9));
               TempRPDHead.Freq := Block9.SamplesPerSec;
               TempRPDHead.DAC := 0;
               Inc (BytesConverted, ThreeByte2Long (Block9.Length));
               Case Block9.FormatTag of
                    $000: TempRPDHead.DAC := EightBitDMA;
                    $001: TempRPDHead.DAC := FourBitDMA;
                    $002: TempRPDHead.DAC := TwoSixBitDMA;
                    $003: TempRPDHead.DAC := TwoBitDMA
                    else
                      WriteLn ('Extended type (16 bit), not supported!');
                    (* All other types are undefined in my program as of now! *)
               end;
               TempRPDHead.Phase := (Block9.Channels - 1);
               If TempRPDHead.Phase = 1 then
                 TempRPDHead.Freq := TempRPDHead.Freq * 2;
               LeftToGo := ThreeByte2Long (Block9.Length) - 12;
               Repeat
                If LeftToGo > $FFFF then
                Begin
                  Dec (LeftToGo, $FFFF);
                  BlockRead (SourceF, TempBuffer^, $FFFF);
                  BlockWrite (DestF, TempBuffer^, $FFFF);
                End
                Else
                Begin
                  BlockRead (SourceF, TempBuffer^, LeftToGo);
                  BlockWrite (DestF, TempBuffer^, LeftToGo);
                  LeftToGo := 0;
                End;
                Write ('.');
              Until LeftToGo = 0;
              WriteLn;
              WriteLn ('Block Type 9: ', ThreeByte2Long (Block9.Length) - 12,
                       ' bytes converted.');
             End;
    end;
  Until Done;
  TempRPDHead.Size := BytesConverted;
  FreeMem (TempBuffer, $FFFF);
  Seek (DestF, 0);
  (* Now, write the completed information... *);
  BlockWrite (DestF, TempRPDHead, SizeOf (TempRPDHead));
  Close (SourceF);
  Close (DestF);
  WriteLn ('File conversion done...');
  WriteLn ('***************************************************');
  Write ('Mode: ');
  Case TempRPDHead.Phase of
    0: WriteLn ('Mono');
    1: WriteLn ('Stereo');
  end;
  Case TempRPDHead.Dac of
       EightBitDMA: WriteLn ('8 Bit (unpacked)');
       FourBitDMA: WriteLn ('4 Bit (packed)');
       FourBitRefDMA: WriteLn ('4 Bit w/Reference Byte (packed)');
       TwoSixBitDMA: WriteLn ('2.6 Bit (packed)');
       TwoSixBitRefDMA: WriteLn ('2.6 Bit w/Reference Byte (packed)');
       TwoBitDMA: WriteLn ('2 Bit (packed)');
       TwoBitRefDMA: WriteLn ('2 Bit w/Reference Byte (packed)');
  end;
  If TempRPDHead.Phase = 1 then TempRPDHead.Freq := TempRPDHead.Freq div 2;
  WriteLn ('Frequency:  ', TempRPDHead.Freq, ' Hz');
  WriteLn ('***************************************************');
End;

Function FileExists (Filename: String): Boolean;
var
 F: file;
Begin
 {$I-}
 Assign(F, FileName);
 FileMode := 0;      (* Set file access to read only *)
 Reset(F);
 Close(F);
 {$I+}
 FileExists := (IOResult = 0) and (FileName <> '');
End;

Begin
  Clrscr;
  WriteLn ('           VOC2RPD version 1.13, By: Romesh Prakashpalan, 1994');
  WriteLn ('                            VOC2RPD is FREEWARE             ');
  Write ('Enter in VOC file to be converted: ');
  ReadLn (Source);
  If not FileExists (Source) then
  Begin
    WriteLn ('Source File Doesn''t Exist!');
    Halt;
  End;
  Write ('Enter in RPD file to convert to: ');
  ReadLn (Destination);
  If FileExists (Destination) then
  Begin
    Write ('File exists! overwrite? (''N'' for No, any other key kills it): ');
    Ch := UpCase (Readkey);
    WriteLn (Ch);
    If Ch = 'N' then Halt;
  End;
  ConvertVOC2RPD (Source, Destination);
End.

(***************************************************************************)

Program WAV2RPD;

(**************************************************************************)
(*                            Program WAV2RPD                             *)
(*                       By: Romesh Prakashpalan                          *)
(**************************************************************************)
(*  This program is similar to my VOC2RPD program in that it converts a   *)
(* WAV file to the RPD format. A WAV file is considerably easier to       *)
(* convert than a VOC file, as there aren't that many Block Types!        *)
(**************************************************************************)

Uses SBDSP, Crt;

(**************************************************************************)
(* Revisions:                                                             *)
(*   *Version 1.0a: Current Version                                       *)
(**************************************************************************)
type
  ChunkType = Record
                ID: Array [1..4] of Char;      (* "RIFF" *)
                Len: LongInt;                  (* Size of the Data Chunk *)
              end;


  DataType = Record    (* For a Wave Type File *)
                ID: Array [1..4] of Char;      (* "WAVE" *)
              end;
  FormatChunkType = Record
                 ChunkID: Array [1..4] of Char; (* "fmt" *)
                 Len: LongInt;         (* Size of Data *)
                 FormatTag: Word;      (* 01 = PCM     *)
                 Channels: Word;       (* 1 = Mono, 2 = Stereo *)
                 SamplesPerSec: Word;  (* PlayBack Freq *)
                 AvgBytesPerSec: Word;
                 BlockAlign: Word;
                 FormatSpecific: Word;
               end;

  RPDHeader = Record
                Sig: Array [0..2] of Char; (* "RPD" *)
                Version: Word;             (* Version # *)
                DAC: Byte;                 (* 16/8/4/4.6/2/2.6, etc...*)
                Phase: Byte;               (* Mono=0, Stereo=1, Surround=2 *)
                Freq: Word;                (* Sample Frequency *)
                Channels: Byte;            (* # of DIGITAL Channels *)
                ChannelMethod: Byte;       (* Method for laying down channels *)
                Size: LongInt;             (* Size of Sample *)
                Reserved: Array [0..31] of Byte;
              end;

var
  Source, Destination: String;
  Ch: Char;

Procedure ConvertWAV2RPD (Source, Destination: String);
var
  SourceF, DestF: File;
  TempRPDHead: RPDHeader;
  TempChunk: ChunkType;
  TempData: DataType;
  TempFormatChunk: FormatChunkType;
  TempBuffer: Pointer;
  NumRead, NumWritten: Word;
Begin
  GetMem (TempBuffer, $FFFF);
  Assign (SourceF, Source);
  Assign (DestF, Destination);
  Reset (SourceF, 1);
  Rewrite (DestF, 1);
  BlockRead (SourceF, TempChunk, SizeOf (TempChunk));
  BlockRead (SourceF, TempData, SizeOf (TempData));
  BlockRead (SourceF, TempFormatChunk, SizeOf (TempFormatChunk));
  TempRPDHead.Sig := 'RPD';
  TempRPDHead.Version := 1;
  TempRPDHead.DAC := EightBitDMA;
  TempRPDHead.Phase := TempFormatChunk.Channels - 1;
  TempRPDHead.Channels := TempFormatChunk.Channels;
  TempRPDHead.Freq := TempFormatChunk.BlockAlign;
  TempRPDHead.ChannelMethod := 0;
  TempRPDHead.Size := FileSize (SourceF) - FilePos (SourceF);
  BlockWrite (DestF, TempRPDHead, SizeOf (TempRPDHead));
  Repeat
    BlockRead(SourceF, TempBuffer^, $FFFF, NumRead);
    BlockWrite(DestF, TempBuffer^, NumRead, NumWritten);
  Until (NumRead = 0) or (NumWritten <> NumRead);
  Close (SourceF);
  Close (DestF);
  FreeMem (TempBuffer, $FFFF);
End;

Function FileExists (Filename: String): Boolean;
var
 F: file;
Begin
 {$I-}
 Assign(F, FileName);
 FileMode := 0;      (* Set file access to read only *)
 Reset(F);
 Close(F);
 {$I+}
 FileExists := (IOResult = 0) and (FileName <> '');
End;

Begin
  Clrscr;
  WriteLn ('           WAV2RPD version 1.0a, By: Romesh Prakashpalan, 1994');
  WriteLn ('                            WAV2RPD is FREEWARE             ');
  Write ('Enter in WAV file to be converted: ');
  ReadLn (Source);
  If not FileExists (Source) then
  Begin
    WriteLn ('Source File Doesn''t Exist!');
    Halt;
  End;
  Write ('Enter in RPD file to convert to: ');
  ReadLn (Destination);
  If FileExists (Destination) then
  Begin
    Write ('File exists! overwrite? (''N'' for No, any other key kills it): ');
    Ch := UpCase (Readkey);
    WriteLn (Ch);
    If Ch = 'N' then Halt;
  End;

  ConvertWAV2RPD (Source, Destination);
End.
