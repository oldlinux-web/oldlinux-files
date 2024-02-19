/*
 *  linux/fs/isofs/inode.c
 * 
 *  (C) 1992  Eric Youngdale Modified for ISO9660 filesystem.
 *
 *  (C) 1991  Linus Torvalds - minix filesystem
 */
#include <linux/config.h>
#include <linux/stat.h>
#include <linux/sched.h>
#include <linux/iso_fs.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/string.h>

#include <asm/system.h>
#include <asm/segment.h>
#include <linux/errno.h>

#ifndef CONFIG_BLK_DEV_SR
#error The iso9660 filesystem can only be used with CDROM.
#endif

extern int check_cdrom_media_change(int, int);

#ifdef LEAK_CHECK
static int check_malloc = 0;
static int check_bread = 0;
#endif

void isofs_put_super(struct super_block *sb)
{
	lock_super(sb);

#ifdef LEAK_CHECK
	printk("Outstanding mallocs:%d, outstanding buffers: %d\n", 
	       check_malloc, check_bread);
#endif
	sb->s_dev = 0;
	free_super(sb);
	return;
}

static struct super_operations isofs_sops = { 
	isofs_read_inode,
	NULL,
	NULL,
	isofs_put_super,
	NULL,
	isofs_statfs
};



static int parse_options(char *options,char *map,char *conversion, char * rock)
{
	char *this,*value;

	*map = 'n';
	*rock = 'y';
	*conversion = 'a';
	if (!options) return 1;
	for (this = strtok(options,","); this; this = strtok(NULL,",")) {
	        if (strncmp(this,"norock",6) == 0) {
		  *rock = 'n';
		  continue;
		};
		if (value = strchr(this,'=')) *value++ = 0;
		if (!strcmp(this,"map") && value) {
			if (value[0] && !value[1] && strchr("on",*value))
				*map = *value;
			else if (!strcmp(value,"off")) *map = 'o';
			else if (!strcmp(value,"normal")) *map = 'n';
			else return 0;
		}
		else if (!strcmp(this,"conv") && value) {
			if (value[0] && !value[1] && strchr("bta",*value))
				*conversion = *value;
			else if (!strcmp(value,"binary")) *conversion = 'b';
			else if (!strcmp(value,"text")) *conversion = 't';
			else if (!strcmp(value,"mtext")) *conversion = 'm';
			else if (!strcmp(value,"auto")) *conversion = 'a';
			else return 0;
		}
		else return 0;
	}
	return 1;
}

struct super_block *isofs_read_super(struct super_block *s,void *data)
{
	struct buffer_head *bh;
	int iso_blknum;
	int high_sierra;
	int dev=s->s_dev;
	struct iso_volume_descriptor *vdp;
	struct hs_volume_descriptor *hdp;

	struct iso_primary_descriptor *pri = NULL;
	struct hs_primary_descriptor *h_pri = NULL;

	struct iso_directory_record *rootp;

	char map, conversion, rock;

	if (!parse_options((char *) data,&map,&conversion, &rock)) {
		s->s_dev = 0;
		return NULL;
	}

	lock_super(s);

	s->u.isofs_sb.s_high_sierra = high_sierra = 0; /* default is iso9660 */

	for (iso_blknum = 16; iso_blknum < 100; iso_blknum++) {
		if (!(bh = bread(dev, iso_blknum << (ISOFS_BLOCK_BITS-ISOFS_BUFFER_BITS), ISOFS_BUFFER_SIZE))) {
			s->s_dev=0;
			printk("bread failed\n");
			free_super(s);
			return NULL;
		}

		vdp = (struct iso_volume_descriptor *)bh->b_data;
		hdp = (struct hs_volume_descriptor *)bh->b_data;

		
		if (strncmp (hdp->id, HS_STANDARD_ID, sizeof hdp->id) == 0) {
		  if (isonum_711 (hdp->type) != ISO_VD_PRIMARY)
			goto out;
		  if (isonum_711 (hdp->type) == ISO_VD_END)
		        goto out;
		
		        s->u.isofs_sb.s_high_sierra = 1;
			high_sierra = 1;
		        rock = 'n';
		        h_pri = (struct hs_primary_descriptor *)vdp;
			break;
		};
		
		if (strncmp (vdp->id, ISO_STANDARD_ID, sizeof vdp->id) == 0) {
		  if (isonum_711 (vdp->type) != ISO_VD_PRIMARY)
			goto out;
		  if (isonum_711 (vdp->type) == ISO_VD_END)
			goto out;
		
		        pri = (struct iso_primary_descriptor *)vdp;
			break;
	        };

		brelse(bh);
	      }
	if(iso_blknum == 100) {
		printk("Unable to identify CD-ROM format.\n");
		s->s_dev = 0;
		free_super(s);
		return NULL;
	};
	
	
	if(high_sierra){
	  rootp = (struct iso_directory_record *) h_pri->root_directory_record;
	  if (isonum_723 (h_pri->volume_set_size) != 1) {
	    printk("Multi-volume disks not (yet) supported.\n");
	    goto out;
	  };
	  s->u.isofs_sb.s_nzones = isonum_733 (h_pri->volume_space_size);
	  s->u.isofs_sb.s_log_zone_size = isonum_723 (h_pri->logical_block_size);
	  s->u.isofs_sb.s_max_size = isonum_733(h_pri->volume_space_size);
	} else {
	  rootp = (struct iso_directory_record *) pri->root_directory_record;
	  if (isonum_723 (pri->volume_set_size) != 1) {
	    printk("Multi-volume disks not (yet) supported.\n");
	    goto out;
	  };
	  s->u.isofs_sb.s_nzones = isonum_733 (pri->volume_space_size);
	  s->u.isofs_sb.s_log_zone_size = isonum_723 (pri->logical_block_size);
	  s->u.isofs_sb.s_max_size = isonum_733(pri->volume_space_size);
	}
	
	s->u.isofs_sb.s_ninodes = 0; /* No way to figure this out easily */
	
	s->u.isofs_sb.s_firstdatazone = isonum_733( rootp->extent) << 
		(ISOFS_BLOCK_BITS - ISOFS_BUFFER_BITS);
	s->s_magic = ISOFS_SUPER_MAGIC;
	
	/* The CDROM is read-only, has no nodes (devices) on it, and since
	   all of the files appear to be owned by root, we really do not want
	   to allow suid.  (suid or devices will not show up unless we have
	   Rock Ridge extensions) */
	
	s->s_flags = MS_RDONLY /* | MS_NODEV | MS_NOSUID */;
	
	if(s->u.isofs_sb.s_log_zone_size != (1 << ISOFS_BLOCK_BITS)) {
		printk("1 <<Block bits != Block size\n");
		goto out;
	};
	
	brelse(bh);
	
	printk("Max size:%d   Log zone size:%d\n",s->u.isofs_sb.s_max_size, 
	       s->u.isofs_sb.s_log_zone_size);
	printk("First datazone:%d   Root inode number %d\n",s->u.isofs_sb.s_firstdatazone,
	       isonum_733 (rootp->extent) << ISOFS_BLOCK_BITS);
	if(high_sierra) printk("Disc in High Sierra format.\n");
	free_super(s);
	/* set up enough so that it can read an inode */
	
	s->s_dev = dev;
	s->s_op = &isofs_sops;
	s->u.isofs_sb.s_mapping = map;
	s->u.isofs_sb.s_rock = (rock == 'y' ? 1 : 0);
	s->u.isofs_sb.s_conversion = conversion;

	if (!(s->s_mounted = iget(s,isonum_733 (rootp->extent) << 
				  ISOFS_BLOCK_BITS))) {
		s->s_dev=0;
		printk("get root inode failed\n");
		return NULL;
	}
	if(MAJOR(s->s_dev) == 11) {
		/* Chech this one more time. */
		if(check_cdrom_media_change(s->s_dev, 0))
		  goto out;
	};
	s->s_blocksize = ISOFS_BUFFER_SIZE;
	return s;
 out: /* Kick out for various error conditions */
	brelse(bh);
	s->s_dev = 0;
	free_super(s);
	return NULL;
}

void isofs_statfs (struct super_block *sb, struct statfs *buf)
{
	put_fs_long(ISOFS_SUPER_MAGIC, &buf->f_type);
	put_fs_long(1 << ISOFS_BLOCK_BITS, &buf->f_bsize);
	put_fs_long(sb->u.isofs_sb.s_nzones, &buf->f_blocks);
	put_fs_long(0, &buf->f_bfree);
	put_fs_long(0, &buf->f_bavail);
	put_fs_long(sb->u.isofs_sb.s_ninodes, &buf->f_files);
	put_fs_long(0, &buf->f_ffree);
	/* Don't know what value to put in buf->f_fsid */
}

int isofs_bmap(struct inode * inode,int block)
{

	if (block<0) {
		printk("_isofs_bmap: block<0");
		return 0;
	}
	return inode->u.isofs_i.i_first_extent + block;
}

void isofs_read_inode(struct inode * inode)
{
	struct buffer_head * bh;
	unsigned char * pnt, *cpnt = 0;
	struct iso_directory_record * raw_inode;
	int high_sierra;
	int block;
	int i;
	
	block = inode->i_ino >> ISOFS_BUFFER_BITS;
	if (!(bh=bread(inode->i_dev,block, ISOFS_BUFFER_SIZE)))
		panic("unable to read i-node block");
	
	pnt = ((char *) bh->b_data) + (inode->i_ino & (ISOFS_BUFFER_SIZE - 1));
	
	raw_inode = ((struct iso_directory_record *) pnt);
	high_sierra = inode->i_sb->u.isofs_sb.s_high_sierra;

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
	

	inode->i_mode = 0444; /* Everybody gets to read the file. */
	inode->i_nlink = 1;
	
	if (raw_inode->flags[-high_sierra] & 2) {
		inode->i_mode = 0555 | S_IFDIR;
		inode->i_nlink = 2; /* There are always at least 2.  It is
				       hard to figure out what is correct*/
	} else {
		inode->i_mode = 0444; /* Everybody gets to read the file. */
		inode->i_nlink = 1;
	        inode->i_mode |= S_IFREG;
/* If there are no periods in the name, then set the execute permission bit */
		for(i=0; i< raw_inode->name_len[0]; i++)
			if(raw_inode->name[i]=='.' || raw_inode->name[i]==';')
				break;
		if(i == raw_inode->name_len[0] || raw_inode->name[i] == ';') 
			inode->i_mode |= 0111; /* execute permission */
	};
	inode->i_uid = 0;
	inode->i_gid = 0;
	inode->i_size = isonum_733 (raw_inode->size);

/* Some dipshit decided to store some other bit of information in the high
   byte of the file length.  Catch this and holler. */

	if(inode->i_size < 0){
	  printk("Illegal format on cdrom.  Pester manufacturer.\n");
	  inode->i_size &= 0x00ffffff;
	};
	
	if (isonum_723 (raw_inode->volume_sequence_number) != 1) {
		panic("Multi volume CD somehow got mounted.\n");
	};

	if (raw_inode->interleave[0]) {
		printk("Interleaved files not (yet) supported.\n");
		inode->i_size = 0;
	};

#ifdef DEBUG
	/* I have no idea what extended attributes are used for, so
	   we will flag it for now */
	if(raw_inode->ext_attr_lengOAth[0] != 0){
		printk("Extended attributes present for ISO file (%d).\n",
		       inode->i_ino);
	}
#endif
	
	/* I have no idea what file_unit_size is used for, so
	   we will flag it for now */
	if(raw_inode->file_unit_size[0] != 0){
		printk("File unit size != 0 for ISO file.(%d)\n",inode->i_ino);
	}

	/* I have no idea what other flag bits are used for, so
	   we will flag it for now */
	if((raw_inode->flags[-high_sierra] & ~2)!= 0){
		printk("Unusual flag settings for ISO file.(%d %x)\n",
		       inode->i_ino, raw_inode->flags[-high_sierra]);
	}

#ifdef DEBUG
	printk("Get inode %d: %d %d: %d\n",inode->i_ino, block, 
	       ((int)pnt) & 0x3ff, inode->i_size);
#endif
	
	inode->i_mtime = inode->i_atime = inode->i_ctime = 
	  iso_date(raw_inode->date, high_sierra);

	inode->u.isofs_i.i_first_extent = isonum_733 (raw_inode->extent) << 
		(ISOFS_BLOCK_BITS - ISOFS_BUFFER_BITS);
	
	inode->u.isofs_i.i_backlink = -1; /* Will be used for previous directory */
	switch (inode->i_sb->u.isofs_sb.s_conversion){
	case 'a':
	  inode->u.isofs_i.i_file_format = ISOFS_FILE_UNKNOWN; /* File type */
	  break;
	case 'b':
	  inode->u.isofs_i.i_file_format = ISOFS_FILE_BINARY; /* File type */
	  break;
	case 't':
	  inode->u.isofs_i.i_file_format = ISOFS_FILE_TEXT; /* File type */
	  break;
	case 'm':
	  inode->u.isofs_i.i_file_format = ISOFS_FILE_TEXT_M; /* File type */
	  break;
	};
	

/* Now test for possible Rock Ridge extensions which will override some of
   these numbers in the inode structure. */

	if (!high_sierra)
	  parse_rock_ridge_inode(raw_inode, inode);
	
#ifdef DEBUG
	printk("Inode: %x extent: %x\n",inode->i_ino, inode->u.isofs_i.i_first_extent);
#endif
	brelse(bh);
	
	if (cpnt) {
		free(cpnt);
		cpnt = 0;
	};
	
	inode->i_op = NULL;
	if (S_ISREG(inode->i_mode))
		inode->i_op = &isofs_file_inode_operations;
	else if (S_ISDIR(inode->i_mode))
		inode->i_op = &isofs_dir_inode_operations;
	else if (S_ISLNK(inode->i_mode))
		inode->i_op = &isofs_symlink_inode_operations;
	else if (S_ISCHR(inode->i_mode))
		inode->i_op = &isofs_chrdev_inode_operations;
	else if (S_ISBLK(inode->i_mode))
		inode->i_op = &isofs_blkdev_inode_operations;
}

/* There are times when we need to know the inode number of a parent of
   a particular directory.  When control passes through a routine that
   has access to the parent information, it fills it into the inode structure,
   but sometimes the inode gets flushed out of the queue, and someone
   remmembers the number.  When they try to open up again, we have lost
   the information.  The '..' entry on the disc points to the data area
   for a particular inode, so we can follow these links back up, but since
   we do not know the inode number, we do not actually know how large the
   directory is.  The disc is almost always correct, and there is
   enough error checking on the drive itself, but an open ended search
   makes me a little nervous.

   The bsd iso filesystem uses the extent number for an inode, and this
   would work really nicely for us except that the read_inode function
   would not have any clean way of finding the actual directory record
   that goes with the file.  If we had such info, then it would pay
   to change the inode numbers and eliminate this function.
*/

int isofs_lookup_grandparent(struct inode * parent, int extent) {
	unsigned int block,offset;
	int parent_dir, inode_number;
	int old_offset;
	char * cpnt = 0;
	int result;
	struct buffer_head * bh;
	struct iso_directory_record * de;
	
	offset = 0;
	block = extent << (ISOFS_BLOCK_BITS - ISOFS_BUFFER_BITS);
	if (!(bh = bread(parent->i_dev, block, ISOFS_BUFFER_SIZE)))  return 0;
	
	while (1 == 1) {
		de = (struct iso_directory_record *) (offset + bh->b_data);
		
		if (*((char*) de) == 0) 
			{
				brelse(bh);
				return -1;
			}
		
		offset += *((unsigned char*) de);
		
		if (offset >=  ISOFS_BUFFER_SIZE) 
			{
				printk(".. Directory not in first block of directory.\n");
				brelse(bh);
				return -1;
			}
		
		if (de->name_len[0] == 1 && de->name[0] == 1) 
			{
				brelse(bh);
				parent_dir = find_rock_ridge_relocation(de, parent);
				break;
			};
	}
#ifdef DEBUG
	printk("Parent dir:%x\n",parent_dir);
#endif
	/* Now we know the extent where the parent dir starts on.  We have no
	   idea how long it is, so we just start reading until we either find it
	   or we find some kind of unreasonable circumstance. */
	
	result = -1;
	
	offset = 0;
	block = parent_dir << (ISOFS_BLOCK_BITS - ISOFS_BUFFER_BITS);
	if (!block || !(bh = bread(parent->i_dev,block, ISOFS_BUFFER_SIZE)))
		return 0;
	
	while (1==1) {
		de = (struct iso_directory_record *) (offset + bh->b_data);
		inode_number = (block << ISOFS_BUFFER_BITS)+(offset & (ISOFS_BUFFER_SIZE - 1));
		
		/* If the length byte is zero, we should move on to the next CDROM sector.
		   If we are at the end of the directory, we kick out of the while loop. */
		
		if (*((char*) de) == 0) 
			{
				brelse(bh);
				offset = 0;
				block++;
				if(block & 1) return -1;
				if (!block || !(bh = bread(parent->i_dev,block, ISOFS_BUFFER_SIZE)))
					return -1;
				continue;
			}
		
		/* Make sure that the entire directory record is in the current bh block.
		   If not, we malloc a buffer, and put the two halves together, so that
		   we can cleanly read the block */
		
		old_offset = offset;
		offset += *((unsigned char*) de);
		if (offset >=  ISOFS_BUFFER_SIZE) 
			{
				cpnt = malloc(1 << ISOFS_BLOCK_BITS);
				memcpy(cpnt, bh->b_data, ISOFS_BUFFER_SIZE);
				de = (struct iso_directory_record *) (old_offset + cpnt);
				brelse(bh);
				offset -= ISOFS_BUFFER_SIZE;
				block++;
				if((block & 1) == 0) return -1;
				if (!(bh = bread(parent->i_dev,block, ISOFS_BUFFER_SIZE)))
					return -1;
				memcpy(cpnt+ISOFS_BUFFER_SIZE, bh->b_data, ISOFS_BUFFER_SIZE);
			}
		
		if (find_rock_ridge_relocation(de, parent) == extent){
			result = inode_number;
			goto out;
		};
		
		if (cpnt) {
			free(cpnt);
			cpnt = 0;
		};
	}
	/* We go here for any condition we cannot handle.  We also drop through
	   to here at the end of the directory. */
	
 out:
	if (cpnt) {
		free(cpnt);
		cpnt = 0;
	};
	brelse(bh);
#ifdef DEBUG
	printk("Resultant Inode %d\n",result);
#endif
	return result;
}
    
#ifdef LEAK_CHECK
#undef malloc
#undef free_s
#undef bread
#undef brelse

void * leak_check_malloc(unsigned int size){
  void * tmp;
  check_malloc++;
  tmp = malloc(size);
  return tmp;
}

void leak_check_free_s(void * obj, int size){
  check_malloc--;
  return free_s(obj, size);
}

struct buffer_head * leak_check_bread(int dev, int block, int size){
  check_bread++;
  return bread(dev, block, size);
}

void leak_check_brelse(struct buffer_head * bh){
  check_bread--;
  return brelse(bh);
}

#endif
