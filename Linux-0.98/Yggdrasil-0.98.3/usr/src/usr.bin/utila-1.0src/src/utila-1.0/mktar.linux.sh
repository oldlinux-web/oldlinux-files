#!/bin/sh
cd /usr
tar zcvf /dist/utila-1.0src.tar.Z src/utila-1.0
tar zcvf /dist/utila-1.0sbin.tar.Z src/utila-1.0/R* \
bin/banner bin/cal bin/col bin/colcrt bin/colrm bin/column \
bin/false bin/hexdump bin/od bin/strings bin/true \
bin/ul bin/uuencode bin/uudecode bin/whereis \
man/man6/banner.6 man/man1/cal.1 man/man1/col.1 man/man1/colcrt.1 \
man/man1/colrm.1 man/man1/column.1 man/man1/false.1 man/man1/hexdump.1 \
man/man1/od.1 man/man1/strings.1 man/man1/true.1 \
man/man1/ul.1 man/man1/uuencode.1 man/man5/uuencode.5 man/man1/whereis.1
