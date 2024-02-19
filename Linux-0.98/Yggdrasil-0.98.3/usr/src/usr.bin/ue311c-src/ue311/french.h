/*	FRENCH.H:	French language text strings for
                        MicroEMACS 3.11

			(C)opyright 1988 by Daniel Lawrence
			Translated by Bertrand DECOUTY, 10/88
			decouty@caroline.irisa.fr, 
			{uunet,mcvax,inria}!irisa!decouty
			IRISA-INRIA, Campus de Beaulieu, F-35042 Rennes Cedex
*/

#define	TEXT1	"[Lancement d'un nouvel interpreteur]"		/*"[Starting new CLI]"*/
#define	TEXT2	"[Ecriture du fichier filtre impossible]"	/*"[Cannot write filter file]"*/
#define	TEXT3	"[L'execution a echoue]"			/*"[Execution failed]"*/
#define	TEXT4	"Variable TERM non definie"			/*"Shell variable TERM not defined!"*/
#define	TEXT5	"Terminal non 'vt100'!"				/*"Terminal type not 'vt100'!"*/
#define	TEXT6	"\r\n\n[Termine]"				/*"\r\n\n[End]"*/
#define	TEXT7	"Aller a la ligne : "				/*"Line to GOTO: "*/
#define	TEXT8	"[Avorte]"					/*"[Aborted]"*/
#define	TEXT9	"[Marque %d posee]"				/*"[Mark set]"*/
#define	TEXT10	"[Marque %d enlevee]"				/*"[Mark removed]"*/
#define	TEXT11	"Pas de marque %d dans cette fenetre"		/*"No mark %d in this window"*/
#define	TEXT12	"[Fichier Help absent]"				/*"[Help file is not online]"*/
#define	TEXT13	": decrire-la-touche "				/*": describe-key "*/
#define	TEXT14	"Non liee"					/*"Not Bound"*/
#define	TEXT15	": lier-une-touche "				/*": bind-to-key "*/
#define	TEXT16	"[Fonction inexistante]"			/*"[No such function]"*/
#define	TEXT17	"La table des liens est PLEINE!"		/*"Binding table FULL!"*/
#define	TEXT18	": touche-a-delier "				/*": unbind-key "*/
#define	TEXT19	"[Touche non liee]"				/*"[Key not bound]"*/
#define	TEXT20	"Chaine a-propos: "				/*"Apropos string: "*/
#define	TEXT21	"Liste des liens"				/*"Binding list"*/
#define	TEXT22	"Affichage de la liste des liens impossible"	/*"Can not display binding list"*/
#define	TEXT23	"[Creation de la liste des liens]"		/*"[Building binding list]"*/
#define	TEXT24	"Tampon a utiliser"				/*"Use buffer"*/
#define	TEXT25	"Tampon a utiliser: "				/*"Use buffer: "*/
#define	TEXT26	"Tampon des effacements"			/*"Kill buffer"*/
#define	TEXT27	"Voyons des effacements"			/*"View buffer"*/
#define	TEXT28	"Le tampon est affiche"				/*"Buffer is being displayed"*/
#define	TEXT29	"Changer le nom du tampon en: "			/*"Change buffer name to: "*/

/*		"ACT   Modes      Size Buffer          File"*/
#define	TEXT30	"ACT   Modes    Taille Tampon          Fichier"
#define	TEXT31	"         Modes Globaux"			/*"         Global Modes"*/
#define	TEXT32	"Annuler les modifications"			/*"Discard changes"*/
#define	TEXT33	"Chaine d'encryptage: "				/*"Encryption String: "*/
#define	TEXT34	"Fichier: "					/*"File: "*/
#define	TEXT35	"autre utilisateur"				/*"another user"*/
#define	TEXT36	"ERREUR AU VERROUILLAGE"			/*"LOCK ERROR -- "*/
#define	TEXT37	"verification de l'existence de %s\n"		/*"checking for existence of %s\n"*/
#define	TEXT38	"creation du repertoire %s\n"			/*"making directory %s\n"*/
#define	TEXT39	"en cours de creation de %s\n"			/*"creating %s\n"*/
#define	TEXT40	"creation d'un fichier verrouille impossible"	/*"could not create lock file"*/
#define	TEXT41	"pid: %ld\n"					/*"pid is %ld\n"*/
#define	TEXT42	"lecture du fichier verrouille %s\n"		/*"reading lock file %s\n"*/
#define	TEXT43	"lecture du fichier verrouille impossible"	/*"could not read lock file"*/
#define	TEXT44	"le pid de %s est %ld\n"			/*"pid in %s is %ld\n"*/
#define	TEXT45	"envoi d'un signal au processus %ld\n"		/*"signaling process %ld\n"*/
#define	TEXT46	"le processus existe"				/*"process exists"*/
#define	TEXT47	"kill a echoue"					/*"kill was bad"*/
#define	TEXT48	"succes du kill; le processus existe"		/*"kill was good; process exists"*/
#define	TEXT49	"unlink de %s en cours\n"				/*"attempting to unlink %s\n"*/
#define	TEXT50	"destruction impossible d'un fichier verrouille"	/*"could not remove lock file"*/
#define	TEXT51	"Variable a positionner: "				/*"Variable to set: "*/
#define	TEXT52	"%%La variable '%s' n'existe pas"		/*"%%No such variable as '%s'"*/
#define	TEXT53	"Valeur: "					/*"Value: "*/
#define	TEXT54	"[Macro terminee anormalement]"			/*"[Macro aborted]"*/
#define	TEXT55	"Affichage de la variable: "			/*"Variable to display: "*/
#define	TEXT56	"Liste des variables"				/*"Variable list"*/
#define	TEXT57	"Affichage impossible de la liste des variables"	/*"Can not display variable list"*/
#define	TEXT58	"[Construction de la liste des variables]"	/*"[Building variable list]"*/
#define	TEXT59	"[Marge droite colonne : %d]"			/*"[Fill column is %d]"*/

/*		"Line %d/%d Col %d/%d Char %D/%D (%d%%) char = 0x%x"*/
#define	TEXT60	"Ligne %d/%d Col %d/%d Car %D/%D (%d%%) car = 0x%x"

#define	TEXT61	"<NOT USED>"					/*"not used"*/
#define	TEXT62	"Mode global a "				/*"Global mode to "*/
#define	TEXT63	"Mode a "					/*"Mode to "*/
#define	TEXT64	"ajouter: "					/*"add: "*/
#define	TEXT65	"supprimer: "					/*"delete: "*/
#define	TEXT66	"Mode inexistant!"				/*"No such mode!"*/
#define	TEXT67	"Message a ecrire: "				/*"Message to write: "*/
#define	TEXT68	"Chaine a inserer<META>: "			/*"String to insert<META>: "*/
#define	TEXT69	"Chaine a substituer<META>: "			/*"String to overwrite<META>: "*/
#define	TEXT70	"[Region copiee]"				/*"[region copied]"*/
#define	TEXT71	"%%Ce tampon est deja diminue"			/*"%%This buffer is already narrowed"*/

/*		"%%Must narrow at least 1 full line"*/
#define	TEXT72	"%%La diminution doit etre d'au moins une ligne complete"

#define	TEXT73	"[Le tampon est diminue]"			/*"[Buffer is narrowed]"*/
#define	TEXT74	"%%Ce tampon n'est pas diminue"			/*"%%This buffer is not narrowed"*/
#define	TEXT75	"[Tampon elargi]"				/*"[Buffer is widened]"*/
#define	TEXT76	"Pas de marque dans cette fenetre"		/*"No mark set in this window"*/
#define	TEXT77	"Bug: marque perdue"				/*"Bug: lost mark"*/
#define	TEXT78	"Recherche avant"				/*"Search"*/
#define	TEXT79	"Pas trouve"					/*"Not found"*/
#define	TEXT80	"Chaine non initialisee"			/*"No pattern set"*/
#define	TEXT81	"Recherche arriere"				/*"Reverse search"*/
#define	TEXT82	"Prenant en compte %d avec un saut de %d"	/*"Considering %d with jump %d"*/
#define	TEXT83	"Un autre cas est %d"				/*"Its other case is %d"*/
#define	TEXT84	"Remplacer"					/*"Replace"*/
#define	TEXT85	"Remplacer avec confirmation"			/*"Query replace"*/
#define	TEXT86	"par"						/*"with"*/
#define	TEXT87	"Remplacer '"					/*"Replace '"*/
#define	TEXT88	"' par '"					/*"' with '"*/
#define	TEXT89	"Avorte!"					/*"Aborted!"*/

/*		"(Y)es, (N)o, (!)Do rest, (U)ndo last, (^G)Abort, (.)Abort back, (?)Help: "*/
#define	TEXT90	"(O)Oui, (N)on, (!)Remp.tout, (U)ndo prec., (^G)Fin,(.)Fin debut, (?)Aide: "

#define	TEXT91	"Chaine vide remplacee, arret"			/*"Empty string replaced, stopping."*/
#define	TEXT92	"%d substitutions"				/* same in French */
#define	TEXT93	"%%ERREUR pendant la suppression"		/*"%%ERROR while deleting"*/
#define	TEXT94	"%%Plus de memoire disponible"			/*"%%Out of memory"*/
#define	TEXT95	"%%mceq: qu'est-ce que %d?"			/*"%%mceq: what is %d?"*/
#define	TEXT96	"%%Pas de caracteres dans cette classe"		/*"%%No characters in character class"*/
#define	TEXT97	"%%Classe de caracteres non terminee"		/*"%%Character class not ended"*/
#define	TEXT98	"Marge droite non initialisee"			/*"No fill column set"*/
#define	TEXT99	"[PLUS DE MEMOIRE]"				/*"[OUT OF MEMORY]"*/

/*		"Words %D Chars %D Lines %d Avg chars/word %f"*/
#define TEXT100 "Mots %D Car %D Lignes %d Moyenne car/mots %f"
#define	TEXT101	"[Je ne peux pas chercher et me deplacer en meme temps!]"	/*"[Can not search and goto at the same time!]"*/
#define	TEXT102	"[Aller-a: Parametre errone]"				/*"[Bogus goto argument]"*/
#define	TEXT103	"[Sauvegarde de %s]"					/*"[Saving %s]"*/
#define	TEXT104	"Des tampons ont ete modifies. On quitte malgre tout"	/*"Modified buffers exist. Leave anyway"*/
#define	TEXT105	"%%Macro deja active"					/*"%%Macro already active"*/
#define	TEXT106	"[Debut de la macro]"				/*"[Start macro]"*/
#define	TEXT107	"%%Macro non active"				/*"%%Macro not active"*/
#define	TEXT108	"[Fin de la macro]"				/*"[End macro]"*/
#define	TEXT109	"[Touche interdite dans le mode VIEW]"		/*"[Key illegal in VIEW mode]"*/
#define	TEXT110	"[Commande a usage RESTREINT]"			/*"[That command is RESTRICTED]"*/
#define	TEXT111	"Pas de macro indiquee"				/*"No macro specified"*/
#define	TEXT112	"Numero de macro hors limite"			/*"Macro number out of range"*/
#define	TEXT113	"Creation de macro impossible"			/*"Can not create macro"*/
#define	TEXT114	"Nom de la procedure: "				/*"Procedure name: "*/
#define	TEXT115	"Executer la procedure: "			/*"Execute procedure: "*/
#define	TEXT116	"Procedure inexistante"				/*"No such procedure"*/
#define	TEXT117	"Executer le tampon: "				/*"Execute buffer: "*/
#define	TEXT118	"Tampon inexistant"				/*"No such buffer"*/
#define	TEXT119	"%%Plus de memoire pendant la recherche"	/*"%%Out of memory during while scan"*/

#define	TEXT120	"%%!BREAK en-dehors d'une boucle !WHILE"	/*"%%!BREAK outside of any !WHILE loop"*/
#define	TEXT121	"%%!ENDWHILE sans !WHILE"		/*"%%!ENDWHILE with no preceding !WHILE"*/
#define	TEXT122	"%%!WHILE sans !ENDWHILE"			/*"%%!WHILE with no matching !ENDWHILE"*/
#define	TEXT123	"%%Plus de memoire pendant l'execution d'une macro"	/*"%%Out of Memory during macro execution"*/
#define	TEXT124	"%%Directive inconnue"					/*"%%Unknown Directive"*/
#define	TEXT125	"Plus de memoire pour l'enregistrement d'une macro"	/*"Out of memory while storing macro"*/
#define	TEXT126	"%%Erreur interne boucle While"				/*"%%Internal While loop error"*/
#define	TEXT127	"%%Etiquette inexistante"			/*"%%No such label"*/

/*		"(e)val exp, (c/x)ommand, (t)rack exp, (^G)abort, <SP>exec, <META>stop debug"*/
#define	TEXT128	"(e)val exp, (c/x)ommand, (t)rack exp, (^G)avorte,<SP>exec, <META> stop debug"

#define	TEXT129	"Executer le fichier: "				/*"File to execute: "*/
#define	TEXT130	"Macro non definie"				/*"Macro not defined"*/
#define	TEXT131	"Lire le fichier"				/*"Read file: "*/
#define	TEXT132	"Inserer le fichier"				/*"Insert file: "*/
#define	TEXT133	"Charger le fichier"				/*"Find file: "*/
#define	TEXT134	"Voir le fichier"				/*"View file: "*/
#define	TEXT135	"[Ancien tampon]"				/*"[Old buffer]"*/
#define	TEXT136	"Nom du tampon: "				/*"Buffer name: "*/
#define	TEXT137	"Creation du tampon impossible"			/*"Cannot create buffer"*/
#define	TEXT138	"[Nouveau fichier]"				/*"[New file]"*/
#define	TEXT139	"[Lecture du fichier en cours]"			/*"[Reading file]"*/
#define	TEXT140	"Lu "						/*"Read "*/
#define	TEXT141	"ERREUR ENTREE/SORTIE"				/*"I/O ERROR, "*/
#define	TEXT142	"PLUS DE MEMOIRE"				/*"OUT OF MEMORY, "*/
#define	TEXT143	" ligne"					/*" line"*/
#define	TEXT144	"Ecrire le fichier: "				/*"Write file: "*/
#define	TEXT145	"Pas de nom de fichier"				/*"No file name"*/
#define	TEXT146	"Fichier tronque..a ecrire ailleurs"		/*"Truncated file..write it out"*/

#define	TEXT147	"Tampon raccourci.. a sauvegarder"		/*"Narrowed Buffer..write it out"*/
#define	TEXT148	"[Ecriture en cours...]"			/*"[Writing...]"*/
#define	TEXT149	"[Ecrit "					/*"[Wrote "*/
#define	TEXT150	", sauvegarde sous le nom "			/*", saved as "*/
#define	TEXT151	"Nom: "						/*"Name: "*/
#define	TEXT152	"[Fichier inexistant]"				/*"[No such file]"*/
#define	TEXT153	"[Insertion du fichier en cours]"		/*"[Inserting file]"*/
#define	TEXT154	"Insere "					/*"Inserted "*/
#define	TEXT155	"Ouverture du fichier en ecriture impossible"	/*"Cannot open file for writing"*/
#define	TEXT156	"Erreur a la fermeture du fichier"		/*"Error closing file"*/
#define	TEXT157	"Erreur en ecriture E/S"			/*"Write I/O error"*/
#define	TEXT158	"Erreur en lecture du fichier"			/*"File read error"*/
#define	TEXT159	"%J'ai besoin d'un numero de touche de fonction"	/*"%Need function key number"*/
#define	TEXT160	"%Numero de touche de fonction hors limite"		/*"%Function key number out of range"*/
#define	TEXT161	"Contenu de l'etiquette: "			/*"Label contents: "*/
#define	TEXT162	" [o/n]? "					/*" [y/n]? "*/
#define	TEXT163	"pas de valeur par defaut"			/*"no default"*/
#define	TEXT164	"[echec de la recherche]"			/*"[search failed]"*/
#define	TEXT165	"Recherche Incrementale: "			/*"ISearch: "*/
#define	TEXT166	"? Chaine de recherche trop longue"		/*"? Search string too long"*/
#define	TEXT167	"? commande trop longue"			/*"? command too long"*/
#define	TEXT168	"%%Insertion de chaine impossible"		/*"%%Can not insert string"*/
#define	TEXT169	"Insere(e)"					/*"Inserted"*/
#define	TEXT170	"bug: linsert"					/* same */
#define	TEXT171	"Remplace(e)"					/*"Replaced"*/
#define	TEXT172	"%%Plus de memoire en mode remplacement"		/*"%%Out of memory while overwriting"*/
#define	TEXT173	"ERREUR VERROUILLAGE: Table de verrouillage pleine"	/*"LOCK ERROR: Lock table full"*/
#define	TEXT174	"Verrouillage impossible, plus de memoire"		/*"Cannot lock, out of memory"*/
#define	TEXT175	"VERROUILLE"						/*"LOCK"*/
#define	TEXT176	"Fichier utilise par "					/*"File in use by "*/
#define	TEXT177	", outrepasse?"						/*", overide?"*/
#define	TEXT178	"[Obtention d'un message d'erreur systeme impossible]"	/*"[can not get system error message]"*/
#define	TEXT179	"  A-propos de MicroEmacs"					/*"  About MicroEmacs"*/
#define	TEXT180	"%%Resolution inexistante"				/*"%%No such resolution"*/
#define	TEXT181	"%%Resolution interdite pour ce type d'ecran"		/*"%%Resolution illegal for this monitor"*/
#define	TEXT182	"Variable TERM non definie"				/*"Environment variable TERM not defined!"*/
#define	TEXT183	"%s: type de terminal inconnu"				/*"Unknown terminal type %s!"*/
#define	TEXT184	"Description termcap incomplete (lignes)"		/*"termcap entry incomplete (lines)"*/
#define	TEXT185	"Decsription termcap incomplete (colonnes)"		/*"Termcap entry incomplete (columns)"*/
#define	TEXT186	"Description termcap incomplete"			/*"Incomplete termcap entry\n"*/
#define	TEXT187	"Description du terminal trop importante"		/*"Terminal description too big!\n"*/
#define	TEXT188	"[Termine]"					/*"[End]"*/
#define	TEXT189	"Description du terminal non trouvee\n"		/*"Cannot find entry for terminal type.\n"*/

/*		"Check terminal type with \"SHOW TERMINAL\" or\n"*/
#define	TEXT190	"Verifiez le type de terminal avec \"SHOW TERMINAL\" ou\n"

/*		"try setting with \"SET TERMINAL/INQUIRE\"\n"*/
#define	TEXT191	"essayez de l'initialiser avec \"SET TERMINAL/INQUIRE\"\n"

/*		"The terminal type does not have enough power to run\n"*/
#define	TEXT192	"Ce terminal n'est pas assez performant pour \n"

/*		"MicroEMACS.  Try a different terminal or check\n"*/
#define	TEXT193	"MicroEMACS. Essayez un autre terminal ou verifiez\n"

/*		"type with \"SHOW TERMINAL\".\n"*/
#define	TEXT194	"celui-ci avec \"SHOW TERMINAL\".\n"

/*		"Cannot open channel to terminal.\n"*/
#define	TEXT195	"Ouverture du canal associe au terminal impossible.\n"

/*		"Cannot obtain terminal settings.\n"*/
#define	TEXT196	"Obtention des caracteristiques du terminal impossible.\n"

/*		"Cannot modify terminal settings.\n"*/
#define	TEXT197	"Modification des caracteristiques du terminal impossible.\n"

#define	TEXT198	"Erreur Entree/Sortie (%d,%d)\n"		/*"I/O error (%d,%d)\n"*/
#define	TEXT199	"[Lancement de DCL]\r\n"			/*"[Starting DCL]\r\n"*/
#define	TEXT200	"[Appel de DCL]\r\n"				/*"[Calling DCL]\r\n"*/
#define	TEXT201	"[Non encore disponible sous VMS]"		/*"[Not available yet under VMS]"*/
#define	TEXT202	"Terminal non 'vt52' ou 'z19'!"			/*"Terminal type not 'vt52'or 'z19' !"*/
#define	TEXT203	"Numero de fenetre hors limite"			/*"Window number out of range"*/
#define	TEXT204	"Destruction de cette fenetre impossible"	/*"Can not delete this window"*/
#define	TEXT205	"Decoupage impossible d'une fenetre de %d lignes"	/*"Cannot split a %d line window"*/
#define	TEXT206	"Une seule fenetre"				/*"Only one window"*/
#define	TEXT207	"Changement impossible"				/*"Impossible change"*/
#define	TEXT208	"[Fenetre inexistante]"				/*"[No such window exists]"*/
#define	TEXT209	"%%Taille d'ecran hors limite"			/*"%%Screen size out of range"*/
#define	TEXT210	"%%Largeur d'ecran hors limite"			/*"%%Screen width out of range"*/
#define	TEXT211	"Liste des Fonctions"				/*"Function list"*/
#define	TEXT212	"Affichage de la liste des fonctions impossible"/*"Can not display function list"*/
#define	TEXT213	"[Creation de la liste des fonctions]"		/*"[Building function list]"*/
#define	TEXT214 "%%le fichier %s inexistante"			/*"%%No such file as %s"*/
#define	TEXT215	": macro-une-touche "				/*": macro-to-key "*/
#define TEXT216 "Cannot read/write directories!!!"
#define TEXT217 "[Not available yet under AOS/VS]"
#define	TEXT218	"Append file: "
#define TEXT219 "%%Macro Failed"
#define	TEXT220	"Ligne %d/%d Col %d/%d Car %D/%D (%d%%) car = 0x%x%x"
#define TEXT221 "Too many groups"
#define TEXT222 "Group not ended"
#define	TEXT223 "%%Column origin out of range"
#define	TEXT224 "%%Row origin out of range"
#define	TEXT225 "[Switched to screen %s]"
#define	TEXT226	"%%Can not kill an executing buffer"
#define	TEXT227 "\n--- Press any key to Continue ---"
#define TEXT228 "[List du ratures vide']"
