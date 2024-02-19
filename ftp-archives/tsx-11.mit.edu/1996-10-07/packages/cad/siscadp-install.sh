#!/bin/sh
###########################################################################
# Installationsprozedur fuer SISCAD-P
# Copyright (c) 1993-1995 STAEDTLER MARS GmbH & Co
###########################################################################
# @(#)install	1.21 of 7/28/95

SISCADPARCHIVE=`ls siscadp-*.tar.*`

VERSION=`echo ${SISCADPARCHIVE}| sed -e 's|siscadp-||' -e 's|\(.*\)\.tar.*|\1|'`

INSTALLBASE=/usr/siscadp-${VERSION}
BACKUPDIR=/usr/siscadp.bak             # hier wird alles gesichert was 
                                       # neuer ist als das LOGFILE der
                                       # bereits installierten Version 
SISCAD_HOME=/usr/siscadp               # wird symlink auf installierte Vers.
LOGFILE=${SISCAD_HOME}/install.log     # protokollfile zur Installation

SELECT_LANGUAGE=/usr/siscadp/exe/sislanguage
SISSTARTER=/usr/siscadp/exe/start
SISBINFILES="siscadp"
BINDIR=/usr/bin


##############
set_language()   # ask for SISLANGUAGE and set it to DEUTSCH or ENGLISH 
##############
{
cat <<EOT

 SISCAD-P Installation 
 ~~~~~~~~~~~~~~~~~~~~~
 Please select the language for which you like to install SISCAD-P 
 
 Bitte waehlen Sie die Sprache in welcher Sie SISCAD-P installieren wollen 

EOT

while true
do
  echo_nl " d = DEUTSCH  e = ENGLISH : " 
  read x
  case ${x} in
    [dD] ) SISLANGUAGE=DEUTSCH; export SISLANGUAGE; break ;;
    [eE] ) SISLANGUAGE=ENGLISH; export SISLANGUAGE; break ;;
    *    ) continue ;;
  esac
done
}


####################
init_text_messages() 
####################
{
case $SISLANGUAGE in

DEUTSCH)
TXT_CONTINUE="Weiter mit RETURN"
TXT_ANSWERYN="Bitte antworten Sie mit j/n : "
TXT_INSTABORT="Installation wurde abgebrochen !"
TXT_EXECROOT="Dieses Programm kann nur als ROOT ausgefuehrt werden"
TXT_SISSIZE="SISCAD-P Groesse im Installationsverzeichnis wird ermittelt ..." 
TXT_DISKOVERVIEW="Festplattenplatz Uebersicht:"
TXT_SISKB1="SISCAD-P benotigt "
TXT_SISKB2="KByte im Verzeichnis "
TXT_INSTDIR1="Falls Sie SISCAD-P in einem anderen Verzeichnis installieren wollen,"
TXT_INSTDIR2="geben Sie jetzt das  gewuenschte Installations-Verzeichnis ein  oder"
TXT_INSTDIR3="RETURN fuer"
TXT_INSTDIR="Installations-Verzeichnis ? "
TXT_YESNO="[j/n]"
TXT_BUPDIR1="Das Sicherungs-Verzeichnis" 
TXT_BUPDIR2="wird neu angelegt"
TXT_SAVEOLD="Sichern modifizierter Daten der alten Version in" 
TXT_SAVEERR="Fehler beim Sichern der Dateien" 
TXT_REMOVED="wird geloescht"
TXT_DIR="Das Verzeichnis"
TXT_CREATING="wird neu angelegt"
TXT_SISCREATE="SISCAD-P Dateien werden angelegt"
TXT_CREATEERR="Fehler beim Anlegen der Dateien" 
TXT_INSTEND1="Falls Sie keine weiteren Installationen dieser Version durchfuehren wollen," 
TXT_INSTEND2="so koennen Sie jetzt"
TXT_INSTEND3="und install loeschen." 
TXT_DOYOULIKE="Wollen Sie"
TXT_INSTALLRM="und install jetzt loeschen"
TXT_WELCOME="Willkommen bei der Installation von STAEDTLER's"
TXT_CONTINST="Soll die Installation fortgesetzt werden ?"
TXTCMD_SUCCESS=TXTCMD_SUCCESS_DE
;;

ENGLISH)
TXT_CONTINUE="Press RETURN to continue"
TXT_ANSWERYN="Please answer with y/n : "
TXT_INSTABORT="Installation aborted !"
TXT_EXECROOT="This program can be executed as ROOT only"
TXT_SISSIZE="Evaluating SISCAD-P size .."
TXT_DISKOVERVIEW="Disk space overview:"
TXT_SISKB1="SISCAD-P needs "
TXT_SISKB2="KByte in directory "
TXT_INSTDIR1="If you wish to install SISCAD-P in another directory then"
TXT_INSTDIR2="enter your favorite directory or"
TXT_INSTDIR3="press RETURN for"
TXT_INSTDIR="Install-directory ? "
TXT_YESNO="[y/n]"
TXT_BUPDIR1="Backup directory" 
TXT_BUPDIR2="will be created"
TXT_SAVEOLD="Saving modified files of the old version" 
TXT_SAVEERR="Error while saving files occured" 
TXT_REMOVED="removing"
TXT_DIR="directory"
TXT_CREATING="created"
TXT_SISCREATE="Creating SISCAD-P files"
TXT_CREATEERR="Error while creating files occured" 
TXT_INSTEND1="If you don't like to install further copies of this version," 
TXT_INSTEND2="then"
TXT_INSTEND3="and install can now be removed." 
TXT_DOYOULIKE="Do you like"
TXT_INSTALLRM="and install to be removed now"
TXT_WELCOME="Welcome to the installation of STAEDTLER's"
TXT_CONTINST="Do you like to continue with the installation ?"
TXTCMD_SUCCESS=TXTCMD_SUCCESS_EN
;;
esac
}

TXTCMD_SUCCESS_DE()
{
cat <<EOT
   wurde erfolgreich abgeschlossen. Um mit SISCAD-P zu arbeiten, gehen Sie
   wie folgt vor:
    
   o SISCAD-P Dokumentation installieren
   o SISCAD-P Copyright und Lizenzbestimmungen lesen
   o SISCAD-P Benutzer einrichten
   o Drucker und Plotter einrichten
   o als SISCAD-P Benutzer SISCAD-P mit '/usr/bin/siscadp' starten  

   Weitere Einzelheiten dazu finden Sie in der
   'SISCAD-P Inbetriebnahmeanleitung' -- /usr/siscadp/dok/install*

EOT
}

TXTCMD_SUCCESS_EN()
{
cat <<EOT
   has been successfully installed. Before starting your work with SISCAD-P,
   please do the following:
    
   o install the SISCAD-P online documentation 
   o read the SISCAD-P copyright and license polices 
   o install a SISCAD-P user account  
   o configure printer and plotter devices 
   o as SISCAD-P user start SISCAD-P with '/usr/bin/siscadp'

   Further details to these steps above you can find in the 
   'SISCAD-P Installation Guide' -- /usr/siscadp/dok/install*

EOT
}


OS=`uname -s`

case ${OS} in
  Linux |\
  IRIX   ) TARPREFIX="" ;;
  *      ) TARPREFIX="./" ;;
esac

case ${OS} in
  SunOS  ) LNCMD="/usr/5bin/ln -fs" ;;
  *      ) LNCMD="ln -fs" ;;
esac

case ${OS} in
  AIX    ) TEST_LN="-L" ;;
  IRIX   ) TEST_LN="-l" ;;
  *      ) TEST_LN="-h" ;;
esac

SIZEFILE=${TARPREFIX}.siscad_size      # SISCAD-P Groesse im Installdir.
SOURCEDIR=`pwd`


PATH=${PATH}:/usr/lbin
export PATH

# 
# Hilfsfunktionen
#

echo_nl()
{
  case ${OS} in
    IRIX   ) echo "$1 \c" ;;
    *      ) echo -n "$1" ;;
  esac
}

antwort()
{
  echo_nl "$1 " ; read $2
}
 
weiter_mit_cr()
{
  antwort $TXT_CONTINUE dummy
}

askyn()
{
  while true
  do
    read x
    case ${x} in
      [jJyY] ) return 0 ;;
      ""     ) return 1 ;;
      [nN]   ) return 1 ;;
      *      ) echo_nl "$TXT_ANSWERYN" 
    esac
  done
}
 
stopMsg()
{
  echo $TXT_INSTABORT 
  echo $1
  exit 1
}

is_root()
{ 
   ID=`id | grep root`

   if [ "${ID}" = "" ]
   then
     stopMsg "$TXT_EXECROOT" 
   fi
}


select_instdir()    # out INSTALLBASE
################
{
  echo $TXT_SISSIZE 
  #zcat ${SOURCEDIR}/${SISCADPARCHIVE} | tar xf - ${SIZEFILE} >/dev/null
  SIZE=25329
  #rm ${SIZEFILE}

  while true
  do
    echo
    echo $TXT_DISKOVERVIEW 
    echo
    df
    echo "
$TXT_SISKB1 ${SIZE} $TXT_SISKB2 ${INSTALLBASE}

$TXT_INSTDIR1
$TXT_INSTDIR2
$TXT_INSTDIR3 ${INSTALLBASE}
"
    echo_nl "$TXT_INSTDIR   [${INSTALLBASE}] : "
    read dummy

    if [ "${dummy}" != "" ]
    then 
      INSTALLBASE="${dummy}"
    fi
    echo_nl "$TXT_INSTDIR ${INSTALLBASE} ok ?  $TXT_YESNO : " ; 
    askyn && { 
             break ; } ;
  done
}

check_oldversion()    # rm old SISCAD-P version ; save modified files
##################
{
  if [ -f ${LOGFILE} ]
  then
    CWD=`pwd`
    if [ ! -d  ${BACKUPDIR} ]
    then
      echo
      echo "$TXT_BUPDIR1 ${BACKUPDIR} $TXT_BUPDIR2 "
      mkdir ${BACKUPDIR}
    fi
    cd ${SISCAD_HOME}
    #
    # Alles sichern, was Kunde seit der Installation eventuell
    # geaendert haben koennte, jedoch nicht sollte !!!!
    # Zur Ermittlung der geaenderten Dateien zuerst auf das Verzeichnis
    # setzen und anschliessend im find mit . arbeiten, da symbolische Links
    # im find-Befehl gesondert behandelt werden.
    #
    FILES2SAVE=`find . -type f -newer ${LOGFILE} -print`
    if [ " " != " ${FILES2SAVE}" ]
    then
      echo
      echo $TXT_SAVEOLD ${BACKUPDIR} ...
      tar cf - `echo ${FILES2SAVE}` | (cd ${BACKUPDIR}; tar xvf -) ||\
      stopMsg "$TXT_SAVEERR"
    fi 

    if [ ${TEST_LN} ${SISCAD_HOME} ]
    then
      # Loeschen des gelinkten Verzeichnisses
      LN_DIR=`ls -l ${SISCAD_HOME} | awk -F\> '{print $2}'`
      RM_DIR="${LN_DIR}"
    else
      LN_DIR=""
      RM_DIR="${SISCAD_HOME}"
    fi

    cd ${CWD}
    echo
    echo ${RM_DIR} $TXT_REMOVED 
    rm -rf ${LN_DIR} ${SISCAD_HOME} 2>&1 >/dev/null
  fi 
}

inst_newversion()     # install new SISCAD-P
#################
{
  if [ ! -d  ${INSTALLBASE} ]
  then
    echo
    echo "$TXT_DIR ${INSTALLBASE} $TXT_CREATING"
    mkdir ${INSTALLBASE} 
  fi

  echo
  echo $TXT_SISCREATE 

  cd ${INSTALLBASE}
  zcat ${SOURCEDIR}/${SISCADPARCHIVE} | tar xvf - ||\
       stopMsg "$TXT_CREATEERR" 

  if [ "${SISCAD_HOME}" != "${INSTALLBASE}" ]
  then
    echo "Link ${INSTALLBASE} -> ${SISCAD_HOME}"
    ${LNCMD} ${INSTALLBASE} ${SISCAD_HOME}
  fi 

  for i in ${SISBINFILES}
  do
    echo "Link ${SISSTARTER} -> ${BINDIR}/${i}"
    ${LNCMD} ${SISSTARTER} ${BINDIR}/${i}
  done

  cd ${SOURCEDIR}
}


#
# Beginn der SISCAD-P Installation
#

clear
set_language
init_text_messages
is_root
clear

echo_nl "


    $TXT_WELCOME

    #####   ###   #####    #####      #     ######            ######
   #     #   #   #     #  #     #    # #    #     #           #     #
   #         #   #        #         #   #   #     #           #     #
    #####    #    #####   #        #     #  #     #   #####   ######
         #   #         #  #        #######  #     #           #
   #     #   #   #     #  #     #  #     #  #     #           #
    #####   ###   #####    #####   #     #  ######            #

   Version ${VERSION}

   $TXT_CONTINST $TXT_YESNO : " 

askyn || stopMsg " " 

clear

select_instdir

check_oldversion

inst_newversion

$SELECT_LANGUAGE $SISLANGUAGE

echo_nl "

$TXT_INSTEND1
$TXT_INSTEND2 ${SISCADPARCHIVE} $TXT_INSTEND3 

$TXT_DOYOULIKE ${SISCADPARCHIVE} $TXT_INSTALLRM ? $TXT_YESNO : "
askyn && rm ${SISCADPARCHIVE} install 

clear

echo "

    #####   ###   #####    #####      #     ######            ######
   #     #   #   #     #  #     #    # #    #     #           #     #
   #         #   #        #         #   #   #     #           #     #
    #####    #    #####   #        #     #  #     #   #####   ######
         #   #         #  #        #######  #     #           #
   #     #   #   #     #  #     #  #     #  #     #           #
    #####   ###   #####    #####   #     #  ######            #

   Version ${VERSION}

"
$TXTCMD_SUCCESS
