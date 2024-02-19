
# Makefile for installation and configuration of LXR

# The location of your perl5 binary
PERLBIN=/usr/bin/perl
# LXR will be installed here
INSTALLPREFIX=/tmp/lxr

# End of configuration parameters
CGISCRIPTS=find ident search diff source
PERLMODULES=SimpleParse.pm Common.pm Config.pm

config: $(CGISCRIPTS) $(PERLMODULES) genxref

$(CGISCRIPTS) genxref: %: %.in
	sed s%@perlbin@%$(PERLBIN)% < $< > $@


install: config genxref
	install --directory $(INSTALLPREFIX)/http
	install --directory $(INSTALLPREFIX)/http/lib
	install --directory $(INSTALLPREFIX)/http/lib/LXR
	install --directory $(INSTALLPREFIX)/bin
	install --directory $(INSTALLPREFIX)/source
	install --mode 755 $(CGISCRIPTS) $(INSTALLPREFIX)/http/
	install --mode 750 genxref $(INSTALLPREFIX)/bin/
	install --mode 755 Common.pm Config.pm $(INSTALLPREFIX)/http/lib/LXR
	install --mode 755 SimpleParse.pm $(INSTALLPREFIX)/http/lib/
	install --mode 644 http/* $(INSTALLPREFIX)/http/

clean:
	rm -f $(CGISCRIPTS) genxref

