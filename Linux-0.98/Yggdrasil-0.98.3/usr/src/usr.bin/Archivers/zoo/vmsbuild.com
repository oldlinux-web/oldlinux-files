$! VMSBUILD.COM for ZOO 2.10
$!
$! Adapted from similar script for zoo 2.01 that was contributed by
$!    Steve Roseman
$!    Lehigh University Computing Center
$!    LUSGR@VAX1.CC.Lehigh.EDU
$! 
$ write sys$output "Compiling zoo..."
$ write sys$output "$ cc addbfcrc.c"
$ cc/nolist addbfcrc.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc addfname.c"
$ cc/nolist addfname.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc basename.c"
$ cc/nolist basename.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc comment.c"
$ cc/nolist comment.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc crcdefs.c"
$ cc/nolist crcdefs.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc decode.c"
$ cc/nolist decode.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc encode.c"
$ cc/nolist encode.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc getfile.c"
$ cc/nolist getfile.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc huf.c"
$ cc/nolist huf.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc io.c"
$ cc/nolist io.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc lzc.c"
$ cc/nolist lzc.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc lzd.c"
$ cc/nolist lzd.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc lzh.c"
$ cc/nolist lzh.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc machine.c"
$ cc/nolist machine.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc makelist.c"
$ cc/nolist makelist.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc maketbl.c"
$ cc/nolist maketbl.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc maketree.c"
$ cc/nolist maketree.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc misc.c"
$ cc/nolist misc.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc misc2.c"
$ cc/nolist misc2.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc needed.c"
$ cc/nolist needed.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc nextfile.c"
$ cc/nolist nextfile.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc options.c"
$ cc/nolist options.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc parse.c"
$ cc/nolist parse.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc portable.c"
$ cc/nolist portable.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc prterror.c"
$ cc/nolist prterror.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc version.c"
$ cc/nolist version.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc vmstime.c"
$ cc/nolist vmstime.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc zoo.c"
$ cc/nolist zoo.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc zooadd.c"
$ cc/nolist zooadd.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc zooadd2.c"
$ cc/nolist zooadd2.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc zoodel.c"
$ cc/nolist zoodel.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc zooext.c"
$ cc/nolist zooext.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc zoolist.c"
$ cc/nolist zoolist.c/define=(BIG_MEM,NDEBUG,VMS)
$ write sys$output "$ cc zoopack.c"
$ cc/nolist zoopack.c/define=(BIG_MEM,NDEBUG,VMS)
$
$
$ write sys$output "Linking zoo..."
$ link /executable=zoo.exe -
   addbfcrc.obj, addfname.obj, basename.obj, comment.obj,  -
   crcdefs.obj, decode.obj, encode.obj, getfile.obj, huf.obj,  -
   io.obj, lzc.obj, lzd.obj, lzh.obj, machine.obj, makelist.obj,  -
   maketbl.obj, maketree.obj, misc.obj, misc2.obj, needed.obj,  -
   nextfile.obj, options.obj, parse.obj, portable.obj, prterror.obj,  -
   version.obj, vmstime.obj, zoo.obj, zooadd.obj, zooadd2.obj,  -
   zoodel.obj, zooext.obj, zoolist.obj, zoopack.obj, -
	options/opt
$
$ write sys$output "Building fiz..."
$ cc/nolist fiz.c
$ link /executable=fiz.exe fiz.obj, addbfcrc.obj, portable.obj, -
	crcdefs.obj, options/opt
$ write sys$output "Building bilf..."
$ cc/nolist bilf.c
$ link /executable=bilf.exe bilf.obj, options/opt
$
$! delete *.obj.*
