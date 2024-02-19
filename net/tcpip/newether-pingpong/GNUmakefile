# This file must be named 'GNUmakefile'.  When it has that name it
# loaded in preference to the regular 'Makefile' (which it include
# right here) so this has the effect of appending lines to the Makefile.

include Makefile

# Add a few files to tcpip.a.
newobjs = 8390.o 3c503.o ne.o wd.o hp.o auto_irq.o
OBJS := $(OBJS) $(newobjs)
tcpip.a: $(newobjs)

# Set the address and IRQ here. The ne.c and 3c503 driver will autoprobe
# if you set the address or IRQ to zero, so we do that by default.
#
# Add -DEI_NAME="eth0" if you want to be exactly compatible with the default
# driver.  This will only work if you don't use the distributed 'we' driver!
#
ether_options :=  -DEI_NAME=\"eth0\" -DEI8390=0 -DEI8390_IRQ=0
Space.o: Space.c GNUmakefile
	$(CC) $(CPPFLAGS) $(CFLAGS) $(ether_options) -c Space.c -o $@

# Change this to define the set of ethercards your kernel will support.
8390.o: 8390.c GNUmakefile
	$(CC) $(CPPFLAGS) $(CFLAGS) -DNE2000 -DWD80x3 -DHPLAN -DEL2 -c 8390.c -o $@

# Change this to -DEL2_AUI if you use the AUI port.
3c503.o: 3c503.c 3c503reg.h GNUmakefile
	$(CC) $(CPPFLAGS) $(CFLAGS) -UEL2_AUI -c 3c503.c -o $@

# Change this to -DSHMEM=0xd0000 and -DFORCE_8BIT if you have an old
# (non-EEPROM) wd8003.
wd.o: wd.c GNUmakefile
	$(CC) $(CPPFLAGS) $(CFLAGS) -UWD_SHMEM -UFORCE_8BIT -c wd.c -o $@
