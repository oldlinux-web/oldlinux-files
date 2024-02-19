# This has the effect of appending lines to the Makefile.
include Makefile

# Add a few files to tcpip.a.
newobjs = 8390.o 3c503.o ne.o wd.o hp.o auto_irq.o
OBJS := $(OBJS) $(newobjs)
tcpip.a: $(newobjs)

# Set the address and IRQ here. The ne.c and 3c503 driver will autoprobe
# if you set the address or IRQ to zero, so we do that by default.
#
# Add -DEI_NAME=eth0 if you want to be exactly compatible with the default
# driver.  This will only work if you don't use the distributed 'we' driver!
#
Space.o: Space.c GNUmakefile
	$(CC) $(CPPFLAGS) $(CFLAGS) -DEI8390=0 -DEI8390_IRQ=0 -c Space.c -o $@

# Change this to define the set of ethercards your kernel will support.
8390.o: 8390.c GNUmakefile
	$(CC) $(CPPFLAGS) $(CFLAGS) -DNE2000 -DWD80x3 -DHPLAN -DEL2 -c 8390.c -o $@

# And set any special compile-time options here.
ne.o: ne.c GNUmakefile
	$(CC) $(CPPFLAGS) $(CFLAGS) -UEI_8BIT -c ne.c -o $@

3c503.o: 3c503.c 3c503reg.h GNUmakefile
	$(CC) $(CPPFLAGS) $(CFLAGS) -UEI8390_THICK -c 3c503.c -o $@
