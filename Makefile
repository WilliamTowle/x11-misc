#!/usr/bin/make

HAVE_CATBOX=y
HAVE_XTEST=y

CC=/usr/bin/gcc
CFLAGS=

ifneq (${MAKE_MAKE_STRICT},)
# [1] https://invisible-island.net/scripts/sample-scripts/gcc-strict.html
CFLAGS+= -O -W \
        -Wbad-function-cast \
        -Wcast-align \
        -Wcast-qual \
        -Wmissing-declarations \
        -Wnested-externs \
        -Wpointer-arith \
        -Wwrite-strings
#CFLAGS+= -ansi \
        -pedantic
ifeq (${MAKE_MAKE_STRICT},very)
# [2] https://invisible-island.net/scripts/sample-scripts/gcc-stricter.html
# 2a. only for any version x.y.z where x={5|6}, y and z non-empty
CFLAGS+= -Wformat -Werror=format-security -fstack-protector-strong
# 2b. for all versions
CFLAGS+= -D_CONST_X_STRING -D_FORTIFY_SOURCE=2
endif
endif	# MAKE_MAKE_STRICT


ALL_EXES=

ifeq (${HAVE_CATBOX},y)
ALL_EXES+= catbox
CFLAGS_CATBOX= ${CFLAGS}
LIBS_CATBOX=-lX11
endif

ifeq (${HAVE_XTEST},y)
ALL_EXES+= xtest
CFLAGS_XTEST= ${CFLAGS}
LIBS_XTEST=-lX11
endif

##

PHONY: default all

default: all

all: ${ALL_EXES}

##

ifeq (${HAVE_CATBOX},y)
catbox: src/catbox.c
	${CC} ${CFLAGS_CATBOX} $< -o $@ ${LIBS_CATBOX}
endif


ifeq (${HAVE_XTEST},y)
xtest: src/xtest.c
	${CC} ${CFLAGS_XTEST} $< -o $@ ${LIBS_XTEST}
endif

clean:
	rm -f ${ALL_EXES}
