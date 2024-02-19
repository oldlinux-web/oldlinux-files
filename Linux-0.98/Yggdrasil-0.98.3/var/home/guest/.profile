cat << !!
	This is the guest account for exploring the system.  The shell
for this account is /bin/bash, the "Bourne Again SHell."  Bash is
compatible with Bourne syntax, and has job control, file completion
(with the tab key), command history and command line editting.

	You can use the "man" command to read the manual page about
different commands.  In particular, check out mv, cp, ls, and rm, and
mkdir for file manipulation, emacs for file editting, tex or groff for
typesetting, gcc and g++ for C and C++ programming, and gs for ghostscript,
a postscript-compatible page description language.

	If you want to start the X window system, here are some
caveats.  The X server uses a configuration file to determine the VGA
video chipset, screen resolutions are available, the type of mouse
where it is connected.  The default configuration file is
/usr/lib/X11/Xconfig, which, as that file is shipped on the CDROM,
expects an ET4000 video chip, and a Mouse Systems compatible serial
mouse attached to the first serial port (/dev/ttys1).  You can start
the X server in this default mode with the command "xinit", or you
select from the other Xconfig files in /usr/lib/X11 or use your own
Xconfig file like so (notice the double hyphen):
	
		xinit -- -xconfig your_xconfig_file

!!
source .bashrc
sync
