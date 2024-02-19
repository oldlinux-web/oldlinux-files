#
# Makefile for cnix.

AS	= as
LD	= ld
LDFLAGS	= -m elf_i386 -Ttext 0x100000 -e  on_journey
CC	= gcc $(RAMDISK)
CFLAGS	= -Wall -O2 -fstrength-reduce -fomit-frame-pointer \
	 -finline-functions -nostdinc -I../include
CPP	= cpp -nostdinc -Iinclude

ARCHIVES = driver/driver.o mm/mm.o fs/fs.o kernel/kernel.o shell/shell.o
LIBS	= lib/lib.a

all:	Image

Image: boot/boot tools/system tools/build
	objcopy -O binary -R .note -R .comment tools/system tools/kernel
	tools/build boot/boot tools/kernel > Image
	tools/ndisasm -b32 tools/kernel > System.asm
	rm tools/kernel -f
	sync

disk:	Image tools/makeimg
	tools/makeimg Image a.img

tools/build:
	(cd tools; make)

tools/system:	boot/head.o init/main.o \
	$(ARCHIVES) $(LIBS)
	$(LD) $(LDFLAGS) boot/head.o init/main.o \
	$(ARCHIVES) \
	$(LIBS) \
	-o tools/system  
	nm tools/system | grep -v '\(compiled\)\|\(\.o$$\)\|\( [aU] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)'| sort > System.map

init/main.o:
	(cd init;make)

boot/boot:	boot/boot.o tools/system
	$(LD) -Ttext 0x0 -s --oformat binary -o $@ boot/boot.o
boot/boot.o:    boot/boot.s
	$(AS) -o $@ $<
boot/boot.s:	boot/boot.S
	$(CC) $(CFLAGS) -S $<>$@

boot/head.o:    boot/head.s
	$(AS) -o $@ $<
boot/head.s:	boot/head.S
	$(CC) $(CFLAGS) -S $<>$@

driver/driver.o:
	(cd driver;make)

shell/shell.o:
	(cd shell;make)
# need to be modified
mm/mm.o:
	(cd mm;make)
fs/fs.o:
	(cd fs;make)

kernel/kernel.o:
	(cd kernel;make)

lib/lib.a:
	(cd lib;make)

clean:
	rm -f Image a.img System.map tmp_make boot/boot core
	rm -f boot/*.o tools/system System.asm
	rm -f *~ boot/*~ tools/*~ include/*~ include/asm/*~ include/cnix/*~
	rm -f .*.swp boot/.*.swp include/.*.swp include/asm/.*.swp include/cnix/.*.swp
	(cd tools; make clean)
	(cd driver;make clean)
	(cd kernel;make clean)
	(cd init;make clean)
	(cd shell;make clean) 
	(cd mm;make clean)
	(cd fs;make clean)
	(cd lib;make clean)
