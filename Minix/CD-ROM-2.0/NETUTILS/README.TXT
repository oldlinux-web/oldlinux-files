09/01/1996

Minixers,
The following software is contained in the .TAZ (tar.Z) files.  If you have any
comments, bug reports, wish lists, etc... they can be directed to me via email
at <temari@ix.netcom.com>.  Any updates to this software will be posted to
comp.os.minix.

Thanks,
Michael Temari
temari@ix.netcom.com


DAYTIMED.TAZ
Daytimed - A TCP server which listens on port daytime (13) and returns the
time of the local host in human readable format.  See RFC867 for details.

TIMED.TAZ
Timed - A TCP server which listens on port time (37) and returns the
time of the local host as 4 bytes (long).  See RFC868 for details.

NETDATE.TAZ
Netdate - A TCP client which connects to servers running a time server and
allows setting the time.  See netdate(1) for more details.

HTTPGET.TAZ
Httpget - A TCP client which connects to servers running the http protocol and
allows getting http documents.

HTTPD.TAZ
Httpd - A TCP server which listens on port http (80).  See the README file for
more details.

FROG.TAZ
Frog (a.k.a. Traceroute) - A program which will show the hops a packet takes to
a given destination.  If called as ping will send out an ICMP echo request
packet to determine if the given host is reachable.

READCLCK.TAZ
Readclock - A new version of the readclock located in /usr/src/commands/ibm
that if linked to setclock allows setting of the PC realtime clock.

