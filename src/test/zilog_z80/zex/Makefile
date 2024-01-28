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
ROOT=		${abspath ../../../..}

include ${ROOT}/mk/config.mk

BIN=		./z80-test

CXXSRCS=	z80_test.cpp \
		z80_test_aspace.cpp

EXTRA_DEPS+=	${ROOT}/src/core/libcaio_core.a \
		${ROOT}/3rdparty/sha2/libsha2.a

LDADD+=		${EXTRA_DEPS}

# Do not change order
TESTS=		zexdoc.com \
		zexall.com

CLEANFILES+=	${BIN}

.PHONY: all clean debug install test

all: ${BIN}

test: ${BIN}
	${BIN} -b ${TESTS}

install:

include ${ROOT}/mk/bin.mk

