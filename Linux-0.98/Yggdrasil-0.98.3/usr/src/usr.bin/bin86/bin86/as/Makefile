CFLAGS		=-O
LDFLAGS		=

OBJS		=as.o assemble.o error.o express.o \
		 genbin.o genlist.o genobj.o gensym.o \
		 keywords.o macro.o mops.o pops.o \
		 readsrc.o scan.o table.o typeconv.o

as: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o as
#	chmem =150000 as

as.o: const.h type.h byteord.h macro.h file.h flag.h globvar.h
assemble.o: const.h type.h address.h globvar.h opcode.h scan.h
error.o: const.h type.h
express.o: const.h type.h address.h globvar.h scan.h source.h
genbin.o: const.h type.h address.h file.h globvar.h
genlist.o: const.h type.h address.h flag.h file.h globvar.h macro.h scan.h \
	   source.h
genobj.o: const.h type.h address.h file.h globvar.h
gensym.o: const.h type.h flag.h file.h globvar.h
keywords.o: const.h type.h opcode.h
macro.o: const.h type.h globvar.h scan.h macro.h
mops.o: const.h type.h globvar.h opcode.h scan.h address.h
pops.o: const.h type.h address.h flag.h globvar.h opcode.h scan.h
readsrc.o: const.h type.h flag.h file.h globvar.h macro.h scan.h source.h
scan.o: const.h type.h scan.h
table.o: const.h type.h globvar.h scan.h
