/* df.c: Copywrite (92) Peter MacDonald:  distribute freely, don't restrict. */

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/fs.h>

char *hdr1="\n            inodes  inodes  inodes     blocks  blocks  blocks  mount";
char *hdr2=  "device      total   used    free       total   used    free    point";
char *hdr3=  "--------------------------------------------------------------------";
char *fmt =  "%-9s   %-7d %-7d %-7d    %-7d %-7d %-7d %s\n";

void do_df(char *dev, char *dir);

int main(int argc, char *argv[])
{ int i;

  sync();
  puts(hdr1);
  puts(hdr2);
  puts(hdr3);
  if (argc != 1)
    for (i=1; i<argc; i++)
      do_df(argv[i],"");
  else
  { FILE *F = fopen("/etc/mtab","r");    
    if (!F)
    { fprintf(stderr,"/etc/mtab not found\n");
      exit(1);
    }
    do 
    { char buf[200], dev[40], dir[150];
      fgets(buf,200,F);
      if (feof(F) || (strlen(buf)<6))
        break;
      sscanf(buf,"%s %s",dev,dir);
      do_df(dev,dir);  
    } while (1);  
  }
  exit(0);
}


#define oops(str,arg) { fprintf(stderr,str,arg); close(fd); return; }

int fd;
int nibblemap[] = { 0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4 };

ino_t count(unsigned numblocks, unsigned numbits)
{ unsigned i, j, end, sum = 0;
  char buf[BLOCK_SIZE];
  
  for (i=0; (i<numblocks) && numbits; i++)
  {
    if (read(fd,buf,sizeof(buf))<0) 
      return(0);
    if (numbits >= (8*BLOCK_SIZE))
    { 
      end = BLOCK_SIZE;
      numbits -= 8*BLOCK_SIZE;
    }
    else
    { int tmp;
      end = numbits >> 3;
      numbits &= 0x7;
      tmp = buf[end] & ((1<<numbits)-1);
      sum += nibblemap[tmp&0xf] + nibblemap[(tmp>>4)&0xf];
      numbits = 0;
    }  
    for (j=0; j<end; j++)
      sum += nibblemap[buf[j] & 0xf] + nibblemap[(buf[j]>>4)&0xf];
  }
  return(sum);
}

void do_df(char *dev, char *dir)
{ int it,iu,bt,bu;
  struct super_block supstruct;
  
  if ((fd=open(dev,O_RDONLY))<0)
    oops("df can't open device: %s",dev);
    
  lseek(fd,BLOCK_SIZE,SEEK_SET);
  if (read(fd,(char*)&supstruct,sizeof(struct super_block)) 
    != sizeof(struct super_block))
    oops("super block unreadable: %s",dev);
    
  lseek(fd,BLOCK_SIZE*2,SEEK_SET);
  if (supstruct.s_magic != SUPER_MAGIC)
    oops("not a valid file system: %s",dev);
    
  it = supstruct.s_ninodes;
  iu = count(supstruct.s_imap_blocks,supstruct.s_ninodes+1);

  bt = supstruct.s_nzones << supstruct.s_log_zone_size;
  bu = count(supstruct.s_zmap_blocks,supstruct.s_nzones) << 
    supstruct.s_log_zone_size;
  
  printf(fmt,dev,it,iu,it-iu,bt,bu,bt-bu,dir);
}
