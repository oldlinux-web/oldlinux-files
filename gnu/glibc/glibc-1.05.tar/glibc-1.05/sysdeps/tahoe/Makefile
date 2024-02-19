ifeq	($(subdir),math)
ifndef	math-twiddled

routines:= $(filter-out acos asin cos sin ceil rint hypot \
			__copysign __scalb __drem __logb __finite,$(routines))
aux	:= $(aux) support exp__E log__L

math-twiddled := t

endif

bsdmath_dirs := $(bsdmath_dirs) tahoe

endif
