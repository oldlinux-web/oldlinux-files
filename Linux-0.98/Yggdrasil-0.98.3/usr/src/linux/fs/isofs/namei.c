/*
 *  linux/fs/isofs/namei.c
 *
 *  (C) 1992  Eric Youngdale Modified for ISO9660 filesystem.
 *
 *  (C) 1991  Linus Torvalds - minix filesystem
 */

#include <linux/sched.h>
#include <linux/iso_fs.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <asm/segment.h>

#include <linux/errno.h>
#include <const.h>

/*
 * ok, we cannot use strncmp, as the name is not in our data space.
 * Thus we'll have to use isofs_match. No big problem. Match also makes
 * some sanity tests.
 *
 * NOTE! unlike strncmp, isofs_match returns 1 for success, 0 for failure.
 */
static int isofs_match(int len,const char * name, char * compare, int dlen)
{
	register int same __asm__("ax");
	
	if (!compare) return 0;
	/* "" means "." ---> so paths like "/usr/lib//libc.a" work */
	if (!len && (compare[0]==0) && (dlen==1))
		return 1;
	
	if (compare[0]==0 && dlen==1 && len == 1)
		compare = ".";
 	if (compare[0]==1 && dlen==1 && len == 2) {
		compare = "..";
		dlen = 2;
	};
#if 0
	if (len <= 2) printk("Match: %d %d %s %d %d \n",len,dlen,compare,de->name[0], dlen);
#endif
	
	if (dlen != len)
		return 0;
	__asm__("cld\n\t"
		"fs ; repe ; cmpsb\n\t"
		"setz %%al"
		:"=a" (same)
		:"0" (0),"S" ((long) name),"D" ((long) compare),"c" (len)
		:"cx","di","si");
	return same;
}

/*
 *	isofs_find_entry()
 *
 * finds an entry in the specified directory with the wanted name. It
 * returns the cache buffer in which the entry was found, and the entry
 * itself (as an inode number). It does NOT read the inode of the
 * entry - you'll have to do that yourself if you want to.
 */
static struct buffer_head * isofs_find_entry(struct inode * dir,
	const char * name, int namelen, int * ino, int * ino_back)
{
	unsigned int block,i, f_pos, offset, inode_number;
	struct buffer_head * bh;
	char * cpnt = 0;
	unsigned int old_offset;
	unsigned int backlink;
	int dlen, rrflag, match;
	char * dpnt;
	struct iso_directory_record * de;
	char c;

	*ino = 0;
	if (!dir) return NULL;
	
	if (!(block = dir->u.isofs_i.i_first_extent)) return NULL;
  
	f_pos = 0;
	
	offset = f_pos & (ISOFS_BUFFER_SIZE - 1);
	block = isofs_bmap(dir,f_pos >> ISOFS_BUFFER_BITS);
	if (!block || !(bh = bread(dir->i_dev,block,ISOFS_BUFFER_SIZE))) return NULL;
  
	while (f_pos < dir->i_size) {
		de = (struct iso_directory_record *) (offset + bh->b_data);
		backlink = dir->i_ino;
		inode_number = (block << ISOFS_BUFFER_BITS)+(offset & (ISOFS_BUFFER_SIZE - 1));
		
		/* If byte is zero, this is the end of file, or time to move to
		   the next sector. Usually 2048 byte boundaries. */
		
		if (*((unsigned char*) de) == 0) {
			brelse(bh);
			offset = 0;
			f_pos =(f_pos & ~(ISOFS_BLOCK_SIZE - 1))+ISOFS_BLOCK_SIZE;
			block = isofs_bmap(dir,(f_pos)>>ISOFS_BUFFER_BITS);
			if (!block || !(bh = bread(dir->i_dev,block,ISOFS_BUFFER_SIZE)))
				return 0;
			continue; /* Will kick out if past end of directory */
		};
		
		old_offset = offset;
		offset += *((unsigned char*) de);
		f_pos += *((unsigned char*) de);
		
		/* Handle case where the directory entry spans two blocks. Usually
		   1024 byte boundaries */
		
		if (offset >=  ISOFS_BUFFER_SIZE) {
			cpnt = malloc(1 << ISOFS_BLOCK_BITS);
			memcpy(cpnt, bh->b_data, ISOFS_BUFFER_SIZE);
			de = (struct iso_directory_record *) (old_offset + cpnt);
			brelse(bh);
			offset = f_pos & (ISOFS_BUFFER_SIZE - 1);
			block = isofs_bmap(dir,f_pos>>ISOFS_BUFFER_BITS);
			if (!block || !(bh = bread(dir->i_dev,block,ISOFS_BUFFER_SIZE)))
				return 0;
			memcpy(cpnt+ISOFS_BUFFER_SIZE, bh->b_data, ISOFS_BUFFER_SIZE);
		}
		
		/* Handle the '.' case */
		
		if (de->name[0]==0 && de->name_len[0]==1) {
			inode_number = dir->i_ino;
			backlink = 0;
		}
		
		/* Handle the '..' case */

		if (de->name[0]==1 && de->name_len[0]==1) {
#if 0
			printk("Doing .. (%d %d)",dir->i_sb->s_firstdatazone << ISOFS_BUFFER_BITS, dir->i_ino);
#endif
			if((dir->i_sb->u.isofs_sb.s_firstdatazone << ISOFS_BUFFER_BITS) != dir->i_ino)
				inode_number = dir->u.isofs_i.i_backlink;
			else
				inode_number = dir->i_ino;
			backlink = 0;
		}
    
		dlen = de->name_len[0];
		dpnt = de->name;
		/* Now convert the filename in the buffer to lower case */
		if (rrflag = get_rock_ridge_filename(de, &dpnt, &dlen, dir)) {
		  if (rrflag == -1) goto out; /* Relocated deep directory */
		} else {
		  if(dir->i_sb->u.isofs_sb.s_mapping == 'n') {
		    for (i = 0; i < dlen; i++) {
			c = dpnt[i];
			if (c >= 'A' && c <= 'Z') c |= 0x20;  /* lower case */
			if (c == ';' && i == dlen-2 && dpnt[i+1] == '1') {
				dlen -= 2;
				break;
			};
			if (c == ';') c = '.';
			de->name[i] = c;
		};
		/* This allows us to match with and without a trailing period  */
		if(dpnt[dlen-1] == '.' && namelen == dlen-1)
			dlen--;
		  };
		};
		match = isofs_match(namelen,name,dpnt, dlen);
		if (cpnt) { free(cpnt); cpnt = 0;};

		if(rrflag) free(dpnt);
		if (match) {
			if(inode_number == -1) { /* Should only happen for the '..' entry */
				inode_number = 
					isofs_lookup_grandparent(dir,
					     find_rock_ridge_relocation(de,dir));
				if(inode_number == -1){ /* Should never happen */
					printk("Backlink not properly set.\n");
					goto out;
				};
			};
			*ino = inode_number;
			*ino_back = backlink;
			return bh;
		      }
	}
 out:
	if (cpnt) free(cpnt);
	brelse(bh);
	return NULL;
}

int isofs_lookup(struct inode * dir,const char * name, int len,
	struct inode ** result)
{
	int ino, ino_back;
	struct buffer_head * bh;

#ifdef DEBUG
	printk("lookup: %x %d\n",dir->i_ino, len);
#endif
	*result = NULL;
	if (!dir)
		return -ENOENT;

	if (!S_ISDIR(dir->i_mode)) {
		iput(dir);
		return -ENOENT;
	}

	ino = 0;
	while(cache.lock);
	cache.lock = 1;
	if (dir->i_dev == cache.dev && 
	    dir->i_ino == cache.dir &&
	    len == cache.dlen && 
	    isofs_match(len, name, cache.filename, cache.dlen))
	  {
	    ino = cache.ino;
	    ino_back = dir->i_ino;
	    /* These two cases are special, but since they are at the start
	       of the directory, we can just as easily search there */
	    if (cache.dlen == 1 && cache.filename[0] == '.') ino = 0;
	    if (cache.dlen == 2 && cache.filename[0] == '.' && 
		cache.filename[1] == '.') ino = 0;
	  };
	cache.lock = 0;

	if (!ino) {
	  if (!(bh = isofs_find_entry(dir,name,len, &ino, &ino_back))) {
	    iput(dir);
	    return -ENOENT;
	  }
	  brelse(bh);
	};

	if (!(*result = iget(dir->i_sb,ino))) {
		iput(dir);
		return -EACCES;
	}

	/* We need this backlink for the .. entry */
	
	if (ino_back) (*result)->u.isofs_i.i_backlink = ino_back; 
	
	iput(dir);
	return 0;
}
