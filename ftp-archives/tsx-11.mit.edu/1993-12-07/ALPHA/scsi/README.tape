This is file contains information about the Linux SCSI tape driver
starting from Linux version 0.99.12 (older versions don't have the
variable block support).

Kai Makisara      email Kai.Makisara@vtt.fi

Last modified: Sun Sep  5 17:10:46 1993 by root@kai


TAPE BASICS AND THE LINUX SCSI TAPE MODEL

A tape drive is a simple and stupid device seen from the SCSI
programmer's point of view. Some of the current drives are quite
complex but much of the complexity comes from implementing the
simple model on a different recording scheme.

There are many different types (QIC-150, QIC-320, DAT, 8-mm,
1/2 inch reel, ...) and brands of tape drives. The optimal parameters
for each particular drive may differ in some details. Often
the operating system specifies the supported drive types and brands. It
can then set the correct parameters based on the device identification
string returned by the device. This is not practical in Linux
because we don't want to limit the support to certain drives.
The Linux SCSI tape driver is generic. It works
with default parameters with most drives. If some drive requires
special parameters, they can be set with ioctl-calls. The driver
does not alter any device parameters when opening and closing the device
file.

The tape can be seen as a sequence of files. Each file contains
a number of tape blocks that can be of equal size or the sizes
can vary within a file. Each file is ended by a filemark (end-of-
file mark, EOF, ...).

End of data on the tape can be marked in many different ways:
- two or more consecutive filemarks (used with 1/2 inch drives)
- blank tape after the last filemark (used by the QIC-streamers)
- ???

There seems to be no universally accepted number of filemarks after
the last file on the tape. The drivers usually use two for 1/2 inch
tapes (because there is no other way to tell end of recorded medium)
and one for QIC-drives. The other drive types get one or two. This
is possible if the driver knows the type and brand of the drive.
The Linux driver does not know this and we have had to choose the
number of filemarks. The choice is one.

The SCSI standard defines two different tape device modes: the
variable block size mode and the fixed block size mode. (The
mode used when writing the tape is not explicitly seen on the tape.
For instance, a tape written using variable block mode can be read
in fixed block mode if the blocks have the same size.)

The variable block mode is simple to implement. The size of a block
on tape equals to the byte count in the corresponding write command.
If we specify in a read command a byte count that does not match
the next block length, we have several choices. The Linux driver
behaves as follows: If the byte count is smaller than the block
size, the user gets what he/she wants and the rest of the block
is discarded. If the byte count is larger than the block size, the
user gets the block (the number of bytes in the block is returned
by the read call). In all cases the read head is positioned at
the beginning of the next tape block.

In fixed block mode we have two choices: either we obey the block
boundaries or we hide them. The Linux driver hides the block
boundaries. This means that any size of write and read calls can
be used without problems other than that the actual file size
is always an even multiple of blocks (this may cause problems if
compressed archives are written). The tape positioning commands
operate in block units. If these commands are used, the user
must think how the data is assembled in the block stream.

When reading a tape the user must usually know the type of the
blocks in the tape. Usually it is best to start from knowing what
you write to the tape (although writing a tape is quite easy without
knowing exactly what you are doing :-). This is especially important
if you use the tapes to transfer data between different systems.
The tape drives power-up in a mode that is specified in the firmware
of the drive. The QIC-streamers usually power-up in fixed block mode.
The DATs and Exabytes tend to power-up in variable block mode although
there are variations (even between different firmware versions of the
same drive model!) You should either know the power-up mode or set
the drive to a known mode and block size before using it.

Note also that some drives power-up with internal buffering disabled.
These stream much better if you enable drive buffering with an ioctl.


THE TAPE DEVICES

The major code for SCSI tape devices is 9. There are two different minor
numbers for each device. The minor corresponding to the number of the tape
in the kernel auto-probing is the auto-rewind tape device. This means
that the tape is rewound each time after the device file is closed. In
many cases this is not useful. The second device has the minor number that
is 128 plus the ordinal number from auto-probing. This device is the
non-rewind tape which means that the tape is left where it is when the
device is closed.

If your MAKEDEV does not support SCSI tapes, the device files corresponding
to the first SCSI tape can be made with the following commands:

mknod /dev/st0 c 9 0
mknod /dev/nst0 c 9 128

(You can, of course, choose the device file names differently if you
want to.)


IMPLEMENTATION-RELATED DETAILS (read this, too)

In order to implement the hidden block boundaries and to get the
tape drives stream better the SCSI tape driver buffers data within
the driver. The default buffer size is 32 kB for each drive recognized
at system initialization. The size of the buffer can be changed by
changing a definition (#define ST_BUFFER_BLOCKS) in the driver file
linux/kernel/blk_drv/scsi/st.c.

In addition to buffering the driver implements also write-behind.
This is usually harmless but it will delay error detection until the
next write call. Write-behind can be disabled by setting the threshold
larger or equal to the buffer size (#define ST_WRITE_THRESHOLD_BLOCKS).

In fixed block mode write commands are accepted until the buffer fills.
Write-behind is activated if the buffer is filled above a threshold
after a write call. In variable block mode only one block is handled
by the buffer at a time. This means that the largest possible block size
equals to the buffer size. A write call returns before the SCSI write
call is finished if write-behind is activated.

Buffering complicates reading of tapes in fixed block mode because
the driver can't predict the subsequent read calls from the user. This
means that the tape may be accidentally positioned past the next
filemark even if the user does not read this far. In this case the
driver backspaces the tape when closing to the location where it should
be if no buffering is used.

The fixed and variable block mode is controlled by the block size parameter
(set with ioctl MTSETBLK). A nonzero number specifies the tape block
size in bytes for fixed block mode. Giving zero sets the drive to
variable block mode (if the drive supports that).

The SCSI read and write transactions should be large to enable streaming
of the tape. In fixed block mode this is taken care of by the driver
buffering. In variable block mode the blocks should be large enough.
A 10 kB block is already large enough for many purposes.

The driver implements two devices for each tape drive recognized at
startup. The minor number equal to the number of the drive being
recognized is the so-called auto-rewind device. This means that
the tape is rewound after it is closed (i.e., it is ver difficult
to write or read more than one file per tape :-). The device with
minor number 128 plus the drive number is a non-rewind device that
is not implicitly rewound after closing.

The driver can give some amount of debugging information if the symbol
DEBUG is defined in the driver source (i.e., #define DEBUG is uncommented).
This debugging output can also be used to see the mode the drive powers
up and to see that the ioctls operate properly. When debugging is enabled
the driver writes on the console some data about the status of the drive
(current block size, tape density, internal buffer status, etc.).

THE mt PROGRAM

This archive includes the source for a simple program mt that can be used
for giving ioctls to the tape. This version originates mt from BSD NET-2.
Commands for some Linux ioctls (e.g., setting block size and buffering)
have been added to the original version. The ioctl codes (where applicable)
are same for all Linux tape devices and this mt-program can thus be used
for all tapes.


POSSIBLE PROBLEMS

PROBLEM: You always find only the latest data on the tape.

SOLUTION: You are using the auto-rewind device even if you don't want to.

PROBLEM: When reading a multifile tar the next tar after a succesful
  reading of a file returns nothing.

SOLUTION: tar knows how much it wants to read and thus it does not move
  the tape past the filemark. The next tar gets zero bytes and tar seems
  happy but silent. Now the tape moves past the filemark and the next
  tar returns data.

PROBLEM: Reading compressed archives fails at the end.

SOLUTION: The last tape block is padded with zeros when writing the last
  piece of compressed data. The decompressing programs does not like these
  added zeros at the end and complains (although all of the written data
  has already been read).

PROBLEM: You can't read a tape made with another operating system or
  another operating system can't read a tape written in Linux.

SOLUTION: If the writing and reading programs are compatible, the problem
  is probably related to the tape block sizes. You should find out the
  blocksize used when writing the tape and use a compatible method for
  reading. Using variable block size for reading is advantageous in many
  cases.

  The block size used when writing the tape in Linux can be found out
  either from the manual of the drive or from the debugging output from
  the SCSI tape driver. If the tape has been written in another system,
  possible data sources are manuals and guessing. Knowing what size of
  write calls the program used for writing the tape uses is advantageous
  (e.g., tar uses 10 kB calls by default).

  If you have a drive supporting variable blocks it is easy to make a
  program that finds out the sizes of the blocks on the tape (probably
  you can find this kind of program from some archive). The principle
  is to execute read calls with large byte count and see what byte count
  the calls return (don't use calls that exceed the driver buffer size).

PROBLEM: The tape does not stream.

SOLUTION: The drive's internal buffer may be disabled by default. You can
  try to enable the buffer with mt (mt drvbuffer 1) and see if this
  helps. You can also see the buffering status from the driver debugging
  output.

PROBLEM: Read after write does not work in tar

SOLUTION: Don't try it in fixed block mode if the block size differs from
  the blocking of tar. Tar thinks in terms of variable block length.
  (In my opinion, read after write is very good if the drive does it
  but doing it in this way by the program is usually not a good idea.
  It is better to first write the complete archive file and then to
  verify it with 'tar d'.)
