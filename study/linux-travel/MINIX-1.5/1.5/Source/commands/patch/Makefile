CFLAGS=-DCHARSPRINTF			# patch for old compiler
obj = patch.s pch.s inp.s util.s version.s

patch: $(obj)
	cc -i -o patch $(obj)

patch.s: config.h common.h patch.c inp.h pch.h util.h version.h
pch.s: config.h common.h pch.c pch.h util.h
inp.s: config.h common.h inp.c inp.h util.h
util.s: config.h common.h util.c util.h
version.s: config.h common.h version.c version.h patchlevel.h util.h


clean:	
	@rm -f *.bak *.s patch
