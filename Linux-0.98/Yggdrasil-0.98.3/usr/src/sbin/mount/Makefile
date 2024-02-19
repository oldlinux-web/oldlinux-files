# To make "ext" the default file system type for mount
#  (used when no other type is specified), replace \"minix\" by \"ext\".
DEFAULT_FSTYPE=\"minix\"

# kernels prior to 0.96c-pl1 use a 4 arg mount(2) and don't have msdos or ext
#DEFINES =

# 0.96c-pl1 and later kernels use a 5 arg mount(2) and have msdos and ext fs
#DEFINES = -DHAVE_MOUNT5
# 0.97-pl3 and later kernels also have swapoff(2)
DEFINES = -DHAVE_MOUNT5 -DHAVE_SWAPOFF

# For debugging, uncomment -g in DEBUG and comment out -N in LDFLAGS.
# If you prefer static binaries, add -static to the LDFLAGS.

# debug link options
#DEBUG = -g
#LDFLAGS = $(DEBUG)

# static link options
#LDFLAGS = -N -s -static

# my favorite link options
LDFLAGS = -N -s

CC = gcc -pipe -O2 -Wall -Wstrict-prototypes -Wmissing-prototypes
CFLAGS = $(DEBUG) -I. -DFSTYPE_DEFAULT=$(DEFAULT_FSTYPE) $(DEFINES)

PROGS = mount umount swapon
MAN5 = fstab.5
MAN8 = mount.8 swapon.8

all: $(PROGS)

install: $(PROGS)
	install -s $(PROGS) /sbin
	install -d /usr/man/cat5
	install -d /usr/man/cat8
	install $(MAN5) /usr/man/cat5
	install $(MAN8) /usr/man/cat8
	(cd /usr/man/cat8 ; ln -fs mount.8 umount.8)
# comment out the two lines below if your kernel version is < 0.97-pl3
	ln -f /sbin/swapon /sbin/swapoff
	(cd /usr/man/cat8 ; ln -fs swapon.8 swapoff.8)

mount: mount.o sundries.o fstab.o

umount: umount.o sundries.o fstab.o

swapon:	swapon.o fstab.o

clean:
	rm -f a.out core *~ *.o $(PROGS)

