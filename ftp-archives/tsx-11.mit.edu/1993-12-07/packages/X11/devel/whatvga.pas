
uses dos,crt;

type
  str10=string[10];


const
  mems:array[0..7] of string[5]=('64 K','128 K','192 K','256 K','512 K','768 K','1 M','2 M');
  mmmask :array[0..7] of byte=(0,0,0,0,1,3,3,7);
  mmbanks:array[0..7] of byte=(1,2,3,4,8,12,16,32);

  _64  =0;
  _128 =1;
  _192 =2;
  _256 =3;
  _512 =4;
  _768 =5;
  _1024=6;
  _2048=7;

  hx:array[0..15] of char='0123456789ABCDEF';


type
  CHIPS=(__EGA,__VGA,__chips451,__chips452,__chips453,__paradise,__video7
        ,__tseng3,__tseng4,__tridBR,__tridCS,__trid89,__everex,__ati1,__ati2
        ,__genoa,__oak,__cirrus,__aheadA,__aheadB,__ncr,__yamaha,__poach
        ,__vesa,__none);

var
  rp:registers;
  mm:byte;  {in 64k blocks}
  name:string[40];
  base,old,curbank,x:word;
  CHIP:CHIPS;

  video:string[5];
  _crt:string[20];
  secondary:string[20];
  extra:string[80];
  bytes:longint;
  ix17,lins,vseg,vgran:word;

function istr(w:word):str10;
var s:str10;
begin
  str(w,s);
  istr:=s;
end;

procedure vio(ax:word);
begin
  rp.ax:=ax;
  intr(16,rp);
end;

function rdinx(pt,inx:word):word;       {read register PT index INX}
begin
  port[pt]:=inx;
  rdinx:=port[pt+1];
end;

procedure wrinx(pt,inx,val:word);       {write VAL to register PT index INX}
begin
  port[pt]  :=inx;
  port[pt+1]:=val;
end;

procedure modinx(pt,inx,mask,nwv:word);
begin
  port[pt]:=inx;
  port[pt+1]:=(port[pt+1] and not mask)+(nwv and mask);

end;

procedure setchip23(bank:word);
begin
  if chip=__chips452 then bank:=bank shl 2 else bank:=bank shl 4;
  wrinx(base+2,16,bank);
 { wrinx(base+2,17,bank);}
end;

procedure setbank(bank:word);
var x:word;
begin
  vseg:=$a000;
  if odd(port[$3cc]) then base:=$3d4 else base:=$3b4;
  case chip of
    __chips451:wrinx(base+2,11,bank);
    __chips452:wrinx(base+2,16,bank shl 2);
    __chips453:wrinx(base+2,16,bank shl 4);
    __paradise:wrinx($3ce,9,bank shl 4);
    __video7:begin
               x:=port[$3cc] and $df;
               if (bank and 2)>0 then inc(x,32);
               port[$3c2]:=x;
               modinx($3c4,$f9,1,bank);
               modinx($3c4,$f6,$80,(bank shr 2)*5);

             end;
    __tseng3:port[$3cd]:=bank*9+64;
    __tseng4:port[$3cd]:=bank*17;
    __tridBR:;
    __tridCS,__poach,__trid89
            :begin
               wrinx($3c4,11,0);
               if rdinx($3c4,11)=0 then;
               modinx($3c4,14,$f,bank xor 2);
             end;
    __everex:begin
               x:=port[$3cc] and $df;
               if (bank and 2)>0 then inc(x,32);
               port[$3c2]:=x;
               modinx($3c4,8,$80,bank shl 7);
             end;
    __ati1:modinx($1ce,$b2,$1e,bank shl 1);
    __ati2:modinx($1ce,$b2,$ee,bank*$22);
    __genoa:wrinx($3c4,6,bank*9+64);
    __oak:wrinx($3de,17,bank*17);
    __aheadA:begin
               wrinx($3ce,13,bank shr 1);
               x:=port[$3cc] and $df;
               if odd(bank) then inc(x,32);
               port[$3c2]:=x;
             end;
    __aheadB:wrinx($3ce,13,bank*17);
    __ncr:wrinx($3c4,$18,bank shl 2);
    __vesa:begin
             rp.bx:=0;
             rp.dx:=bank*longint(64) div vgran;
             vio($4f05);
             rp.bx:=1;
             vio($4f05);
           end;
  end;
  curbank:=bank;
end;

procedure setpix(x,y,col:word);
var l:longint;
begin
  l:=y*bytes+x;
  setbank(l shr 16);
  mem[vseg:word(l)]:=col;
end;

procedure setvesa(bx:word);
var vesarec:array[0..255] of byte;
begin
  rp.bx:=bx;
  vio($4f02);
  rp.cx:=bx;
  rp.es:=sseg;
  rp.di:=ofs(vesarec);
  vio($4f01);
  vgran:=vesarec[4];
end;

procedure setchip(mde:word);
begin
  vio(mde);
  portw[$46e8]:=$1e;
  portw[$103]:=$80;
  portw[$46e8]:=$e;
  modinx(base+2,4,4,4);
  modinx(base+2,11,3,1);
end;

procedure setev(mde:word);
begin
  rp.bl:=mde;
  vio($70);
end;

procedure setwd(mde:word);
begin
  vio(mde);
  modinx($3ce,15,$17,5);
  wrinx(base,$29,$85);
  modinx(base,$2f,2,0);
end;

procedure setvideo(mde:word);
begin
  rp.bl:=mde;
  vio($6f05);
end;


procedure setmode0;        {Enter 320x200 mode}
begin
  bytes:=320;lins:=200;
  case CHIP of
    __chips451,__chips452,__chips453:setchip($13);
    __paradise:setwd($13);
  else vio($13);
  end;
end;

procedure setmode1;        {Enter 640x400 mode}
begin
  bytes:=640;lins:=400;
  case CHIP of
    __chips451,__chips452,__chips453:setchip($78);
    __paradise:setwd($5e);
    __video7:setvideo($66);
    __tseng3:begin vio($2d);lins:=350 end;
    __tseng4:vio($2f);
    __tridBR,__tridCS,__poach,__trid89:vio($5c);
    __everex:setev($14);
    __ati1,__ati2:vio($61);
    __genoa:vio($7e);
    __oak:;
    __cirrus:;
    __aheadA,__aheadB:vio($60);
    __ncr:;
    __vesa:setvesa($100);
  end;
end;

procedure setmode2;     {Enter 640x480 mode}
begin
  bytes:=640;lins:=480;
  case CHIP of
    __chips451,__chips452,__chips453:setchip($79);
    __paradise:setwd($5f);
    __video7:setvideo($67);
    __tseng3,__tseng4:vio($2e);
    __tridBR,__tridCS,__poach,__trid89:vio($5d);
    __everex:setev($30);
    __ati1,__ati2:vio($62);
    __genoa:vio($5c);
    __oak:vio($53);
    __cirrus:;
    __aheadA,__aheadB:vio($61);
    __ncr:;
    __vesa:setvesa($101);
  end;
end;

procedure setmode3;     {Enter 800x600 mode}
begin
  bytes:=800;lins:=600;
  case CHIP of
    __chips451,__chips452,__chips453:setchip($7b);
    __paradise:setwd($5c);
    __video7:setvideo($69);
    __tseng3,__tseng4:vio($30);
    __tridBR:;
    __tridCS,__poach,__trid89:vio($5e);
    __everex:setev($31);
    __ati1,__ati2:vio($63);
    __genoa:vio($5e);
    __oak:vio($54);
    __cirrus:;
    __aheadA,__aheadB:vio($61);
    __ncr:;
    __vesa:setvesa($101);
  end;
end;

procedure setmode4;        {Enter 1024x768 mode}
begin
  bytes:=1024;lins:=768;
  case CHIP of
    __tseng4:vio($38);
    __tridCS,__trid89:vio($61);
    __everex:setev($32);
    __ati2:vio($61);
    __aheadB:vio($63);
    __vesa:setvesa($105);
  end;
end;

procedure setvstart(l:longint);       {Set the display start address}
var x,y:word;
begin
  x:=l shr 2;
  y:=(l shr 18) and mmmask[mm];
  wrinx(base,13,lo(x));
  wrinx(base,12,hi(x));
  case chip of
    __tseng3:modinx(base,$23,2,y shl 1);
    __tseng4:modinx(base,$33,3,y);
    __tridcs:modinx(base,$1e,32,y shl 5);
    __trid89:begin
               modinx(base,$1e,$a0,y shl 5+128);
               wrinx($3c4,11,0);
               modinx($3c4,$e,1,y shr 1);
             end;
    __video7:modinx($3c4,$f6,$70,(y shl 4) and $30);
  __paradise:modinx($3ce,$d,$18,y shl 3);
  __chips452,__chips453:
             begin
               wrinx($3d6,12,y);
               modinx($3d6,4,4,4);
             end;
  __aheadb:modinx($3ce,$1c,3,y);

  end;
end;

procedure wrtxt(x,y:word;txt:string);      {write TXT to pos (X,Y)}
type
  pchar=array[char] of array[0..15] of byte;
var
  p:^pchar;
  c:char;
  i,j,z,b:integer;
begin
  rp.bh:=6;
  vio($1130);
  p:=ptr(rp.es,rp.bp);
  for z:=1 to length(txt) do
  begin
    c:=txt[z];
    for j:=0 to 15 do
    begin
      b:=p^[c][j];
      for i:=x+7 downto x do
      begin
        if odd(b) then setpix(i,y+j,15)
                  else setpix(i,y+j,0);
        b:=b shr 1;
      end;
    end;
    inc(x,8);
  end;
end;

procedure testvmode;          {Test pattern}
begin
  for x:=50 to bytes-50 do
  begin
    setpix(x,30,lo(x));
    setpix(x,lins-30,lo(x));
  end;
  for x:=30 to lins-30 do
  begin
    setpix(x+20,x,lo(x));
    setpix(bytes-30-x,x,lo(x));
    setpix(50,x,lo(x));
    setpix(bytes-50,x,lo(x));
  end;
  wrtxt(70,70,name+' with '+mems[mm]+'bytes.');
  wrtxt(70,100,'Mode: '+istr(bytes)+'x'+istr(lins)+' 256 color');
  if readkey=' ' then;
  textmode(3);
end;


function getbios(offs,lnn:word):string;
var s:string;
begin
  s[0]:=chr(lnn);
  move(mem[$c000:offs],s[1],lnn);
  getbios:=s;
end;

function tstrg(pt,msk:word):boolean;       {Returns true if the bits in MSK
                                            of register PT are read/writable}
var old,nw1,nw2:word;
begin
  old:=port[pt];
  port[pt]:=old and not msk;
  nw1:=port[pt] and msk;
  port[pt]:=old or msk;
  nw2:=port[pt] and msk;
  port[pt]:=old;
  tstrg:=(nw1=0) and (nw2>0);
end;

function testreg(pt,rg:word):boolean;      {Returns }
var old,nw1,nw2:word;
begin
  port[pt]:=rg;
  testreg:=tstrg(pt+1,$ff);
end;

function testreg2(pt,rg,msk:word):boolean;
var old,nw1,nw2:word;
begin
  port[pt]:=rg;
  testreg2:=tstrg(pt+1,msk);
end;

function memtst:boolean;
var ar:array[0..1023] of byte;
  x:word;
begin
  move(mem[$a000:0],ar,1024);
  for x:=0 to 1023 do
    inc(mem[$a000:x],x);

  memtst:=true;
  for x:=0 to 1023 do
    if mem[$a000:x]<>lo(ar[x]+x) then
      memtst:=false;
  move(ar,mem[$a000:0],1024);
end;

function tsengmem(bank:word):boolean;
var old:word;
begin
  old:=port[$3cd];
  port[$3cd]:=bank;
  tsengmem:=memtst;
  port[$3cd]:=old;
end;

function tridmem(bank:word):boolean;
var old:word;
begin
  old:=rdinx($3c4,14);
  port[$3c5]:=bank xor 2;
  tridmem:=memtst;
  port[$3c5]:=old;
end;

procedure _chipstech;
begin
  vio($5f00);
  if rp.al=$5f then
  begin
    case rp.bl shr 4 of
      0:name:='Chips & Tech 82c451';
      1:name:='Chips & Tech 82c452';
      2:name:='Chips & Tech 82c455';
      3:name:='Chips & Tech 82c453';
      5:name:='Chips & Tech 82c456';
    else name:='Unknown Chips & Tech';
    end;
    case rp.bl shr 4 of
      1:CHIP:=__chips452;
      3:CHIP:=__chips453;
    else chip:=__chips451;
    end;
    case rp.bh of
      1:mm:=_512;
      2:mm:=_1024;
    end;
  end;
end;

procedure _paradise;
var old,old2:word;
begin
  if getbios($7d,4)='VGA=' then
  begin
    old:=rdinx($3ce,15);
    port[$3cf]:=old and $e8+5;   {Unlock registers}
    old2:=rdinx(base,$29);
    port[base+1]:=old2 and $60+$85;
    if not testreg(base,$2b) then name:='Paradise PVGA1A'
    else if not testreg2($3c4,18,64) then name:='Western Digital WD90C00'
    else if not testreg2($3c4,16,4) then name:='Western Digital WD90C10'
                                    else name:='Western Digital WD90C11';
    port[$3ce]:=11;
    case port[$3cf] shr 6 of
       2:mm:=_512;
       3:mm:=_1024;
    end;
    wrinx(base,$29,old2);
    wrinx($3ce,15,old);
    chip:=__paradise;
  end;
end;

procedure _video7;
begin
  vio($6f00);
  if rp.bx=$5637 then
  begin
    vio($6f07);
    case rp.bl of
      $80..$ff:name:='Video7 VEGA VGA';
      $70..$7f:name:='Video7 FASTWRITE/VRAM';
      $50..$5f:name:='Video7 Version 5';
      $41..$4f:name:='Video7 1024i';
    end;
    case rp.ah and 127 of
      2:mm:=_512;
      4:mm:=_1024;
    end;
    chip:=__video7;
  end
end;

procedure _genoa;
var ad:word;
begin
  ad:=memw[$c000:$37];
  if (memw[$c000:ad+2]=$6699) and (mem[$c000:ad]=$77) then
  begin
    case mem[$c000:ad+1] of
      0:name:='Genoa 62/300';
    $11:begin
          name:='Genoa 64/500';
          mm:=_512;
        end;
    $22:name:='Genoa 6100';
    $33:name:='Genoa 51/5200 (Tseng 3000)';
    $55:begin
          name:='Genoa 53/5400 (Tseng 3000)';
          mm:=_512;
        end;
    end;
    if mem[$c000:ad+1]<$33 then chip:=__genoa else chip:=__tseng3;
  end
end;

procedure _tseng;
begin
  if tstrg($3cd,$3f) then
  begin
    if testreg2(base,$33,$f) then
    begin
      name:='Tseng ET4000';
      case rdinx(base,$37) and 11 of
       3,9:mm:=_256;
        10:mm:=_512;
        11:mm:=_1024;
      end;
      {if tsengmem($ff) then mm:=_1024
      else if tsengmem($77) then mm:=_512;}
      chip:=__tseng4;
    end
    else begin
      name:='Tseng ET3000';
      if tsengmem($7f) then mm:=_512;
      chip:=__tseng3;
    end;
  end;
end;

procedure _trident;
var chp,old,val:word;
begin
  wrinx($3c4,11,0);
  chp:=port[$3c5];
  old:=rdinx($3c4,14);
  port[$3c5]:=0;
  val:=port[$3c5];
  port[$3c5]:=old;
  if val and 15=2 then
  begin
    case chp of
      1:name:='Trident 8800BR';
      2:name:='Trident 8800CS';
      3:name:='Trident 8900';
    else name:='Unknown Trident VGA'
    end;
    case chp of
      1:chip:=__tridbr;
      2:chip:=__tridCS;
      3:chip:=__trid89;
    end;
    if (pos('Zymos Poach 51',getbios(0,255))>0) or
       (pos('Zymos Poach 51',getbios(230,255))>0) then
    begin
      name:=name+' (Zymos Poach)';
      chip:=__poach;
    end;
    if (chp>=3) then mm:=_256+rdinx(base,$1f) and 3
    else if (rdinx(base,$1f) and 2)>0 then mm:=_512;

  end;
end;

procedure _oak;
begin
  if testreg2($3de,$d,$ff) then
  begin
    name:='OAK 037C';
    if testreg2($3de,$11,$ff) then name:='OAK-067';
    if rdinx($3de,13)>127 then mm:=_512;
    chip:=__oak;
  end;
end;

procedure _cirrus;
var old,eagle:word;
begin
  old:=rdinx(base,12);
  port[base+1]:=0;
  eagle:=rdinx(base,$1f);
  wrinx($3c4,6,lo(eagle shr 4) or lo(eagle shl 4));
  if port[$3c5]=0 then
  begin
    port[$3c5]:=eagle;
    if port[$3c5]=1 then
    begin
      case eagle of
        $EC:name:='Cirrus 510/520';
        $CA:name:='Cirrus 610/620';
        $EA:name:='Cirrus Video 7 OEM'
      else name:='Unknown Cirrus Chip';
      end;
      chip:=__cirrus;
    end;
  end;
  wrinx(base,12,old);
end;

procedure _ahead;
var old:word;
begin
  portw[$3ce]:=$200f;
  old:=port[$3cf];
  case old of
    $20:begin
          name:='Ahead A';
          chip:=__aheadA;
        end;
    $21:begin
          name:='Ahead B';
          chip:=__aheadB;
        end;
  end;
end;

procedure _everex;
var x:word;
begin
  rp.bx:=0;
  vio($7000);
  if rp.al=$70 then
  begin
    x:=rp.dx shr 4;
    if (x<>$678) and (x<>$236) then     {Some Everex boards use Trident chips.}
    begin
      case rp.ch shr 6 of
        1:mm:=_512;
        2:mm:=_1024;
        3:mm:=_2048;
      end;
      name:='Everex Ev'+hx[x shr 8]+hx[(x shr 4) and 15]+hx[x and 15];
      chip:=__everex;
    end;
  end;
end;

procedure _ati;
var w:word;
begin
  if getbios($31,9)='761295520' then
  case memw[$c000:$40] of
   $3133:begin
           name:='ATI VGA Wonder';
           w:=rdinx($1ce,$bb);
           if (w and 32)>0 then mm:=_512;
           case w and 15 of
             0:_crt:='EGA';
             1:_crt:='Analog Monochrome';
             2:_crt:='Monochrome';
             3:_crt:='Analog Color';
             4:_crt:='CGA';
             6:_crt:='';
             7:_crt:='IBM 8514/A';
           else _crt:='Multisync';
           end;
           rp.bx:=$5506;
           rp.bp:=$ffff;
           rp.si:=0;
           vio($1255);
           if rp.bp=$ffff then
           begin
             name:=name+' revision 1.';
             chip:=__ati1;
           end
           else begin
             name:=name+' revision 2.';
             chip:=__ati2;
           end;
         end;
   $3233:begin
           name:='ATI EGA Wonder';
           video:='EGA';
           chip:=__ega;
         end;
  end;
end;

procedure _vesa;
begin
  vio($4f03);
  if rp.al=$4f then
  begin
    name:='VESA';
    chip:=__vesa;
  end;
end;

procedure _yamaha;
begin
  if testreg2($3d4,$7c,$7c) then
  begin
    name:='Yamaha 6388'
  end;
end;

procedure _ncr;
begin
  if testreg2($3c4,5,$ff) then
  begin
    portw[$3c4]:=5;        {Disable extended registers}
    if not testreg2($3c4,16,$ff) then
    begin
      portw[$3c4]:=$105;        {Enable extended registers}
      if testreg2($3c4,16,$ff) then
      begin
        chip:=__ncr;
        name:='NCR 77C22E';
      end;
    end;
  end;
end;

begin
  extra:='';
  _crt:='';
  chip:=__none;
  secondary:='';
  name:='';
  video:='none';
  rp.ah:=18;
  rp.bx:=$1010;
  intr(16,rp);
  if rp.bh<=1 then
  begin
    video:='EGA';
    chip:=__ega;
    if odd(port[$3cc]) then base:=$3d4 else base:=$3b4;

    mm:=rp.bl;
    vio($1a00);
    if rp.al=$1a then
    begin
      if (rp.bl<4) and (rp.bh>3) then
      begin
        old:=rp.bl;
        rp.bl:=rp.bh;
        rp.bh:=old;
      end;
      video:='MCGA';
      case rp.bl of
        2,4,6,10:_crt:='TTL Color';
        1,5,7,11:_crt:='Monochrome';
        8,12:_crt:='Analog Color';
      end;
      case rp.bh of
        1:secondary:='Monochrome';
        2:secondary:='CGA';
      end;
      if (getbios($31,9)='') and (getbios($40,2)='22') then
      begin
        video:='EGA';       {@#%@  lying ATI EGA Wonder !}
        name:='ATI EGA Wonder';

      end else
      if (rp.bl<10) or (rp.bl>12) then
      begin
        video:='VGA';
        chip:=__vga;
        mm:=_256;
        vio(19);
        _vesa;
        if name='' then _chipstech;
        if name='' then _paradise;
        if name='' then _video7;
        if name='' then _genoa;
        if name='' then _tseng;
        if name='' then _everex;
        if name='' then _trident;
        if name='' then _ati;
        if name='' then _oak;
        if name='' then _cirrus;
        if name='' then _ahead;
        if name='' then _yamaha;
        if name='' then _ncr;
      end;
    end;
  end;
  textmode(3);
  write('Video system: ',video,' with ',mems[mm]+'bytes.');
  if _crt<>'' then write(' Monitor: '+_crt);
  writeln;
  if secondary<>'' then writeln('Secondary display: '+secondary);
  if name<>'' then writeln('Chipset: '+name);
  if extra<>'' then writeln(extra);

  writeln;
  writeln;
  if chip<=__vga then
  begin
    if readkey=' ' then;
  end
  else begin
    write('Run 640x350/400 256 color test (Y/N) ?');
    if upcase(readkey)<>'N' then
    begin
      setmode1;
      testvmode;
    end;

    writeln;
    write('Run 640x480 256 color test (Y/N) ?');
    if upcase(readkey)<>'N' then
    begin
      setmode2;
      testvmode;
    end;

    writeln;
    write('Run 800x600 256 color test (Y/N) ?');
    if upcase(readkey)<>'N' then
    begin
      setmode3;
      testvmode;
    end;

    writeln;
    write('Run 1024x768 256 color test (Y/N) ?');
    if upcase(readkey)<>'N' then
    begin
      setmode4;
      testvmode;
    end;

    writeln;
    write('Run scroll test (Y/N) ?');
    if upcase(readkey)<>'N' then
    begin
      setmode0;
      IX17:=RDINX(base,$17);

      bytes:=320;
      for x:=0 to pred(mmbanks[mm]) do     {Clear video memory}
      begin
        setbank(x);
        fillchar(mem[$a000:0],$8000,0);
        fillchar(mem[$a800:0],$8000,0);
      end;

      for x:=mmbanks[mm]*10 downto 1 do
        wrtxt(20,x*20-15,'Linie '+istr(x)+' scrolling');

      for x:=1 to mmbanks[mm]*10-9 do       {Scroll text up}
      begin
        setvstart(pred(x)*20*longint(bytes));
        delay(200);
      end;
    end;



  end;
  textmode(3);
end.