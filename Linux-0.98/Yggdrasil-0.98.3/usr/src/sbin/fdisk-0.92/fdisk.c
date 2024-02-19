/* fdisk.c -- Partition table manipulator for Linux.
 *
 * Copyright (C) 1992  A. V. Le Blanc (LeBlanc@mcc.ac.uk)
 *
 * This program is free software.  You can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation: either version 1 or
 * (at your option) any later version.
 *
 * Before Linux version 0.95c, this program requires a kernel patch.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

#include <linux/hdreg.h>
#include <linux/genhd.h>

#define hex_val(c)	({ \
				char _c = (c); \
				isdigit(_c) ? _c - '0' : \
					tolower(_c) + 10 - 'a'; \
			})
#define VERSION		"0.92"
#define DEFAULT_DEVICE	"/dev/hda"
#define LINE_LENGTH	80
#define MAXIMUM_PARTS	60
#define SECTOR_SIZE	512
#define PART_TABLE_FLAG 0xaa55
#define table_check(b)	((unsigned short *)((b) + 0x1fe))
#define offset(b, n) 	((struct partition *)((b) + 0x1be + \
	(n) * sizeof(struct partition)))
#define sector(s)	((s) & 0x3f)
#define cylinder(s, c)	((c) | (((s) & 0xc0) << 2))
#define calculate(h,s,c) ((cylinder(s,c) * sectors + sector(s) - 1) \
	* heads + (h))
#define t_head(t)	((t) % heads)
#define t_sector(t)	(sector((t) % sectors + 1) | (t) / sectors >> 2 \
	& 0xc0)
#define t_cylinder(t)	((t) / sectors & 0xff)
#define ACTIVE_FLAG	0x80
#define EXTENDED	5
#define LINUX_PARTITION	0x81
#define LINUX_SWAP	0x82

enum failure {usage, unable_to_open, unable_to_read, unable_to_seek,
	unable_to_write, out_of_memory};

char	*disk_device = DEFAULT_DEVICE,	/* hda, unless specified */
	*line_ptr,			/* interactive input */
	line_buffer[LINE_LENGTH],
	changed[MAXIMUM_PARTS],		/* marks changed buffers */
	buffer[SECTOR_SIZE],		/* first four partitions */
	*buffers[MAXIMUM_PARTS]		/* pointers to buffers */
		= {buffer, buffer, buffer, buffer};
int	fd,				/* the disk */
	ext_index,			/* the prime extended partition */
	partitions = 4;			/* maximum partition + 1 */
uint	heads,
	sectors,
	cylinders,
	extended_offset = 0,		/* offset of link pointers */
	offsets[MAXIMUM_PARTS] = {0, 0, 0, 0};
struct	partition *part_table[MAXIMUM_PARTS]	/* partitions */
		= {offset(buffer, 0), offset(buffer, 1),
		offset(buffer, 2), offset(buffer, 3)},
	*ext_pointers[MAXIMUM_PARTS]		/* link pointers */
		= {NULL, NULL, NULL, NULL};
struct systypes {
	unsigned char index;
	char *name;
	} sys_types[] = {
		{0, "Empty"},
		{1, "DOS 12-bit FAT"},
		{2, "XENIX"},
		{3, "Old XENIX"},
		{4, "DOS 16-bit <32M"},
		{EXTENDED, "Extended"},
		{6, "DOS 16-bit >=32M"},
		{7, "OS/2 HPFS"},
		{8, "AIX"},
		{9, "AIX bootable"},
		{10, "OPUS?"},
		{0x10, "OPUS?"},
		{0x40, "Venix"},
		{0x51, "Novell?"},
		{0x52, "CP/M?"},
		{0x63, "386/IX"},
		{0x64, "Novell"},
		{0x75, "PCIX"},
		{0x80, "Old MINIX"},
		{LINUX_PARTITION, "Linux/MINIX"},
		{LINUX_SWAP, "Linux swap"},
		{0x93, "Amoeba"},
		{0x94, "Amoeba BBT"},
		{0xdb, "CP/M"},
		{0xff, "BBT"}
	};

void fatal(enum failure why)
{
	char	error[LINE_LENGTH],
		*message = error;

	switch (why) {
		case usage: message = "Usage: fdisk [-v] [/dev/hdx]\n";
			break;
		case unable_to_open:
			sprintf(error, "Unable to open %s\n", disk_device);
			break;
		case unable_to_read:
			sprintf(error, "Unable to read %s\n", disk_device);
			break;
		case unable_to_seek:
			sprintf(error, "Unable to seek on %s\n", disk_device);
			break;
		case unable_to_write:
			sprintf(error, "Unable to write %s\n", disk_device);
			break;
		case out_of_memory:
			message = "Unable to allocate any more memory\n";
			break;
		default: message = "Fatal error\n";
	}
	fputs(message, stderr);
	exit(1);
}

void menu()
{
	puts("Command action\n"
		"   a   toggle a bootable flag\n"
		"   d   delete a partition\n"
		"   l   list known partition types\n"
		"   m   print this menu\n"
		"   n   add a new partition\n"
		"   p   print the partition table\n"
		"   q   quit without saving changes\n"
		"   t   change a partition's system id\n"
		"   v   verify the partition table\n"
		"   w   write table to disk and exit\n"
		"   x   extra functionality (experts only)"
	);
}

void xmenu()
{
	puts("Command action\n"
		"   b   move beginning of data in a partition\n"
		"   c   change number of cylinders\n"
		"   e   list extended partitions\n"
		"   h   change number of heads\n"
		"   m   print this menu\n"
		"   p   print the partition table\n"
		"   q   quit without saving changes\n"
		"   r   return to main menu\n"
		"   s   change number of sectors\n"
		"   w   write table to disk and exit"
	);
}

char *partition_type(unsigned char type)
{
	int high = sizeof(sys_types) / sizeof(struct systypes),
		low = 0, mid;
	uint tmp;

	while (high >= low) {
		mid = high + low >> 1;
		if ((tmp = sys_types[mid].index) == type)
			return sys_types[mid].name;
		else if (tmp < type)
			low = mid + 1;
		else high = mid - 1;
	}
	return NULL;
}

void list_types()
{
	uint last[4], done = 0, next = 0,
		size = sizeof(sys_types) / sizeof(struct systypes);
	int i;

	for (i = 3; i >= 0; i--)
		last[3 - i] = done += (size + i - done) / (i + 1);
	i = done = 0;

	do {
		printf("%c%2x  %-15.15s", i ? ' ' : '\n',
			sys_types[next].index, sys_types[next].name);
		next = last[i++] + done;
		if (i > 3 || next >= last[i]) {
			i = 0;
			next = ++done;
		}
	} while (done < last[0]);
	putchar('\n');
}

void clear_partition(struct partition *p)
{
	p->boot_ind = 0;
	p->head = 0;
	p->sector = 0;
	p->cyl = 0;
	p->sys_ind = 0;
	p->end_head = 0;
	p->end_sector = 0;
	p->end_cyl = 0;
	p->start_sect = 0;
	p->nr_sects = 0;
}

void set_partition(int i, struct partition *p, uint start, uint stop,
	int sys, uint offset)
{
	p->boot_ind = 0;
	p->sys_ind = sys;
	p->start_sect = start - offset;
	p->nr_sects = stop - start + 1;
	p->head = t_head(start);
	start /= heads;
	p->sector = t_sector(start);
	p->cyl = t_cylinder(start);
	p->end_head = t_head(stop);
	stop /= heads;
	p->end_sector = t_sector(stop);
	p->end_cyl = t_cylinder(stop);
	changed[i] = 1;
}

void read_extended(struct partition *p)
{
	int i;
	struct partition *q;

	ext_pointers[ext_index] = part_table[ext_index];
	if (!p->start_sect)
		printf("Bad offset in primary extended partition\n");
	else while (p->sys_ind == EXTENDED) {
		if (partitions >= MAXIMUM_PARTS) {
			printf("Warning: deleting partitions after %d\n",
				partitions);
			clear_partition(ext_pointers[partitions - 1]);
			changed[partitions - 1] = 1;
			return;
		}
		offsets[partitions] = extended_offset + p->start_sect;
		if (!extended_offset)
			extended_offset = p->start_sect;
		if (lseek(fd, offsets[partitions] * SECTOR_SIZE, SEEK_SET) < 0)
			fatal(unable_to_seek);
		if (!(buffers[partitions] = (char *) malloc(SECTOR_SIZE)))
			fatal(out_of_memory);
		if (SECTOR_SIZE != read(fd, buffers[partitions], SECTOR_SIZE))
			fatal(unable_to_read);
		part_table[partitions] = ext_pointers[partitions] = NULL;
		q = p = offset(buffers[partitions], 0);
		for (i = 0; i < 4; i++, p++) {
			if (p->sys_ind == EXTENDED)
				if (ext_pointers[partitions])
					printf("Warning: extra link pointer "
						"in partition table %d\n",
						partitions + 1);
				else
					ext_pointers[partitions] = p;
			else if (p->sys_ind)
				if (part_table[partitions])
					printf("Warning: ignoring extra data "
						"in partition table %d\n",
						partitions + 1);
				else
					part_table[partitions] = p;
		}
		if (!part_table[partitions])
			if (q != ext_pointers[partitions])
				part_table[partitions] = q;
			else part_table[partitions] = q + 1;
		if (!ext_pointers[partitions])
			if (q != part_table[partitions])
				ext_pointers[partitions] = q;
			else ext_pointers[partitions] = q + 1;
		p = ext_pointers[partitions++];
	}
}

void get_boot()
{
	int i;
	struct hd_geometry geometry;

	if ((fd = open(disk_device, O_RDWR)) < 0)
		fatal(unable_to_open);
	if (SECTOR_SIZE != read(fd, buffer, SECTOR_SIZE))
		fatal(unable_to_read);
	if (!ioctl(fd, HDIO_REQ, &geometry)) {
		heads = geometry.heads;
		sectors = geometry.sectors;
		cylinders = geometry.cylinders;
		if (cylinders > 1024) {
			printf("Reducing %d cylinders to 1024\n", cylinders);
			cylinders = 1024;
		}
	}
	if (!heads || !sectors || !cylinders)
		printf("Warning: unable to get disk geometry from system\n");

	for (i = 0; i < 4; i++)
		if(part_table[i]->sys_ind == EXTENDED)
			if (partitions != 4) {
				printf("Ignoring extra extended "
					"partition %d\n", i + 1);
			}
			else read_extended(part_table[ext_index = i]);
	for (i = 3; i < partitions; i++)
		if (*table_check(buffers[i]) != PART_TABLE_FLAG) {
			printf("Warning: invalid flag %04x of partition table "
				"%d will be corrected by w(rite)\n",
				*table_check(buffers[i]), i + 1);
			changed[i] = 1;
		}
}

int read_line()
{
	if (!fgets(line_buffer, LINE_LENGTH, stdin))
		return 0;
	line_ptr = line_buffer;
	while (*line_ptr && !isgraph(*line_ptr))
		line_ptr++;
	return *line_ptr;
}

char read_char(char *mesg)
{
	do
		fputs(mesg, stdout);
	while (!read_line());
	return *line_ptr;
}

uint read_int(uint low, uint high, char *mesg)
{
	uint i;
	char ms[40];
	sprintf(ms, "%s (%d-%d): ", mesg, low, high);

	while (1) {
		while (!isdigit(read_char(ms)));
		if ((i = atoi(line_ptr)) >= low && i <= high)
			break;
	}
	return i;
}

int get_partition(int warn, int max)
{
	int i = read_int(1, max, "Partition number") - 1;

	if (warn && !part_table[i]->sys_ind)
		printf("Warning: partition %d has empty type\n", i + 1);
	return i;
}

void toggle_active(int i)
{
	struct partition *p = part_table[i];

	if (p->sys_ind == EXTENDED) {
		printf("Partition %d is an extended partition\n",
			i + 1);
		p->boot_ind = 0;
	}
	else if (p->boot_ind)
		p->boot_ind = 0;
	else p->boot_ind = ACTIVE_FLAG;
	changed[i] = 1;
}

int warn_geometry()
{
	if (heads && sectors && cylinders)
		return 0;
	printf("You must set heads, sectors, and cylinders.\n"
		"You can do this from the extra functionality menu.\n");
	return 1;
}

void delete_partition(int i)
{
	struct partition *p = part_table[i], *q = ext_pointers[i];

/* Note that for the fifth partition (i == 4) we don't actually
 * decrement partitions.
 */

	if (warn_geometry())
		return;
	changed[i] = 1;
	if (i < 4) {
		if (p->sys_ind == EXTENDED && i == ext_index) {
			while (partitions > 4)
				free(buffers[--partitions]);
			ext_pointers[ext_index] = NULL;
			extended_offset = 0;
		}
		clear_partition(p);
	}
	else if (!q->sys_ind && i > 4) {
		free(buffers[--partitions]);
		clear_partition(ext_pointers[--i]);
	}
	else if (i > 3) {
		if (i > 4) {
			p = ext_pointers[i - 1];
			p->boot_ind = 0;
			p->head = q->head;
			p->sector = q->sector;
			p->cyl = q->cyl;
			p->sys_ind = EXTENDED;
			p->end_head = q->end_head;
			p->end_sector = q->end_sector;
			p->end_cyl = q->end_cyl;
			p->start_sect = q->start_sect;
			p->nr_sects = q->nr_sects;
			changed[i - 1] = 1;
		}
		else {
			part_table[5]->start_sect +=
				offsets[5] - extended_offset;
			offsets[5] = extended_offset;
			changed[5] = 1;
		}
		if (partitions > 5) {
			partitions--;
			free(buffers[i]);
			while (i < partitions) {
				changed[i] =  changed[i + 1];
				buffers[i] =  buffers[i + 1];
				offsets[i] =  offsets[i + 1];
				part_table[i] =  part_table[i + 1];
				ext_pointers[i] =  ext_pointers[i + 1];
				i++;
			}
		}
		else
			clear_partition(part_table[i]);
	}
}

void change_sysid()
{
	char *temp;
	int i = get_partition(0, partitions), sys;
	struct partition *p = part_table[i];

	if ((sys = p->sys_ind) == EXTENDED)
		printf("Partition %d is extended.  Delete it\n", i + 1);
	else if (!sys)
		printf("Partition %d does not exist yet!\n", i + 1);
	else while (1) {
		read_char("Hex code (type L to list codes): ");
		if (tolower(*line_ptr) == 'l')
			list_types();
		else if (isxdigit(*line_ptr)) {
			sys = 0;
			do
				sys = sys << 4 | hex_val(*line_ptr++);
			while (isxdigit(*line_ptr));
			if (!sys) {
				delete_partition(i);
				break;
			}
			else if (sys == EXTENDED) {
				printf("You may not change a partition "
					"to be an extended partition\n");
				break;
			}
			else if (sys < 256) {
				if (sys == p->sys_ind)
					break;
				part_table[i]->sys_ind = sys;
				printf ("Changed system type of partition %d "
					"to %x (%s)\n", i + 1, sys,
					(temp = partition_type(sys)) ? temp :
					"Unknown");
				changed[i] = 1;
				break;
			}
		}
	}
}

void list_table()
{
	struct partition *p;
	char *type;
	int i, w = strlen(disk_device);

	printf("\nDisk %s: %d heads, %d sectors, %d cylinders\n\n",
		disk_device, heads, sectors, cylinders);
	if (w < 5)
		w = 5;
	printf("%*s Boot  Begin   Start     End  Blocks  Id  System\n",
		w + 1, "Device");
	for (i = 0 ; i < partitions; i++)
		if ((p = part_table[i])->sys_ind)
			printf("%*s%-2d  %c%8d%8d%8d%8d%c  %2x  %s\n", w,
				disk_device, i + 1,
				!p->boot_ind ? ' ' : p->boot_ind == ACTIVE_FLAG
					? '*' : '?',
				calculate(p->head, p->sector, p->cyl),
				p->start_sect + offsets[i],
				p->start_sect + offsets[i] + p->nr_sects -
					(p->nr_sects ? 1: 0),
				p->nr_sects / 2, p->nr_sects & 1 ? '+' : ' ',
				p->sys_ind,
				(type = partition_type(p->sys_ind)) ?
					type : "Unknown");
}

void x_list_table(int extend)
{
	struct partition *p, **q;
	int i;

	if (extend)
		q = ext_pointers;
	else
		q = part_table;
	printf("\nDisk %s: %d heads, %d sectors, %d cylinders\n\n",
		disk_device, heads, sectors, cylinders);
	printf("Nr AF  Hd Sec  Cyl  Hd Sec  Cyl  Start   Size ID\n");
	for (i = 0 ; i < partitions; i++) {
		if (p = q[i])
			printf("%2d %02x%4d%4d%5d%4d%4d%5d%7d%7d %02x\n",
				i + 1, p->boot_ind, p->head,
				sector(p->sector),
				cylinder(p->sector, p->cyl), p->end_head,
				sector(p->end_sector),
				cylinder(p->end_sector, p->end_cyl),
				p->start_sect, p->nr_sects, p->sys_ind);
	}
}

void check_bounds(uint *first, uint *last)
{
	int i;
	uint max = 256 * 63 * 1024;
	struct partition *p = part_table[0];

	for (i = 0; i < partitions; p = part_table[++i])
		if (!p->sys_ind || p->sys_ind == EXTENDED) {
			first[i] = max;
			last[i] = 0;
		}
		else {
			first[i] = calculate(p->head, p->sector, p->cyl);
			last[i] = p->start_sect + offsets[i] + p->nr_sects - 1;
		}
}

void check(int n, uint h, uint s, uint c, uint start)
{
	uint total, real_s, real_c;

	real_s = sector(s) - 1;
	real_c = cylinder(s, c);
	total = (real_c * sectors + real_s) * heads + h;
	if (!total)
		printf("Warning: partition %d contains sector 0\n", n);
	if (h >= heads)
		printf("Partition %d: head %d greater than maximum %d\n",
			n, h + 1, heads);
	if (real_s >= sectors)
		printf("Partition %d: sector %d greater than "
			"maximum %d\n", n, s, sectors);
	if (real_c >= cylinders)
		printf("Partitions %d: cylinder %d greater than "
			"maximum %d\n", n, real_c + 1, cylinders);
	if (start != total)
		printf("Partition %d: previous sectors %d disagrees with "
			"total %d\n", n, start, total);
}

void verify()
{
	int i, j;
	uint total = 1,
		first[partitions], last[partitions];
	struct partition *p = part_table[0];

	if (warn_geometry())
		return;

	check_bounds(first, last);
	for (i = 0; i < partitions; p = part_table[++i])
		if (p->sys_ind && (p->sys_ind != EXTENDED)) {
			if (p->start_sect + offsets[i] < first[i])
				printf("Warning: bad start-of-data in "
					"partition %d\n", i + 1);
			check(i + 1, p->end_head, p->end_sector, p->end_cyl,
				last[i]);
			total += last[i] + 1 - first[i];
			for (j = 0; j < i; j++)
				if (first[i] >= first[j] && first[i] <= last[j]
						|| last[i] <= last[j] &&
						last[i] >= first[j]) {
					printf("Warning: partition %d overlaps "
						"partition %d.\n", j + 1,
						i + 1);
					total += first[i] >= first[j] ?
						first[i] : first[j];
					total -= last[i] <= last[j] ?
						last[i] : last[j];
				}
		}

	if (extended_offset) {
		uint e_last = part_table[ext_index]->start_sect +
			part_table[ext_index]->nr_sects - 1;

		for (p = part_table[i = 4]; i < partitions;
				p = part_table[++i]) {
			total++;
			if (!p->sys_ind) {
				if (i != 4 || i + 1 < partitions)
					printf("Warning: partition %d "
						"is empty\n", i + 1);
			}
			else if (first[i] < extended_offset ||
					last[i] > e_last)
				printf("Logical partition %d not entirely in "
					"partition %d\n", i + 1, ext_index + 1);
		}
	}

	if (total > heads * sectors * cylinders)
		printf("Total allocated sectors %d greater than the maximum "
			"%d\n", total, heads * sectors * cylinders);
	else if (total = heads * sectors * cylinders - total)
		printf("%d unallocated sectors\n", total);
}

void add_partition(int n, int sys)
{
	int i, read = 0;
	struct partition *p = part_table[n], *q = part_table[ext_index];
	uint start, stop, limit, temp,
		first[partitions], last[partitions];

	if (p->sys_ind) {
		printf("Partition %d is already defined.  Delete "
			"it before re-adding it.\n", n + 1);
		return;
	}
	check_bounds(first, last);
	if (n < 4) {
		start = 1;
		limit = heads * sectors * cylinders - 1;
		if (extended_offset) {
			first[ext_index] = extended_offset;
			last[ext_index] = q->start_sect + q->nr_sects - 1;
		}
	}
	else {
		start = extended_offset + 1;
		limit = q->start_sect + q->nr_sects - 1;
	}

	do {
		temp = start;
		for (i = 0; i < partitions; i++) {
			if (start == offsets[i])
				start++;
			if (start >= first[i] && start <= last[i])
				start = last[i] + 1;
		}
		if (start > limit)
			break;
		if (start != temp && read) {
			printf("Sector %d is already allocated\n", temp);
			temp = start = stop;
			read = 0;
		}
		if (!read && start == temp) {
			start = read_int((stop = start) + (n > 4), limit,
				"First sector");
			read = 1;
		}
	} while (start != temp && !read);
	if (n > 4)			/* NOT for fifth partition */
		offsets[n] = start - 1;

	for (i = 0; i < partitions; i++) {
		if (start < offsets[i] && limit >= offsets[i])
			limit = offsets[i] - 1;
		if (start < first[i] && limit >= first[i])
			limit = first[i] - 1;
	}
	if (start > limit) {
		printf("No free sectors available\n");
		if (n > 4) {
			free(buffers[n]);
			partitions--;
		}
		return;
	}
	if (start == limit)
		stop = start;
	else
		stop = read_int(start, limit, "Last sector");

	set_partition(n, p, start, stop, sys, offsets[n]);

	if (sys == EXTENDED) {
		ext_index = n;
		offsets[4] = extended_offset = start;
		ext_pointers[n] = p;
		if (!(buffers[4] = calloc(1, SECTOR_SIZE)))
			fatal(out_of_memory);
		part_table[4] = offset(buffers[4], 0);
		ext_pointers[4] = part_table[4] + 1;
		changed[4] = 1;
		partitions = 5;
	}
	else {
		if (n > 4)
			set_partition(n - 1, ext_pointers[n - 1], start - 1,
				stop, EXTENDED, extended_offset);
#if 0
	/* The warning about the odd number of sectors is confusing to
	   users and only saves and average of 256 bytes per disk (i.e.,
	   it saves 512 bytes half of the time, and no bytes the other
	   half of the time. */

		if ((limit = p->nr_sects) & 1)
			printf("Warning: partition %d has an odd "
				"number of sectors.\n", n + 1);

	/* The warning about having too sectors does not apply to the
	   extended file system and won't apply to the log structured
	   file system. */
		if (limit > 131070)
			printf("Warning: Linux cannot currently use %d "
				"sectors of this partition\n",
				limit - 131070);
#endif
	}
}

void add_logical()
{
	if (partitions > 5 || part_table[4]->sys_ind) {
		if (!(buffers[partitions] = calloc(1, SECTOR_SIZE)))
			fatal(out_of_memory);
		part_table[partitions] = offset(buffers[partitions], 0);
		ext_pointers[partitions] = part_table[partitions] + 1;
		offsets[partitions] = 0;
		partitions++;
	}
	add_partition(partitions - 1, LINUX_PARTITION);
}

void new_partition()
{
	int i, free_primary = 0;

	if (warn_geometry())
		return;
	if (partitions >= MAXIMUM_PARTS) {
		printf("The maximum number of partitions has been created\n");
		return;
	}

	for (i = 0; i < 4; i++)
		free_primary += !part_table[i]->sys_ind;
	if (!free_primary)
		if (extended_offset)
			add_logical();
		else
			printf("You must delete some partition and add "
				"an extended partition first\n");
	else {
		char c, line[LINE_LENGTH];
		sprintf(line, "Command action\n   %s\n   p   primary "
			"partition (1-4)\n", extended_offset ?
			"l   logical (5 or over)" : "e   extended");
		while (1) {
			if ((c = tolower(read_char(line))) == 'p') {
				add_partition(get_partition(0, 4),
					LINUX_PARTITION);
				return;
			}
			else if (c == 'l' && extended_offset) {
				add_logical();
				return;
			}
			else if (c == 'e' && !extended_offset) {
				add_partition(get_partition(0, 4),
					EXTENDED);
				return;
			}
		}
	}
}

void write_table()
{
	int i;

	changed[3] = changed[0] || changed[1] || changed[2] || changed[3];
	for (i = 3; i < partitions; i++)
		if (changed[i]) {
			*table_check(buffers[i]) = PART_TABLE_FLAG;
			if (lseek(fd, offsets[i] * SECTOR_SIZE, SEEK_SET) < 0)
				fatal(unable_to_seek);
			if (write(fd, buffers[i], SECTOR_SIZE) != SECTOR_SIZE)
				fatal(unable_to_write);
	}
	close(fd);
	sync();
	printf("The partition table has been altered.\n"
		"Please reboot before doing anything else.\n");
	exit(0);
}

void move_begin(int i)
{
	struct partition *p = part_table[i];
	uint new, first;

	if (warn_geometry())
		return;
	if (!p->sys_ind || !p->nr_sects || p->sys_ind == EXTENDED) {
		printf("Partition %d has no data area\n", i + 1);
		return;
	}
	first = calculate(p->head, p->sector, p->cyl);
	new = read_int(first, p->start_sect + p->nr_sects + offsets[i] - 1,
		"New beginning of data") - offsets[i];

	if (new != p->nr_sects) {
		first = p->nr_sects + p->start_sect - new;
		p->nr_sects = first;
		p->start_sect = new;
		changed[i] = 1;
	}
}

void xselect()
{
	while(1) {
		putchar('\n');
		switch (tolower(read_char("Expert command (m for help): "))) {
			case 'b': move_begin(get_partition(0, partitions));
				break;
			case 'c': cylinders = read_int(1, 1024,
					"Number of cylinders");
				break;
			case 'e': x_list_table(1);
				break;
			case 'h': heads = read_int(1, 256, "Number of heads");
				break;
			case 'p': x_list_table(0);
				break;
			case 'q': close(fd);
				exit(0);
			case 'r': return;
			case 's': sectors = read_int(1, 63,
					"Number of sectors");
				break;
			case 'w': write_table();
			default: xmenu();
		}
	}
}

void main(int argc, char ** argv)
{
	if (argc > 3)
		fatal(usage);
	if (argc > 1 && *argv[1] == '-') {
		if (*(argv[1] + 1) != 'v')
			fatal(usage);
		printf("Version " VERSION "\n");
		if (argc == 2)
			exit(0);
	}
	if (argc > 1)
		disk_device = argv[argc - 1];
	get_boot();

	while (1) {
		putchar('\n');
		switch (tolower(read_char("Command (m for help): "))) {
			case 'a': toggle_active(get_partition(1, partitions));
				break;
			case 'd': delete_partition(get_partition(1, partitions));
				break;
			case 'l': list_types();
				break;
			case 'n': new_partition();
				break;
			case 'p': list_table();
				break;
			case 'q': close(fd);
				exit(0);
			case 't': change_sysid();
				break;
			case 'v': verify();
				break;
			case 'w': write_table();
			case 'x': xselect();
				break;
			default: menu();
		}
	}
}
