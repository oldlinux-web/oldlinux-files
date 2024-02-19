/* the simple ide disk drive for cnix,it is bug about disk order,I use the default first master
 * disk for the pc.if your pc isn't the configuer ,it will error! I default hda! the support the
 * one time extended partition!
*/


#include <asm/io.h>
#include <cnix/ide.h> 
#include <cnix/partition.h>
#include <string.h>

#define ide_io_read(port,buf,nr) \
__asm__("cld;rep;insw"::"d" (port),"D" (buf),"c" (nr))

#define ide_io_write(port,buf,nr) \
__asm__("cld;rep;outsw"::"d" (port),"S" (buf),"c" (nr))


ide_stutas_ok()
{
      while(inb(HD_STATUS) & BUSY_STAT){
	    delay_50ms();
	    
      }
}

#define READ_CMD 0x20

lba_read_sector(long sector,unsigned char *buf)
{
        int c,h,s;

       
	s = sector &0xff;
	c = (sector &0xffff00)>>8;
	h = (sector &0xf000000)>>24;

     	outb(1,HD_NSECTOR);

	outb(s,HD_SECTOR);
	outb(c,HD_LCYL);
	outb(c>>8,HD_HCYL);
	/*the master disk*/
	outb(0xE0|(0<<4)|h,HD_CURRENT);
	outb(READ_CMD,HD_COMMAND);

	ide_stutas_ok();
	ide_io_read(HD_DATA,buf,256);
}

/*cnix don't used the read mode */
ide_chs_read(int c,int h,int s,unsigned char *buf)
{
	/* the new ide have not the attrib
	outb(_WPCOM,HD_ERROR);
	*/
        /*READ sector data,the asmcos default=1*/
	outb(1,HD_NSECTOR);

	outb(s,HD_SECTOR);
	outb(c,HD_LCYL);
	outb(c>>8,HD_HCYL);
	/*the master disk*/
	outb(0xA0|(0<<4)|h,HD_CURRENT);
	outb(READ_CMD,HD_COMMAND);

	ide_stutas_ok();
	ide_io_read(HD_DATA,buf,256);

}

get_ide_info()
{
        int status;
        hd_driveid id; 
      
        unsigned short *ver_add;
	unsigned char *disk_add;
	unsigned int  cyl;
	unsigned char  head;
	unsigned int  sect;
	unsigned int ver_cs,ver_offset,ver_line;

        outb(0xa0,HD_CURRENT);
        outb(0xec,HD_STATUS);
      
        while(inb(HD_STATUS)&0x80);
	
	ide_io_read(HD_DATA,&id,256);
	/*bios C/H/S */
	ver_add = 0x104;
	ver_offset = ver_add[0];
	ver_cs = ver_add[1];
	ver_line= ver_cs*0x10 + ver_offset;
	disk_add = ver_line; 
	
	cyl =  (disk_add[1]<<8)+disk_add[0];
	head = disk_add[2];
	sect = disk_add[14];
	/*lba mode*/
        cyl = id.lba_capacity /(head*sect) ;
	printk("Your disk C:%d,H:%d,S:%d,LBA:%d\n",cyl,head,sect,id.lba_capacity);
}
/*start for get partition info*/
struct disk_partition part_p[8]; /*primary*/

get_partition()
{
	int i;
	unsigned char st_sector[512];

	lba_read_sector(0,st_sector);
	
        for(i=0;i<4;i++){
	  memcpy(&part_p[i].part.boot_ind,&st_sector[0x1be + 16*i],16);
	  memcpy(part_p[i].name,"hda",3);
	  part_p[i].name[3]=i+0x31;
	}

	find_extended_part();
}



#define DOS_EXT   0x5
#define CHS_EXT   0xf
#define LINUX_EXT 0x85
#define EXT2_PART 0x83

int find_extended_part()
{
         unsigned char buf[512];
         int i;
	
	 for(i=0;i<4;i++){
	   switch(part_p[i].part.sys_ind)
	     {
	     case DOS_EXT:
	     case CHS_EXT:
	     case LINUX_EXT:
	       lba_read_sector(part_p[i].part.start_sect,buf);
	       break;
 
	     default:
	       break;
	     }
	 }
	 	
        for(i=0;i<4;i++){
	  memcpy(&part_p[i+4].part.boot_ind,&buf[0x1be + 16*i],16);
	  memcpy(part_p[i+4].name,"hda",3);
	  part_p[i+4].name[3]=i+0x35;
	}

}


const char *
get_sysname(unsigned char type) {
       struct systypes *s;

       for (s = sys_types; s->type<0xff; s++)
            if (s->type == type)
	          return s->name;
       return "Unknown,wait you add";
}



print_part(struct disk_partition *part_p)
{
         int enable;
	 int i;

	 char *part_type;

	 if (part_p->part.sys_ind ==0) return;

	 part_type = get_sysname(part_p->part.sys_ind);

	 enable = (part_p->part.boot_ind &0x80)>>1;
	 printk("%s:\n",part_p->name);
	 printk("Filesystem type ID:%s\n",part_type);
	 printk("Size :%d!Mb\n\n",part_p->part.nr_sects/2048);
}
print_part_list()
{
         int i;

	 for(i=0;i<8;i++)
	   print_part(&part_p[i]);

}

ide_init()
{
	get_ide_info();
}
