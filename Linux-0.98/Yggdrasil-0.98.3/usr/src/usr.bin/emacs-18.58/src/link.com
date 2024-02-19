$ ! VMS command file to link the .OBJ files of Emacs,
$ ! using `temacs.opt' and producing `temacs.exe'.
$ if "''CC'".eqs."" then goto link_cc
$ if  f$extract(0,3,cc).eqs."GCC" then goto link_gcc
$ link temacs/opt
$ exit
$link_gcc:
$ link temacs/opt+gnu_cc:[000000]gcclib/lib
