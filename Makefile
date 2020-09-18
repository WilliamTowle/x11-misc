#!/usr/bin/make

HAVE_XTEST=y

CC=/usr/bin/gcc

ALL_EXES=
ifeq (${HAVE_XTEST},y)
ALL_EXES= xtest
CFLAGS_XTEST= ${CFLAGS}
LIBS_XTEST=-lX11
endif

##

PHONY: default all

default: all

all: ${ALL_EXES}

##

ifeq (${HAVE_XTEST},y)
xtest: src/xtest.c
	${CC} ${CFLAGS_XTEST} $< -o $@ ${LIBS_XTEST}
endif

clean:
	rm -f ${ALL_EXES}
