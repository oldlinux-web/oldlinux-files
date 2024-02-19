/*----------------------------------------------------------------------*/
/*			LHarc Add Command				*/
/*		This is part of LHarc UNIX Archiver Driver		*/
/*									*/
/*		Copyright(C) MCMLXXXIX  Yooichi.Tagawa			*/
/*									*/
/*  V0.00  Original				1988.05.23  Y.Tagawa	*/
/*  V1.00  Fixed				1989.09.22  Y.Tagawa	*/
/*  V1.02  Bug fix				1990.01.19  Y.Tagawa	*/
/*  V0.03  LHa for UNIX				1991.12.05  M.Oki	*/
/*----------------------------------------------------------------------*/

#include "lharc.h"

extern int encode_lzhuf ();
extern int encode_stored_crc ();

static char new_archive_name_buffer [ FILENAME_LENGTH ];
static char *new_archive_name;
FILE *temporary_fp = NULL;

/*----------------------------------------------------------------------*/
/*									*/
/*----------------------------------------------------------------------*/




static void
add_one (fp, nafp, hdr)
     FILE *fp, *nafp;
     LzHeader *hdr;
{
  long header_pos, next_pos, org_pos, data_pos;
  long v_original_size, v_packed_size;

  reading_filename = hdr->name;
  writting_filename = temporary_name;

  if (!fp && generic_format)	/* [generic] doesn't need directory info. */
    return;
  header_pos = ftell (nafp);
  write_header (nafp, hdr);	/* DUMMY */

  if (hdr->original_size == 0)	/* empty file or directory */
      return;			/* previous write_header is not DUMMY. (^_^) */

  org_pos = ftell (fp);
  data_pos = ftell (nafp);

  hdr->crc = encode_lzhuf (fp, nafp, hdr->original_size,
			   &v_original_size, &v_packed_size, hdr->name, hdr->method);

  if (v_packed_size < v_original_size)
    {
      next_pos = ftell (nafp);
    }
  else
    {				/* retry by stored method */
      fseek (fp, org_pos, SEEK_SET);
      fseek (nafp, data_pos, SEEK_SET);
      hdr->crc = encode_stored_crc (fp, nafp, hdr->original_size,
				    &v_original_size, &v_packed_size);
      fflush (nafp);
      next_pos = ftell (nafp);
#ifndef NOFTRUNCATE
      ftruncate (fileno (nafp), next_pos);
#endif
      bcopy (LZHUFF0_METHOD, hdr->method, METHOD_TYPE_STRAGE);
    }
  hdr->original_size = v_original_size;
  hdr->packed_size = v_packed_size;
  fseek (nafp, header_pos, SEEK_SET);
  write_header (nafp, hdr);
  fseek (nafp, next_pos, SEEK_SET);
}


FILE *
append_it (name, oafp, nafp)
     char *name;
     FILE *oafp, *nafp;
{
  LzHeader ahdr, hdr;
  FILE *fp;
  long old_header;
  int cmp;
  int filec;
  char **filev;
  int i;
  struct stat stbuf;
  boolean directory;

  if (stat (name, &stbuf) < 0)
    {
      error ("cannot access", name); /* See cleaning_files, Why? */
      return oafp;
    }

  directory = is_directory (&stbuf);

  init_header (name, &stbuf, &hdr);

  if (!directory && !noexec)
    fp = xfopen (name, READ_BINARY);
  else
    fp = NULL;

  while (oafp)
    {
      old_header = ftell (oafp);
      if (!get_header (oafp, &ahdr))
	{
	  fclose (oafp);
	  oafp = NULL;
	  break;
	}
      else
	{
	  cmp = STRING_COMPARE (ahdr.name, hdr.name);
	  if (cmp < 0)
	    {			/* SKIP */
	      /* copy old to new */
	      if (!noexec)
		{
		  fseek (oafp, old_header, SEEK_SET);
		  copy_old_one (oafp, nafp, &ahdr);
		}
	      else
		fseek (oafp, ahdr.packed_size, SEEK_CUR);
	    }
	  else if (cmp == 0)
	    {			/* REPLACE */
	      /* drop old archive's */
	      fseek (oafp, ahdr.packed_size, SEEK_CUR);
	      break;
	    }
	  else			/* cmp > 0, INSERT */
	    {
	      fseek (oafp, old_header, SEEK_SET);
	      break;
	    }
	}
    }

  if (update_if_newer)
    {
      if (!oafp ||		/* not in archive */
	  cmp > 0 ||		/* // */
	  ahdr.unix_last_modified_stamp < /* newer than archive's */
	  hdr.unix_last_modified_stamp)
	{
	  if (noexec)
	    printf ("ADD %s\n", name);
	  else
	    add_one (fp, nafp, &hdr);
	}
      else /* cmp == 0 */
	{			/* copy old to new */
	  if (!noexec)
	    {
	      fseek (oafp, old_header, SEEK_SET);
	      copy_old_one (oafp, nafp, &ahdr);
	    }
	}
    }
  else
    {
      if (!oafp || cmp > 0)	/* not in archive or dropped */
	{
	  if (noexec)
	    printf ("ADD %s\n", name);
	  else
	    add_one (fp, nafp, &hdr);
	}
      else /* cmp == 0 */	/* replace */
	{
	  if (noexec)
	    printf ("REPLACE\n");
	  else
	    add_one (fp, nafp, &hdr);
	}
    }

  if (!directory)
    {
      if (!noexec)
	fclose (fp);
    }
  else
    {			/* recurcive call */
      if (find_files (name, &filec, &filev))
	{
	  for (i = 0; i < filec; i ++)
	    oafp = append_it (filev[i], oafp, nafp);
	  free_files (filec, filev);
	}
    }
  return oafp;
}

static void
find_update_files (oafp)
     FILE *oafp;		/* old archive */
{
  char name[FILENAME_LENGTH];
  struct string_pool sp;
  LzHeader hdr;
  long pos;
  struct stat stbuf;
  int len;

  pos = ftell (oafp);

  init_sp (&sp);
  while (get_header (oafp, &hdr))
    {
      if ((hdr.unix_mode & UNIX_FILE_TYPEMASK) == UNIX_FILE_REGULAR)
	{
	  if (stat (hdr.name, &stbuf) >= 0) /* exist ? */
	    add_sp (&sp, hdr.name, strlen (hdr.name) + 1);
	}
      else if ((hdr.unix_mode & UNIX_FILE_TYPEMASK) == UNIX_FILE_DIRECTORY)
	{
	  strcpy (name, hdr.name);
	  len = strlen (name);
	  if (len > 0 && name[len - 1] == '/')
	    name[--len] = '\0'; /* strip tail '/' */
	  if (stat (name, &stbuf) >= 0) /* exist ? */
	    add_sp (&sp, name, len+1);
	}
      fseek (oafp, hdr.packed_size, SEEK_CUR);
    }

  fseek (oafp, pos, SEEK_SET);

  finish_sp (&sp, &cmd_filec, &cmd_filev);
}

static void
delete (oafp, nafp)
     FILE *oafp, *nafp;
{
  LzHeader ahdr;
  long old_header_pos;

  old_header_pos = ftell (oafp);
  while (get_header (oafp, &ahdr))
    {
      if (need_file (ahdr.name))
	{			/* skip */
	  fseek (oafp, ahdr.packed_size, SEEK_CUR);
	  if (noexec)
	    printf ("DELETE %s\n", ahdr.name);
	  else if (verbose)
	    printf ("Delete %s\n", ahdr.name);
	}
      else
	{			/* copy */
	  if (noexec)
	    {
	      fseek (oafp, ahdr.packed_size, SEEK_CUR);
	    }
	  else
	    {
	      fseek (oafp, old_header_pos, SEEK_SET);
	      copy_old_one (oafp, nafp, &ahdr);
	    }
	}
      old_header_pos = ftell (oafp);
    }
  return;
}


/*----------------------------------------------------------------------*/
/*									*/
/*----------------------------------------------------------------------*/
static FILE *
build_temporary_file ()
{
  int old_umask;
  FILE *afp;

  build_temporary_name ();
  signal (SIGINT, interrupt);
  signal (SIGHUP, interrupt);

  old_umask = umask (077);
  afp = xfopen (temporary_name, WRITE_BINARY);
  remove_temporary_at_error = TRUE;
  temporary_fp = afp;
  umask (old_umask);

  return afp;
}

static void
build_backup_file ()
{
  
  build_backup_name (backup_archive_name, archive_name);
  if (!noexec)
    {
      signal (SIGINT, SIG_IGN);
      signal (SIGHUP, SIG_IGN);
      if (rename (archive_name, backup_archive_name) < 0)
		fatal_error (archive_name);
      recover_archive_when_interrupt = TRUE;
      signal (SIGINT, interrupt);
      signal (SIGHUP, interrupt);
    }
}

static void
report_archive_name_if_different ()
{
  if (!quiet && new_archive_name == new_archive_name_buffer)
    {
      /* warning at old archive is SFX */
      printf ("New archive file is \"%s\"\n", new_archive_name);
    }
}

#ifdef TMP_FILENAME_TEMPLATE
void
temporary_to_new_archive_file (new_archive_size)
     long new_archive_size;
{
  FILE *oafp, *nafp;

  oafp = xfopen (temporary_name, READ_BINARY);
  if (!strcmp(new_archive_name, "-"))
    {
      nafp = stdout;
      writting_filename = "starndard output";
    }
  else
    {
      nafp = xfopen (new_archive_name, WRITE_BINARY);
      writting_filename = archive_name;
    }
  reading_filename = temporary_name;
  copyfile (oafp, nafp, new_archive_size, 0);
  if (nafp != stdout) fclose (nafp);
  fclose (oafp);

  recover_archive_when_interrupt = FALSE;
  unlink (temporary_name);

  remove_temporary_at_error = FALSE;
}
#else
temporary_to_new_archive_file (new_archive_size)
long new_archive_size;
{
  char *p;
  p = (char *)rindex(new_archive_name,'/');
  p = p ? p+1 : new_archive_name;
  unlink ( new_archive_name );
  if ( rename ( temporary_name , p )<0 ) {
    fprintf(stderr, "Can't rename temporary_name '%s'\n", new_archive_name);
    exit(1);
  }
}
#endif

static void
set_archive_file_mode ()
{
  int umask_value;
  struct stat stbuf;

  if (archive_file_gid < 0)
    {
      umask (umask_value = umask (0));
      archive_file_mode = (~umask_value) & 0666; /* rw-rw-rw- */
      if (stat (".", &stbuf) >= 0)
	archive_file_gid = stbuf.st_gid;
    }
  if (archive_file_gid >= 0)
    chown (new_archive_name, getuid (), archive_file_gid);

  chmod (new_archive_name, archive_file_mode);
}


/*----------------------------------------------------------------------*/
/*		REMOVE FILE/DIRECTORY					*/
/*----------------------------------------------------------------------*/

static void remove_files ();

static void
remove_one (name)
     char *name;
{
  struct stat stbuf;
  int filec;
  char **filev;

  if (stat (name, &stbuf) < 0)
    {
      warning ("Cannot access", name);
    }
  else if (is_directory (&stbuf))
    {
      if (find_files (name, &filec, &filev))
	{
	  remove_files (filec, filev);
	  free_files (filec, filev);
	}
      else
	warning ("Cannot open directory", name);

      if (noexec)
	printf ("REMOVE DIRECTORY %s\n", name);
      else if (rmdir (name) < 0)
	warning ("Cannot remove directory", name);
      else if (verbose)
	printf ("Removed %s.\n", name);
    }
  else if (is_regularfile (&stbuf))
    {
      if (noexec)
	printf ("REMOVE FILE %s.\n", name);
      else if (unlink (name) < 0)
	warning ("Cannot remove", name);
      else if (verbose)
	printf ("Removed %s.\n", name);
    }
  else
    {
      error ("Cannot remove (not a file or directory)", name);
    }
}

static void
remove_files (filec, filev)
     int filec;
     char **filev;
{
  int i;

  for (i = 0; i < filec; i++)
    remove_one (filev[i]);
}


/*----------------------------------------------------------------------*/
/*									*/
/*----------------------------------------------------------------------*/

void
cmd_add ()
{
  LzHeader ahdr;
  FILE *oafp, *nafp;
  int i;
  long old_header;
  boolean old_archive_exist;
  long new_archive_size;

  /* exit if no operation */
  if (!update_if_newer && cmd_filec == 0)
    {
      error ("No files given in argument, do nothing.", "");
      return;
    }

  /* open old archive if exist */
  if ((oafp = open_old_archive ()) == NULL)
    old_archive_exist = FALSE;
  else
    old_archive_exist = TRUE;

  if (update_if_newer && cmd_filec == 0 && !oafp)
    fatal_error (archive_name); /* exit if cannot execute automatic update */
  errno = 0;

  if (new_archive && old_archive_exist)
    {
      fclose (oafp);
      oafp = NULL;
    }

  if (oafp)
    {
      if (archive_is_msdos_sfx1 (archive_name))
        {
          skip_msdos_sfx1_code (oafp);
          build_standard_archive_name (new_archive_name_buffer, archive_name);
          new_archive_name = new_archive_name_buffer;
        }
      else
        {
          new_archive_name = archive_name;
        }
    }
  else
    {
      if (!old_archive_exist)
        {
          build_standard_archive_name (new_archive_name_buffer, archive_name);
          new_archive_name = new_archive_name_buffer;
        }
      else
        {
          new_archive_name = archive_name;
        }
    }

  /* build temporary file */
  if (!noexec)
    nafp = build_temporary_file ();

  /* find needed files when automatic update */
  if (update_if_newer && cmd_filec == 0)
    find_update_files (oafp);

  /* build new archive file */
  /* cleaning arguments */
  cleaning_files (&cmd_filec, &cmd_filev);
  if (cmd_filec == 0)
    {
      if (oafp)
	fclose (oafp);
      if  (!noexec)
	fclose (nafp);
      return;
    }

  for (i = 0; i < cmd_filec; i ++)
    oafp = append_it (cmd_filev[i], oafp, nafp);
  if (oafp)
    {
      old_header = ftell (oafp);
      while (get_header (oafp, &ahdr))
	{
	  if (noexec)
	    fseek (oafp, ahdr.packed_size, SEEK_CUR);
	  else
	    {
	      fseek (oafp, old_header, SEEK_SET);
	      copy_old_one (oafp, nafp, &ahdr);
	    }
	  old_header = ftell (oafp);
	}
      fclose (oafp);
    }
  if (!noexec)
    {
      write_archive_tail (nafp);
      new_archive_size = ftell (nafp);
      fclose (nafp);
    }

  /* build backup archive file */
  if (old_archive_exist)
    build_backup_file ();

  report_archive_name_if_different ();

  /* copy temporary file to new archive file */
  if (!noexec && (!strcmp(new_archive_name, "-") ||
		  rename (temporary_name, new_archive_name) < 0))
    temporary_to_new_archive_file (new_archive_size);

  /* set new archive file mode/group */
  set_archive_file_mode ();

  /* remove archived files */
  if (delete_after_append)
    remove_files (cmd_filec, cmd_filev);

  return;
}


void
cmd_delete ()
{
  FILE *oafp, *nafp;
  long new_archive_size;

  /* open old archive if exist */
  if ((oafp = open_old_archive ()) == NULL)
    fatal_error (archive_name);
  errno = 0;

  /* exit if no operation */
  if (cmd_filec == 0)
    {
      fclose (oafp);
      warning ("No files given in argument, do nothing.", "");
      return;
    }

  if (archive_is_msdos_sfx1 (archive_name))
    {
      skip_msdos_sfx1_code (oafp);
      build_standard_archive_name (new_archive_name_buffer, archive_name);
      new_archive_name = new_archive_name_buffer;
    }
  else
    {
      new_archive_name = archive_name;
    }

  /* build temporary file */
  if (!noexec)
    nafp = build_temporary_file ();

  /* build new archive file */
  delete (oafp, nafp);
  fclose (oafp);

  if (!noexec)
    {
      write_archive_tail (nafp);
      new_archive_size = ftell (nafp);
      fclose (nafp);
    }

  /* build backup archive file */
  build_backup_file ();

  report_archive_name_if_different ();

  /* copy temporary file to new archive file */
  if (!noexec && rename (temporary_name, new_archive_name) < 0)
    temporary_to_new_archive_file (new_archive_size);

  /* set new archive file mode/group */
  set_archive_file_mode ();

  return;
}
