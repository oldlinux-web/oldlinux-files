/****************************************************************/
/*								*/
/*	de_diskio.c						*/
/*								*/
/*		Reading and writing to a file system device.	*/
/*								*/
/****************************************************************/
/*  origination         1989-Jan-15        Terrence W. Holm	*/
/****************************************************************/


#include <minix/config.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>

#include <minix/const.h>
#include <minix/type.h>
#include "../../fs/const.h"
#include "../../fs/type.h"
#include "../../fs/super.h"

#include "de.h"




/****************************************************************/
/*								*/
/*	Read_Disk( state, block_addr, buffer )			*/
/*								*/
/*		Reads a 1k block at "block_addr" into "buffer".	*/
/*								*/
/****************************************************************/


void Read_Disk( s, block_addr, buffer )
  de_state *s;
  off_t  block_addr;
  char  *buffer;

  {
  if ( lseek( s->device_d, block_addr, SEEK_SET ) == -1 )
    Error( "Error seeking %s", s->device_name );

  if ( read( s->device_d, buffer, K ) != K )
    Error( "Error reading %s", s->device_name );
  }






/****************************************************************/
/*								*/
/*	Read_Block( state, buffer )				*/
/*								*/
/*		Reads a 1k block from "state->address" into	*/
/*		"buffer". Checks "address", and updates		*/
/*		"block" and "offset".				*/
/*								*/
/****************************************************************/


void Read_Block( s, buffer )
  de_state *s;
  char *buffer;

  {
  off_t end_addr = (long) s->device_size * K - 1;
  off_t block_addr;

  if ( s->address < 0 )
    s->address = 0L;

  if ( s->address > end_addr )
    s->address = end_addr;

  /*  The address must be rounded off for  */
  /*  certain visual display modes.        */

  if ( s->mode == WORD )
    s->address &= ~1L;
  else if ( s->mode == MAP )
    s->address &= ~3L;


  block_addr = s->address & K_MASK;

  s->block  = (unsigned) (block_addr >> K_SHIFT);
  s->offset = (unsigned) (s->address - block_addr);

  Read_Disk( s, block_addr, buffer );
  }






/****************************************************************/
/*								*/
/*	Read_Super_Block( state )				*/
/*								*/
/*		Read and check the super block.			*/
/*								*/
/****************************************************************/


void Read_Super_Block( s )
  de_state *s;

  {
  struct super_block *super = (struct super_block *) s->buffer;

  Read_Disk( s, (long) 1 * K, s->buffer );

  s->inodes = super->s_ninodes;
  s->zones  = super->s_nzones;

  s->inode_maps   = bitmapsize(1 + s->inodes);
  s->zone_maps    = bitmapsize(s->zones);

  s->inode_blocks = (s->inodes + INODES_PER_BLOCK - 1) / INODES_PER_BLOCK;
  s->first_data   = 2 + s->inode_maps + s->zone_maps + s->inode_blocks;

  s->inodes_in_map = s->inodes + 1;
  s->zones_in_map  = s->zones + 1 - s->first_data;

  /*
  if ( s->zones != s->device_size )
    Warning( "Zone count does not equal device size" );
  */

  s->device_size = s->zones;

  if ( s->inode_maps != super->s_imap_blocks )
    Warning( "Corrupted inode map count in super block" );

  if ( s->zone_maps != super->s_zmap_blocks )
    Warning( "Corrupted zone map count in super block" );

  if ( s->first_data != super->s_firstdatazone )
    Warning( "Corrupted first data zone count in super block" );

  if ( super->s_log_zone_size != 0 )
    Warning( "Can not handle multiple blocks per zone" );

  if ( super->s_magic != SUPER_MAGIC )
    Warning( "Corrupted magic number in super block" );
  }






/****************************************************************/
/*								*/
/*	Read_Bit_Maps( state )					*/
/*								*/
/*		Read in the i-node and zone bit maps from the	*/
/*		specified file system device.			*/
/*								*/
/****************************************************************/


void Read_Bit_Maps( s )
  de_state *s;

  {
  int i;

  if ( s->inode_maps > I_MAP_SLOTS  ||  s->zone_maps > ZMAP_SLOTS )
    {
    Warning( "Super block specifies too many bit map blocks" );
    return;
    }

  for ( i = 0;  i < s->inode_maps;  ++i )
    {
    Read_Disk( s, (long) (2 + i) * K, &s->inode_map[ i * K ] );
    }

  for ( i = 0;  i < s->zone_maps;  ++i )
    {
    Read_Disk( s, (long) (2 + s->inode_maps + i) * K, &s->zone_map[ i * K ] );
    }
  }






/****************************************************************/
/*								*/
/*	Search( state, string )					*/
/*								*/
/*		Search from the current address for the ASCII	*/
/*		"string" on the device.				*/
/*								*/
/****************************************************************/


off_t Search( s, string )
  de_state *s;
  char *string;

  {
  off_t address   = s->address + 1;
  off_t last_addr = address;
  char  buffer[ SEARCH_BUFFER ];
  int   offset;
  int   tail_length = strlen( string ) - 1;
  int   count = SEARCH_BUFFER;
  int   last_offset;


  for (  ;  count == SEARCH_BUFFER;  address += SEARCH_BUFFER - tail_length )
    {
    if ( lseek( s->device_d, address, SEEK_SET ) == -1 )
      Error( "Error seeking %s", s->device_name );

    if ( (count = read( s->device_d, buffer, SEARCH_BUFFER)) == -1 )
      Error( "Error reading %s", s->device_name );


    if ( address - last_addr >= 500L * K )
      {
      putchar( '.' );
      fflush( stdout );

      last_addr += 500L * K;
      }


    last_offset = count - tail_length;

    for ( offset = 0;  offset < last_offset;  ++offset )
      {
      register char c = buffer[ offset ];

      if ( c == *string )
	{
	char *tail_buffer = &buffer[ offset + 1 ];
	char *tail_string = string + 1;

	do
	  {
	  if ( *tail_string == '\0' )
	    return( address + offset );
	  }
          while ( *tail_buffer++ == *tail_string++ );
        }
      }  /*  end for ( offset )  */
    }  /*  end for ( address )  */

  return( -1L );
  }






/****************************************************************/
/*								*/
/*	Write_Word( state, word )				*/
/*								*/
/*		Write a word at address.			*/
/*								*/
/****************************************************************/


void Write_Word( s, word )
  de_state *s;
  unsigned word;

  {
  if ( s->address & 01 )
    Error( "Internal fault (unaligned address)" );

  if ( lseek( s->device_d, s->address, SEEK_SET ) == -1 )
    Error( "Error seeking %s", s->device_name );

  if ( write( s->device_d, (char *) &word, 2 ) != 2 )
    Error( "Error writing %s", s->device_name );
  }






/* The next routine is copied from fsck.c and mkfs.c...  (Re)define some
 * things for consistency.  This sharing should be done better in a library
 * library routine.  In the library routine, the shifts should be replaced
 * by multiplications and divisions by MAP_BITS_PER_BLOCK since log2 of
 * this is too painful to get right.
 */
#define BITMAPSHIFT	 13	/* = log2(MAP_BITS_PER_BLOCK) */
#define bit_nr unsigned

/* Convert from bit count to a block count. The usual expression
 *
 *	(nr_bits + (1 << BITMAPSHIFT) - 1) >> BITMAPSHIFT
 *
 * doesn't work because of overflow.
 *
 * Other overflow bugs, such as the expression for N_ILIST overflowing when
 * s_inodes is just over INODES_PER_BLOCK less than the maximum+1, are not
 * fixed yet, because that number of inodes is silly.
 */
int bitmapsize(nr_bits)
bit_nr nr_bits;
{
	int nr_blocks;

	nr_blocks = nr_bits >> BITMAPSHIFT;
	if ((nr_blocks << BITMAPSHIFT) < nr_bits)
		++nr_blocks;
	return(nr_blocks);
}
