  The Sierra SC11486 RAMDAC is an alternative RAMDAC for use with
  the Tseng 4000 and ATI VGA Wonder XL.

  It provides 15 bit RGB with 5 bits for each basic color.

  When in HiColor mode each pixel occupied two bytes.

  Bits  0- 4 of the pixel is the blue component.
        5- 9 of the pixel is the green component.
       10-14 of the pixel is the red component.


   This works for Tseng ET-4000 systems:


   function sethicolor(mode:word);
   var rp:registers;
   begin
     sethicolor:=false;
     rp.ax:=$10f1;
     intr($10,rp);
     if (rp.ax=$10) and (rp.bl=1) then
     begin
       rp.ax:=$10f0;
       rp.bl:=mode;      {or another video mode.}
       intr($10,rp);
       if rp.ax=$10 then
       begin                 {now in HiColor mode.}
         sethicolor:=true;
       end;
     end;
   end;

   procedure plot(xcoor,ycoor,red,green,blue:word);
   var l:longint;
   begin
     l:=(longint(640)*ycoor+xcoor)*2;
     port[$3cd]:=l shr 16;
     memw[$a000:(l and $ffff)]:=blue+(green shl 5)+(red shl 10);
   end;





   BIOS extensions  (Tseng 4000 Sierra HiColor DAC):

----------1010F0-----------------------------
INT 10 - VIDEO - Tseng ET-4000 BIOS - SET HiColor GRAPHICS MODE
        AX = 10F0h
        BL = video mode (see also AH=00h)
             32768-color modes:
                   13h = 320x200
                   2Dh = 640x350
                   2Eh = 640x480
                   2Fh = 640x400
                   30h = 800x600
Return: AX = 0010h if successful
             other on error
Note: the Tseng HiColor BIOS extensions are supported by:
          Diamond Computer Systems  SpeedStar HiColor VGA
          Everex Systems            HC VGA
          Focus Information Systems 2theMax 4000
          Cardinal Technologies     VGA732
          Orchid ProDesigner IIs
SeeAlso: AX=10F1h

Note: Not all BIOS versions support all of these modes!!
----------1010F1-----------------------------
INT 10 - VIDEO - Tseng ET-4000 BIOS - GET DAC TYPE
        AX = 10F1h
Return: AX = 0010h if succesful, errorcode if not
        BL = type of digital/analog converter
             00h normal VGA DAC
             01h Sierra SC1148x HiColor DAC
             else other HiColor DAC
SeeAlso: AX=10F0h
