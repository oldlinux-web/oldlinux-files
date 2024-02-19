This is the Readme file for one of the Linux SCSI tape drivers.


WARNING

You use this driver at your own risk. I am not responsible for any
direct or indirect damage caused by installation or use of this driver.


WHAT IS INCLUDED IN THE PACKAGE

README           - This file
ioctl.h          - a replacement /usr/include/sys/ioctl.h
mtio.h           - /usr/include/sys/mtio.h
mt.c             - source for mt
mt.1             - man page for mt
tapetest.c       - a simple tape test program


HISTORY

This driver has been derived from the SCSI tape driver made bye Dwayne
Forsyth in 1992. The definitions in mtio.h and ioctl.h come from the
QIC-02 tape driver by Hennus Bergman (his wise suggestions was to use
the same mtio.h definitions for all of the tape drivers).

The SCSI tape driver was first meant to be a block driver (maybe
accidentally). Because of this the files related to the driver are
in the directory kernel/blk_drv/scsi and the stub st.c initialized
the block driver structures for SCSI tape.


FEATURES
    - support for different block sizes and internal buffering
    - rewind after close and non-rewind devices
    - *nix-style ioctl with codes from mtio.h from the QIC-02 driver by
      Hennus Bergman (command MTSETBLK added)
    - improved error handling
    - character device
    - capability to handle several tape drives simultaneously
    - one buffer if one drive, two buffers if more than one drive (limits the
      number of simultaneously open drives to two)
    - write behind


DESIGN PRINCIPLES

This driver is a character driver using its own internal buffers. The
size of the buffers can be changed by changing the definition
#define ST_BUFFER_BLOCKS  64
This number specifies the buffer size in units of 512 bytes. The default
value 60 gives nice performance in my setup. The buffer size can be modified
if desired. The buffers are allocated at system initialization. One buffer is
allocated if there is one tape drive and two buffers are allocated if there
are more drives (this makes tape copies possible).

Write behind can be used by setting the definition
#define ST_WRITE_THRESHOLD_BLOCKS 60
to a value which is smaller than the buffer size. Write behind may cause
problems in end of media detection (it is detected but maybe too late).

This driver can read and write only fixed length blocks. The block size is
selectable. The driver reads the current block size from the drive when
the device is opened. This block size is used for reading and writing. This
means that the tape drive must support the mode sense-command used to read
the current block size. If a different block size is desired, it should be
set using an ioctl-command (the tape control program mt can be used at shell
level). Setting of block size is done with the mode select-command.

The internal buffer usually contains several tape blocks. This poses some
problems in reading because the last read request might not want to read
the complete buffer. This has been taken into account at the end of a file
but not in the middle of a file.

The ioctl-function is designed according to the *nix-tradition. The format
and the parameters are the same as used by Hennus Bergman in his QIC-02
tape driver. Two commands have been added: the command MTSETBLK to set the
tape block size and the command MTSETDENSITY to set the tape density.
The tape control program mt shows examples of using the ioctl.


INSTALLATION

The tape driver is now included in Eric Youngdale's new scsi driver set. The
following two things must be done in order to use the tape drive:

1. Define CONFIG_BLK_DEV_ST before compiling the kernel.

2. Create the necessary special files in the directory /dev. Here is an
example for the first scsi tape drive recognized at Linux initialization
(note that the scsi tape number is not necessarily the scsi id of the
tape drive):

cd /dev
mknod rmt0 c 9 0
mknod nrmt0 c 9 128

When you use the device /dev/rmt0 the tape is rewound after closing the
device. If you use the device /dev/nrmt0 the tape is not rewound after
closing the device. For the second tape drive use minor numbers 1 and 129,
etc..


THE TAPE CONTROL PROGRAM MT

The tape control program in this package is mt from the BSD NET2-release
with the following additions: the commands erase, setblk, and setdensity
have been added. setblk takes one argument which is the block size for the
subsequent tape requests. For example, the command to set the block size
to 1024 is:

mt -f /dev/nrmt0 setblk 1024

setdensity takes one parameter which is the density code to be sent to the
drive. The proper codes should be looked up from the documentation of the
drive.

THE PROGRAM TAPETEST

Tapetest is a simpple test program for a tape drive (or driver). It first
writes a file using transfers of random sizes and then reads the file. The
program is hard-coded to use the autorewind device /dev/rmt0h. Change the
program if you want to use another device.


POSSIBLE PROBLEMS

As mentioned before, the driver tries to maintain consistency at the
end of a file even if buffering is used. One example of this is when
a tar archive is being read. tar knows when to stop reading and it
does not read over the filemark. Prereading a buffer may cross the
filemark because the driver can't predict future. If this happens the
driver backs up over the filemark. There may be some problems with this
code. Be careful not to overwrite anything useful if you are going to
append something to a tape.

The problems with end of media detection were mentioned earlier.


MISCELLANEOUS NOTES

The driver is able to stream in my machine (a 486/33 with Adaptec AHA1542B
and Wangtek 5525ES) when doing a tar backup from an IDE disk. If tape
streaming is not as good as you would like to have, use a buffering program
between tar and the tape drive if doing backup from a scsi disk. One
commonly available buffer program is dd.


November 9, 1992 (last modified Nov 29, 1992).

Kai Makisara, email makisara@vtinsx.ins.vtt.fi or Kai.Makisara@vtt.fi
