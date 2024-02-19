var ptitle, zname, zurl, i, sname;
ptitle=document.title;

sname=new Array(10);
 sname[0]="SteiniOnline - Startseite -";
 sname[1]="SteiniOnline - Portrait -";
 sname[2]="SteiniOnline - Kleiststadt Frankfurt (Oder) -";
 sname[3]="SteiniOnline - Bildergallerie: Übersicht -";
 sname[4]="www.Metafa.de";
 sname[5]="";
 sname[6]="SteiniOnline - Lyrics (Index)-";
 sname[7]="[kostenloses Forum von com!-Online] Das Forum bei SteiniOnline";
 sname[8]="Chat bei SteiniOnline";
 sname[9]="SteiniOnline - Hotlinks -";
 sname[10]="LinuxonLaptops -Index-";

zurl=new Array(10);
 zurl[0]="index2.htm";
 zurl[1]="portr.htm";
 zurl[2]="meinestadt/index.htm";
 zurl[3]="gallerie/index.htm";
 zurl[4]="http://www.metafa.de";
 zurl[5]="http://cgi06.puretec.de/cgi-bin/gb?clsid=642eb018df4f80ce3cd8b239a695b944";
 zurl[6]="lyrics/index.htm";
 zurl[7]="http://forum.webmart.de/wmforum.cfm?id=121876";
 zurl[8]="chat.htm";
 zurl[9]="hotlinks.htm";
 zurl[10]="lol/lol_index.htm";

 zname=new Array(10);
  zname[0]="Startseite";
  zname[1]="Portrait";
  zname[2]="Kleiststadt Frankfurt (Oder)";
  zname[3]="Bildergallerie (&Uuml;bersicht)";
  zname[4]="http://www.Metafa.de";
  zname[5]="G&auml;stebuch";
  zname[6]="Lyrics (&Uuml;bersicht)";
  zname[7]="Forum";
  zname[8]="Online Chat";
  zname[9]="Hotlinks";
  zname[10]="Linux on Laptops";

 function wechsel()
   {
    var auswahl, seite;
    auswahl=document.navi.Navigator.selectedIndex;
    seite=document.navi.Navigator.options[auswahl].value;
    if (seite!=" ")
    {
    if (seite=="metafa/de/index.htm" || seite=="../metafa/de/index.htm")
    {
     window.open (seite);
     }
    else
    {
    location.href=seite;
    }
    }
     else
     {
     document.navi.reset();
     return false;
     }

  }

if (ptitle==sname[4])
 {
document.write("<hr />");
  }
else
 {
document.write("<p><hr />");
  }

document.write("<font color=\"#000000\">");
document.write("<div align=\"center\">");
document.write("<form name=\"navi\">");
document.write("<select size=\"1\" name=\"Navigator\">")
if (ptitle==sname[10] || ptitle=="Toshiba 2180CDT" || ptitle=="Toshiba T1600")
 {
 document.write ("<option value=\"http://www.cs.utexas.edu/users/kharker/linux-laptop/ \" selected=\"selected\">Zur Linux on Laptops-Homepage</option>");
 document.write ("<option value=\"http://www.mobilix.org/ \">Mobilix.Org</option>");
 document.write ("<option value=\" \">------------------------</option>");
 document.write ("<option value=\"../index2.htm \">Startseite</option>");
   }
else
 {
   if (ptitle==sname[6]  || ptitle==sname[3]   || ptitle==sname[2] || ptitle=="SteiniOnline - Lyrics -")
   {
    document.write ("<option value=\"../index2.htm \">Startseite</option>");
   }
   else
  {
  document.write ("<option value=\"./index2.htm \" selected=\"selected\">Startseite</option>");
  }
  }

for (i=1;i<=10;i++)
{
 if (ptitle != sname[i])
  {
   if (ptitle==sname[10] || ptitle=="Toshiba 2180CDT" || ptitle=="Toshiba T1600" || ptitle==sname[6]  || ptitle==sname[3]   || ptitle==sname[2] || ptitle=="SteiniOnline - Lyrics -")
   {
      document.write ("<option value=\"../"+zurl[i]+" \">"+ zname[i] +" </option>");
   }
   else
   {
    document.write ("<option value=\""+zurl[i]+" \">"+ zname[i] +" </option>");
   }}
}
document.write("</select>");
document.write("&nbsp;&nbsp;<input type=\"button\" value=\"Go!\" onClick=\"wechsel()\" />");
document.write("</form></div></font></p>");