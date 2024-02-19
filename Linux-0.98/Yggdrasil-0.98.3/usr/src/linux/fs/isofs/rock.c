/*
 *  linux/fs/isofs/rock.c
 *
 *  (C) 1992  Eric Youngdale
 *
 *  Rock Ridge Extensions to iso9660
 */
#include <linux/config.h>
#include <linux/stat.h>
#include <linux/sched.h>
#include <linux/iso_fs.h>
#include <linux/string.h>

#include "rock.h"

/* These functions are designed to read the system areas of a directory record
 * and extract relevant information.  There are different functions provided
 * depending upon what information we need at the time.  One function fills
 * out an inode structure, a second one extracts a filename, a third one
 * returns a symbolic link name, and a fourth one returns the extent number
 * for the file. */

#define SIG(A,B) ((A << 8) | B)


/* This is a way of ensuring that we have something in the system
   use fields that is compatible with Rock Ridge */
#define CHECK_SP(FAIL)	       			\
      if(rr->u.SP.magic[0] != 0xbe) FAIL;	\
      if(rr->u.SP.magic[1] != 0xef) FAIL;

/* We define a series of macros because each function must do exactly the
   same thing in certain places.  We use the macros to ensure that everyting
   is done correctly */

#define CONTINUE_DECLS \
  int cont_extent = 0, cont_offset = 0, cont_size = 0;   \
  char * buffer = 0

#define CHECK_CE	       			\
      {cont_extent = isonum_733(rr->u.CE.extent); \
      cont_offset = isonum_733(rr->u.CE.offset); \
      cont_size = isonum_733(rr->u.CE.size);}

#define SETUP_ROCK_RIDGE(DE,CHR,LEN)	      		      	\
  {LEN= sizeof(struct iso_directory_record) + DE->name_len[0];	\
  if(LEN & 1) LEN++;						\
  CHR = ((unsigned char *) DE) + LEN;				\
  LEN = *((unsigned char *) DE) - LEN;}

#define MAYBE_CONTINUE(LABEL,DEV) \
  {if (buffer) free(buffer); \
  if (cont_extent){ \
    int block, offset; \
    struct buffer_head * bh; \
    buffer = malloc(cont_size); \
    block = cont_extent << 1; \
    offset = cont_offset; \
    if (offset >= 1024) block++; \
    offset &= 1023; \
    bh = bread(DEV, block, 1024); \
    memcpy(buffer, bh->b_data, cont_size); \
    brelse(bh); \
    chr = buffer; \
    len = cont_size; \
    cont_extent = 0; \
    cont_size = 0; \
    cont_offset = 0; \
    goto LABEL; \
  }}


/* This is the inner layer of the get filename routine, and is called
   for each system area and continuation record related to the file */

int find_rock_ridge_relocation(struct iso_directory_record * de, 
			       struct inode * inode) {
  int flag;
  int len;
  int retval;
  unsigned char * chr;
  CONTINUE_DECLS;
  flag = 0;
  
  /* If this is a '..' then we are looking for the parent, otherwise we
     are looking for the child */
  
  if (de->name[0]==1 && de->name_len[0]==1) flag = 1;
  /* Return value if we do not find appropriate record. */
  retval = isonum_733 (de->extent);
  
  if (!inode->i_sb->u.isofs_sb.s_rock) return retval;

  SETUP_ROCK_RIDGE(de, chr, len);
 repeat:
  {
    int rrflag, sig;
    struct rock_ridge * rr;
    
    while (len > 1){ /* There may be one byte for padding somewhere */
      rr = (struct rock_ridge *) chr;
      if (rr->len == 0) goto out; /* Something got screwed up here */
      sig = (chr[0] << 8) + chr[1];
      chr += rr->len; 
      len -= rr->len;

      switch(sig){
      case SIG('R','R'):
	rrflag = rr->u.RR.flags[0];
	if (flag && !(rrflag & RR_PL)) goto out;
	if (!flag && !(rrflag & RR_CL)) goto out;
	break;
      case SIG('S','P'):
	CHECK_SP(goto out);
	break;
      case SIG('C','L'):
	printk("RR: CL\n");
	if (flag == 0) {
	  retval = isonum_733(rr->u.CL.location);
	  goto out;
	};
	break;
      case SIG('P','L'):
	printk("RR: PL\n");
	if (flag != 0) {
	  retval = isonum_733(rr->u.PL.location);
	  goto out;
	};
	break;
      case SIG('C','E'):
	CHECK_CE; /* This tells is if there is a continuation record */
	break;
      default:
	break;
      }
    };
  };
  MAYBE_CONTINUE(repeat, inode->i_dev);
  return retval;
 out:
  if(buffer) free(buffer);
  return retval;
}

int get_rock_ridge_filename(struct iso_directory_record * de,
			   char ** name, int * namlen, struct inode * inode)
{
  int len;
  unsigned char * chr;
  CONTINUE_DECLS;
  char * retname = NULL;
  int retnamlen = 0, truncate=0;
 
  if (!inode->i_sb->u.isofs_sb.s_rock) return 0;

  SETUP_ROCK_RIDGE(de, chr, len);
 repeat:
  {
    struct rock_ridge * rr;
    int sig;
    
    while (len > 1){ /* There may be one byte for padding somewhere */
      rr = (struct rock_ridge *) chr;
      if (rr->len == 0) goto out; /* Something got screwed up here */
      sig = (chr[0] << 8) + chr[1];
      chr += rr->len; 
      len -= rr->len;

      switch(sig){
      case SIG('R','R'):
	if((rr->u.RR.flags[0] & RR_NM) == 0) goto out;
	break;
      case SIG('S','P'):
	CHECK_SP(goto out);
	break;
      case SIG('C','E'):
	CHECK_CE;
	break;
      case SIG('N','M'):
	if (truncate) break;
	if (rr->u.NM.flags & ~1) {
	  printk("Unsupported NM flag settings (%d)\n",rr->u.NM.flags);
	  break;
	};
	if (!retname){
	  retname = (char *) malloc (255); /* This may be a waste, but we only
					    need this for a moment.  The layers
					    that call this function should
					    deallocate the mem fairly soon
					    after control is returned */
	  *retname = 0; /* Zero length string */
	  retnamlen = 0;
	};
	if((strlen(retname) + rr->len - 5) >= 254) {
	  truncate = 1;
	  break;
	};
	strncat(retname, rr->u.NM.name, rr->len - 5);
	retnamlen += rr->len - 5;
	break;
      case SIG('R','E'):
	printk("RR: RE (%x)\n", inode->i_ino);
	if (buffer) free(buffer);
	if (retname) free(retname);
	return -1;
      default:
	break;
      }
    };
  }
  MAYBE_CONTINUE(repeat,inode->i_dev);
  if(retname){
    *name = retname;
    *namlen = retnamlen;
    return 1;
  };
  return 0;  /* This file did not have a NM field */
 out:
  if(buffer) free(buffer);
  if (retname) free(retname);
  return 0;
}

int parse_rock_ridge_inode(struct iso_directory_record * de,
			   struct inode * inode){
  int len;
  unsigned char * chr;
  CONTINUE_DECLS;

  if (!inode->i_sb->u.isofs_sb.s_rock) return 0;

  SETUP_ROCK_RIDGE(de, chr, len);
 repeat:
  {
    int cnt, sig;
    struct inode * reloc;
    struct rock_ridge * rr;
    
    while (len > 1){ /* There may be one byte for padding somewhere */
      rr = (struct rock_ridge *) chr;
      if (rr->len == 0) goto out; /* Something got screwed up here */
      sig = (chr[0] << 8) + chr[1];
      chr += rr->len; 
      len -= rr->len;
      
      switch(sig){
      case SIG('R','R'):
	if((rr->u.RR.flags[0] & 
 	    (RR_PX | RR_TF | RR_SL | RR_CL)) == 0) goto out;
	break;
      case SIG('S','P'):
	CHECK_SP(goto out);
	break;
      case SIG('C','E'):
	CHECK_CE;
	break;
      case SIG('E','R'):
	printk("ISO9660 Extensions: ");
	{ int p;
	  for(p=0;p<rr->u.ER.len_id;p++) printk("%c",rr->u.ER.data[p]);
	};
	  printk("\n");
	break;
      case SIG('P','X'):
	inode->i_mode  = isonum_733(rr->u.PX.mode);
	inode->i_nlink = isonum_733(rr->u.PX.n_links);
	inode->i_uid   = isonum_733(rr->u.PX.uid);
	inode->i_gid   = isonum_733(rr->u.PX.gid);
	break;
      case SIG('P','N'):
	{ int high, low;
	  high = isonum_733(rr->u.PN.dev_high);
	  low = isonum_733(rr->u.PN.dev_low);
	  inode->i_rdev = ((high << 8) | (low && 0xff)) & 0xffff;
	};
	break;
      case SIG('T','F'):
	cnt = 0; /* Rock ridge never appears on a High Sierra disk */
	if(rr->u.TF.flags & TF_CREATE) inode->i_ctime = iso_date(rr->u.TF.times[cnt++].time, 0);
	if(rr->u.TF.flags & TF_MODIFY) inode->i_mtime = iso_date(rr->u.TF.times[cnt++].time, 0);
	if(rr->u.TF.flags & TF_ACCESS) inode->i_atime = iso_date(rr->u.TF.times[cnt++].time, 0);
	break;
      case SIG('S','L'):
	{int slen;
	 struct SL_component * slp;
	 slen = rr->len - 5;
	 slp = &rr->u.SL.link;
	 while (slen > 1){
	   switch(slp->flags &~1){
	   case 0:
	     inode->i_size += slp->len;
	     break;
	   case 2:
	     inode->i_size += 1;
	     break;
	   case 4:
	     inode->i_size += 2;
	     break;
	   case 8:
	     inode->i_size += 1;
	     break;
	   default:
	     printk("Symlink component flag not implemented\n");
	   };
	   slen -= slp->len + 2;
	   slp = (struct SL_component *) (((char *) slp) + slp->len + 2);

	   if(slen < 2) break;
	   inode->i_size += 1;
	 };
       };
	break;
      case SIG('R','E'):
	printk("Attempt to read inode for relocated directory\n");
	goto out;
      case SIG('C','L'):
	printk("RR CL (%x)\n",inode->i_ino);
	inode->u.isofs_i.i_first_extent = isonum_733(rr->u.CL.location);
	reloc = iget(inode->i_sb, inode->u.isofs_i.i_first_extent << ISOFS_BLOCK_BITS);
	inode->i_mode = reloc->i_mode;
	inode->i_nlink = reloc->i_nlink;
	inode->i_uid = reloc->i_uid;
	inode->i_gid = reloc->i_gid;
	inode->i_rdev = reloc->i_rdev;
	inode->i_size = reloc->i_size;
	inode->i_atime = reloc->i_atime;
	inode->i_ctime = reloc->i_ctime;
	inode->i_mtime = reloc->i_mtime;
	iput(reloc);
	break;
      default:
	break;
      }
    };
  }
  MAYBE_CONTINUE(repeat,inode->i_dev);
  return 0;
 out:
  if(buffer) free(buffer);
  return 0;
}


/* Returns the name of the file that this inode is symlinked to.  This is
   in malloc'd memory, so it needs to be freed, once we are through with it */

char * get_rock_ridge_symlink(struct inode * inode)
{
  struct buffer_head * bh;
  unsigned char * pnt, *cpnt = 0;
  char * rpnt;
  struct iso_directory_record * raw_inode;
  CONTINUE_DECLS;
  int block;
  int sig;
  int len;
  char * chr;
  struct rock_ridge * rr;
  
  if (!inode->i_sb->u.isofs_sb.s_rock)
    panic("Cannot have symlink with high sierra variant of iso filesystem\n");

  rpnt = 0;
  
  block = inode->i_ino >> ISOFS_BUFFER_BITS;
  if (!(bh=bread(inode->i_dev,block, ISOFS_BUFFER_SIZE)))
    panic("unable to read i-node block");
  
  pnt = ((char *) bh->b_data) + (inode->i_ino & (ISOFS_BUFFER_SIZE - 1));
  
  raw_inode = ((struct iso_directory_record *) pnt);
  
  if ((inode->i_ino & (ISOFS_BUFFER_SIZE - 1)) + *pnt > ISOFS_BUFFER_SIZE){
    cpnt = malloc(1 << ISOFS_BLOCK_BITS);
    memcpy(cpnt, bh->b_data, ISOFS_BUFFER_SIZE);
    brelse(bh);
    if (!(bh = bread(inode->i_dev,++block, ISOFS_BUFFER_SIZE)))
      panic("unable to read i-node block");
    memcpy(cpnt+ISOFS_BUFFER_SIZE, bh->b_data, ISOFS_BUFFER_SIZE);
    pnt = ((char *) cpnt) + (inode->i_ino & (ISOFS_BUFFER_SIZE - 1));
    raw_inode = ((struct iso_directory_record *) pnt);
  };
  
  /* Now test for possible Rock Ridge extensions which will override some of
     these numbers in the inode structure. */
  
  SETUP_ROCK_RIDGE(raw_inode, chr, len);
  
 repeat:
  while (len > 1){ /* There may be one byte for padding somewhere */
    if (rpnt) break;
    rr = (struct rock_ridge *) chr;
    if (rr->len == 0) goto out; /* Something got screwed up here */
    sig = (chr[0] << 8) + chr[1];
    chr += rr->len; 
    len -= rr->len;
    
    switch(sig){
    case SIG('R','R'):
      if((rr->u.RR.flags[0] & RR_SL) == 0) goto out;
      break;
    case SIG('S','P'):
      CHECK_SP(goto out);
      break;
    case SIG('S','L'):
      {int slen;
       struct SL_component * slp;
       slen = rr->len - 5;
       slp = &rr->u.SL.link;
       while (slen > 1){
	 if (!rpnt){
	   rpnt = (char *) malloc (inode->i_size +1);
	   *rpnt = 0;
	 };
	 switch(slp->flags &~1){
	 case 0:
	   strncat(rpnt,slp->text, slp->len);
	   break;
	 case 2:
	   strcat(rpnt,".");
	   break;
	 case 4:
	   strcat(rpnt,"..");
	   break;
	 case 8:
	   strcpy(rpnt,"/");
	   break;
	 default:
	   printk("Symlink component flag not implemented (%d)\n",slen);
	 };
	 slen -= slp->len + 2;
	 slp = (struct SL_component *) (((char *) slp) + slp->len + 2);

	 if(slen < 2) break;
	 strcat(rpnt,"/");
       };
       break;
     default:
       break;
     }
    };
  };
  MAYBE_CONTINUE(repeat,inode->i_dev);
  brelse(bh);
  
  if (cpnt) {
    free(cpnt);
    cpnt = 0;
  };
  return rpnt;
 out:
  if(buffer) free(buffer);
  return 0;
}






