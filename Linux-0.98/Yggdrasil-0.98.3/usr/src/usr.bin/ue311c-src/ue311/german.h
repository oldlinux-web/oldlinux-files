/*	GERMAN.H:	German language text strings for
                        MicroEMACS 3.11

			translated by Martin Neitzel
			unido!sigserv!neitzel@uunet.uu.net
			(C)opyright 1988 by Daniel Lawrence
*/

#define	TEXT1	"[neuer Kommando-Interpreter wird gestartet]"
/* if `CLI' is the Amiga-specific term, better keep the keyword: */
/* #define TEXT1	"[neuer CLI wird gestartet]" */

#define	TEXT2	"[Kann Filter-Datei nicht schreiben]"
#define	TEXT3	"[Ausfuehrung schlug fehl]"
#define	TEXT4	"Shell-Variable TERM nicht definiert!"
#define	TEXT5	"Terminal ist kein 'vt100'!"
#define	TEXT6	"\r\n\n[Ende]"
#define	TEXT7	"Gehe zu Zeile: "
#define	TEXT8	"[abgebrochen]"
#define	TEXT9	"[Marke %d gesetzt]"
#define	TEXT10	"[Marke %d entfernt]"
#define	TEXT11	"Keine Marke %d in diesem Fenster"
#define	TEXT12	"['Hilfe'-Datei nicht gefunden]"
#define	TEXT13	": zeige Tastenbelegung fuer "
/* >	leave the colon here...(and elsewhere) it pretends to be a command
 *	prompt
 */
#define	TEXT14	"ist nicht belegt"
#define	TEXT15	": definiere Taste fuer Funktion "
#define	TEXT16	"[Keine solche Funktion]"
#define	TEXT17	"Definitions-Tabelle VOLL!"
#define	TEXT18	": entferne Funktions-Definition fuer Taste "
#define	TEXT19	"[Taste hat keine Definition]"
#define	TEXT20	"Apropos zu Text: "
#define	TEXT21	"Definitions-Tabelle"
#define	TEXT22	"Kann Definitions-Tabelle nicht anzeigen"
#define	TEXT23	"[Defintions-Tabelle wird erstellt...]"
#define	TEXT24	"Wechsel zu Buffer"
#define	TEXT25	"Wechsel zu Buffer: "
#define	TEXT26	"Loesche Buffer"
#define	TEXT27	"Pop buffer"
#define	TEXT28	"Buffer wird gerade angezeigt"
#define	TEXT29	"Aendere Buffer-Namen zu: "
#define	TEXT30	"ACT   Modi    Groesse Buffer          Datei"
/* >   ^^^The spacing must match on this line. It is the header for the
 *	buffer list.
 *	(I extended "size" to the left to make "Groesse" fit.)
 */

#define	TEXT31	"         Globale Modi"
#define	TEXT32	"Aenderungen wegwerfen"
#define	TEXT33	"Verschluesselungs-Text: "
#define	TEXT34	"Datei: "
#define	TEXT35	"ein anderer Benutzer"

/* >The UNIX terms should stay the same in the next few entryies */
#define	TEXT36	"LOCK FEHLER -- "
#define	TEXT37	"Ueberpruefen der Existenz von %s\n"
#define	TEXT38	"Einrichten von directory %s\n"
#define	TEXT39	"erzeuge %s\n"
#define	TEXT40	"konnte \"lock\"-Datei nicht einrichten"
#define	TEXT41	"pid ist %ld\n"
#define	TEXT42	"lese lock-Datei %s\n"
#define	TEXT43	"konnte lock-Datei nicht lesen"
#define	TEXT44	"pid in %s ist %ld\n"
#define	TEXT45	"Benachrichtigung (`kill') an Prozess %ld\n"
#define	TEXT46	"Prozess beendet"
#define	TEXT47	"`kill' war nicht in Ordnung"
#define	TEXT48	"`kill' war OK; Prozess beendet"
#define	TEXT49	"Versuch: unlink %s\n"
#define	TEXT50	"konnte lock-Datei nicht loeschen"
#define	TEXT51	"zu setzende Variable: "
#define	TEXT52	"%%Keine solche Variable '%s'"
#define	TEXT53	"Wert: "
#define	TEXT54	"[Makro abgebrochen]"
#define	TEXT55	"anzuzeigende Variable: "
#define	TEXT56	"Variablen-Liste"
#define	TEXT57	"kann Variablen-Liste nicht anzeigen"
#define	TEXT58	"[Variablen-Liste wird aufgebaut...]"
#define	TEXT59	"[Umbruchspalte ist %d]"
#define	TEXT60	"Zeile %d/%d Spalte %d/%d Zeichen %D/%D (%d%%) Zeichen = 0x%x"
#define	TEXT61	"<NOT USED>"
#define	TEXT62	"Globaler Modus, der "
#define	TEXT63	"Modus, der "
#define	TEXT64	"hinzugenommen werden soll: "
#define	TEXT65	"entfernt werden soll: "
#define	TEXT66	"Kein solcher Modus!"
#define	TEXT67	"Auszugebender Text: "
#define	TEXT68	"Einzufuegender Text<META>: "
#define	TEXT69	"Zu ueberschreibender Text<META>: "
#define	TEXT70	"[Region kopiert]"
#define	TEXT71	"%%Dieser Buffer ist bereits auf einen Bereich eingeschraenkt"
#define	TEXT72	"%%Bereich muss mind. eine ganze Zeile umfassen"
#define	TEXT73	"[Buffer ist auf Bereich eingeschraenkt]"
#define	TEXT74	"%%Dieser Buffer ist nicht eingeschraenkt."
#define	TEXT75	"[Buffer ist ohne Bereichs-Einschraenkung.]"
#define	TEXT76	"Keine Marke in diesem Fenster gesetzt"
#define	TEXT77	"Fehler: Marke verloren (nicht tragisch)"
#define	TEXT78	"Suche"
#define	TEXT79	"Nicht gefunden"
#define	TEXT80	"Kein Such-Muster gesetzt"
#define	TEXT81	"Rueckwaerts-Suche"

/* The next two most probably need to be rewritten: */
#define	TEXT82	"Bedenke %d mit Sprung %d"
/* What should that mean????
 */

/* #define	TEXT83	"Its other case is %d"
 * Are we talking about upper/lower case conversions or something
 * completely different here?  I suspected so.  Otherwise, take the other
 * translation.
 */
#define	TEXT83	"Die andere zugehoerige Gross/Kleinschreibung ist %d"
/* #define	TEXT83	"Der andere zugehoerige Fall ist %d" */

#define	TEXT84	"Ersetze"
#define	TEXT85	"Ersetze interaktiv"
#define	TEXT86	"durch"
#define	TEXT87	"Ersetzen von '"
#define	TEXT88	"' durch '"
#define	TEXT89	"Abgebrochen!"
/*
 * The following couldn't be translated without interferring with the
 * expected input characters, so I let it untouched.
 */
#define	TEXT90	"(Y)es, (N)o, (!)Do rest, (U)ndo last, (^G)Abort, (.)Abort back, (?)Help: "
/* >  ^^^^   make sure this is not longer than 72 characters */

#define	TEXT91	"leeren Text ersetzt, gestoppt."
#define	TEXT92	"%d Ersetzungen"
#define	TEXT93	"%%Fehler waehrend des Loeschens"
#define	TEXT94	"%%Kein Speicherplatz mehr frei"
#define	TEXT95	"%%mceq: Was ist %d?"
#define	TEXT96	"%%Keine Zeichen in Zeichenklasse"
#define	TEXT97	"%%Zeichenklasse nicht beendet"
#define	TEXT98	"Keine Spalte fuer Umbruch (fill-column) definiert"
#define	TEXT99	"[FREIER SPEICHERPLATZ AUFGEBRAUCHT]"
#define	TEXT100	"%D Worte, %D Zeichen, %d Zeilen, durchschn. Wortlaenge %f"
#define	TEXT101	"[kann nicht zugleich suchen und zu einer Zeile gehen!]"
#define	TEXT102	"[Unsinniges Argument fuer `goto']"
#define	TEXT103	"[Abspeichern von %s]"
#define	TEXT104	"Geaenderte Buffer existieren.  Trotzdem beenden"
#define	TEXT105	"%%Makro bereits aktiv"
#define	TEXT106	"[Anfang von Makro]"
#define	TEXT107	"%%Makro nicht aktiv"
#define	TEXT108	"[Ende von Makro]"
#define	TEXT109	"[Taste unzulaessig im VIEW-Modus]"
#define	TEXT110	"[Dieses Kommando ist nicht freigegeben!]"
#define	TEXT111	"Kein Makro angegeben"
#define	TEXT112	"Makro-Nummer ausserhalb des gueltigen Bereichs"
#define	TEXT113	"Kann Makro nicht erzeugen"
#define	TEXT114	"Prozedur-Name: "
#define	TEXT115	"Fuehre Prozedur aus: "
#define	TEXT116	"Keine solche Prozedur"
#define	TEXT117	"Fuehre Buffer aus: "
#define	TEXT118	"Kein solcher Buffer"
#define	TEXT119	"%%freier Speicher aufgebraucht waehrend `while-scan'"

/* > Directive names and keywords are not translated... */
#define	TEXT120	"%%!BREAK ausserhalb einer !WHILE-Schleife"
#define	TEXT121	"%%!ENDWHILE ohne vorausgegangenes !WHILE"
#define	TEXT122	"%%!WHILE ohne zugehoeriges !ENDWHILE"
#define	TEXT123	"%%freier Speicher waehrend Makroausfuehrung aufgebraucht"
#define	TEXT124	"%%Unbekannte Direktive"
#define	TEXT125	"freier Speicher beim Abspeichern eines Makros aufgebraucht"
#define	TEXT126	"%%interner Fehler bei While-Schleife"
#define	TEXT127	"%%Kein solches Label"
#define	TEXT128	"(e)val exp, (c/x)ommand, (t)rack exp, (^G)abort, <SP>exec, <META> stop debug"
/* > ^^^ No longer than 72 chars again
 * And, again too, no translation because of expected input characters
 */

#define	TEXT129	"Auszufuehrende Datei: "
#define	TEXT130	"Makro nicht definiert"
#define	TEXT131	"Einzulesende Datei"
#define	TEXT132	"Einzufuegende Datei"
#define	TEXT133	"Finde Datei"
#define	TEXT134	"Zu betrachtende Datei"
#define	TEXT135	"[alter Buffer]"
#define	TEXT136	"Buffer-Name: "
#define	TEXT137	"Kann Buffer nicht einrichten"
#define	TEXT138	"[neue Datei]"
#define	TEXT139	"[Einlesen von Datei]"


#define	TEXT140	"Lese "
/* probably a bad choice, has to be checked against its context. */

#define	TEXT141	"I/O FEHLER, "
#define	TEXT142	"FREIER SPEICHERPLATZ VERBRAUCHT, "
#define	TEXT143	" Zeile"
#define	TEXT144	"Abspeichern in Datei: "
#define	TEXT145	"Kein Dateiname"
/*
 * I wonder how the next two msgs are meant... I guess the user is encouraged
 * to do a write-file instead of a save-file.
 * #define	TEXT146	"Truncated file..write it out"
 * #define	TEXT147	"Narrowed Buffer..write it out"
 */
#define	TEXT146	"Unvollstaendige Datei; [`write-file' benutzen]"
#define	TEXT147	"Buffer auf Bereich eingeschraenkt; [`write-file' benutzen]"

#define	TEXT148	"[Datei wird geschrieben...]"
#define	TEXT149	"[Datei geschrieben: "
#define	TEXT150	", gespeichert als "
#define	TEXT151	"Name: "
#define	TEXT152	"[Keine solche Datei]"
#define	TEXT153	"[Datei wird eingefuegt]"
#define	TEXT154	"Eingefuegt: "
#define	TEXT155	"Kann Datei nicht zum Schreiben oeffnen"
#define	TEXT156	"Fehler beim Schliessen der Datei"
#define	TEXT157	"Schreib-I/O-Fehler"
#define	TEXT158	"Fehler beim Lesen der Datei"
#define	TEXT159	"%Brauche Funktionstasten-Nummer"
#define	TEXT160	"%Nummer fuer Funktionstaste ausserhalb vom gueltigen Bereich"
#define	TEXT161	"Label-Inhalt: "

/* Again, unchanged due to expected characters: */
#define	TEXT162	" [y/n]? "

#define	TEXT163	"Kein Default"
#define	TEXT164	"[Suche erfolglos]"
#define	TEXT165	"inkrementelle Suche: "
#define	TEXT166	"? Suchtext zu lang"
#define	TEXT167	"? Kommando zu lang"
#define	TEXT168	"%%Kann Text nicht einfuegen"
#define	TEXT169	"Eingefuegt"
#define	TEXT170	"bug: linsert"
#define	TEXT171	"Ersetzt"
#define	TEXT172	"%%freier Speicherplatz beim Ueberschreiben verbraucht"
#define	TEXT173	"LOCK FEHLER: Lock-Tabelle voll"
#define	TEXT174	"Kann kein `LOCK' auf Datei legen, freier Speicherplatz verbraucht"
#define	TEXT175	"LOCK (Dateizugriff reserviert)"

/* If ", override?" in TEXT177 refers to a lock and not to a file, better use
 * ", ignorieren?" (==ignore) or ", uebernehmen?" (==take over) instead.
 */
#define	TEXT176	"Datei wird benutzt von "
#define	TEXT177	", ueberschreiben?"

#define	TEXT178	"[kann System-Fehlertext nicht herausfinden]"
#define	TEXT179	"  Ueber MicroEmacs"
#define	TEXT180	"%%Keine solche Aufloesung"
#define	TEXT181	"%%Aufloesung unzulaessig fuer diesen Bildschirm"
#define	TEXT182	"Environment-Variable TERM nicht definiert!"
#define	TEXT183	"Unbekannter Terminal-Typ %s!"
#define	TEXT184	"Termcap-Eintrag unvollstaendig (`lines')"
#define	TEXT185	"Termcap-Eintrag unvollstaendig (`columns')"
#define	TEXT186	"Unvollstaendiger Termcap-Eintrag\n"
#define	TEXT187	"Terminal-Beschreibung zu lang!\n"
#define	TEXT188	"[Ende]"
#define	TEXT189	"Kann Eintrag fuer Terminal-Typ nicht finden.\n"

/* > Don't translate the VMS commands in the next 5 lines */
#define	TEXT190	"Ueberpruefe Terminal-Typ mit \"SHOW TERMINAL\" oder\n"
#define	TEXT191	"versuche eine Einstellung mit \"SET TERMINAL/INQUIRE\"\n"
#define	TEXT192	"Der Terminaltyp hat nicht genuegend Faehigkeiten fuer\n"
#define	TEXT193	"MicroEMACS.  Versuche ein anderes Terminal oder ueberpruefe\n"
#define	TEXT194	"den Typ mit \"SHOW TERMINAL\".\n"

#define	TEXT195	"Kann Kanal zum Terminal nicht oeffnen.\n"
#define	TEXT196	"Kann Terminal-Einstellungen nicht herausfinden.\n"
#define	TEXT197	"Kann Terminal-Einstellungen nicht aendern.\n"
#define	TEXT198	"I/O error (%d,%d)\n"
#define	TEXT199	"[DCL wird gestartet]\r\n"
#define	TEXT200	"[DCL wird aufgerufen]\r\n"
#define	TEXT201	"[Noch nicht unter VMS verfuegbar]"
#define	TEXT202	"Terminal-Typ weder 'vt52' noch 'z19' !"
#define	TEXT203	"Fenster-Nummer ausserhalb des zulaessigen Bereichs"
#define	TEXT204	"Kann dieses Fenster nicht loeschen"
#define	TEXT205	"Kann ein %d-Zeilen-Fenster nicht weiter teilen"
#define	TEXT206	"Nur ein Fenster"
#define	TEXT207	"Unmoegliche Aenderung"
#define	TEXT208	"[Kein solches Fenster existiert]"
#define	TEXT209	"%%Bildschirmgroesse ausserhalb des gueltigen Bereichs"
#define	TEXT210	"%%Bildschirmbreite ausserhalb des gueltigen Bereichs"
#define	TEXT211	"Funktion-Liste"
#define	TEXT212	"kann Funktion-Liste nicht anzeigen"
#define	TEXT213	"[Funktion-Liste wird aufgebaut...]"
#define	TEXT214 "%%No such file as %s"
#define	TEXT215	": definiere Makro fuer Funktion "
#define TEXT216 "Cannot read/write directories!!!"
#define TEXT217 "[Not available yet under AOS/VS]"
#define	TEXT218	"Append file: "
#define TEXT219 "%%Macro Failed"
#define	TEXT220	"Line %d/%d Col %d/%d Char %D/%D (%d%%) char = 0x%x%x"
#define TEXT221 "Too many groups"
#define TEXT222 "Group not ended"
#define	TEXT223 "%%Column origin out of range"
#define	TEXT224 "%%Row origin out of range"
#define	TEXT225 "[Switched to screen %s]"
#define	TEXT226	"%%Can not kill an executing buffer"
#define	TEXT227 "\n--- Press any key to Continue ---"
#define TEXT228 "[Kill ring cleared]"

