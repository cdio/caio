#
# Copyright (C) 2020 Claudio Castiglia
#
# This file is part of caio.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, see http://www.gnu.org/licenses/
#
ROOT=		${abspath ../../}

include ${ROOT}/mk/config.mk

LIB=		libsha2.a

SRCS=		sha2.c
INCS=		sha2.h
OBJS=		${SRCS:%.c=%.o}
HDEPS=		${OBJS:%.o=%.d}

CPPFLAGS=	-I.

CFLAGS=		-Wall \
		-Werror \
		-fPIC \
		-O3

override CC=	clang

CLEANFILES+=	${LIB} ${OBJS} ${HDEPS}

.PHONY:		_all all debug clean install

all debug install: _all
	${MAKE} ${MAKEARGS} -j _all

_all: ${LIB}

${LIB}: ${OBJS}
	${AR} ${ARFLAGS} $@ $^

-include ${HDEPS}

%.o: %.c
	${CC} ${CPPFLAGS} ${CFLAGS} ${MKDEP_FLAGS} -c -o $@ $<

include ${ROOT}/mk/clean.mk
