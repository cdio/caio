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
ROOT=		${abspath ../../../../}

include ${ROOT}/mk/config.mk

BIN=		./6502-test

CXXSRCS=	mos_6502_test.cpp \
		mos_6502_test_aspace.cpp

EXTRA_DEPS+=	${ROOT}/src/core/libcaio_core.a \
		${ROOT}/3rdparty/sha2/libsha2.a

LDADD+=		${EXTRA_DEPS}

TEST=		funct.bin
TEST_ASM=	${TEST:%.bin=%.a65}
TEST_DEP=	report.i65

DOSBOX=		dosbox
UNZIP=		unzip
AS65=		AS65-DOS.EXE
AS65_ZIP=	${ROOT}/3rdparty/tests/6502/6502_65C02_functional_tests/as65_142.zip

CLEANFILES+=	${BIN} \
		${AS65} \
		*.BIN \
		*.LST

.PHONY: all clean debug install test

all: ${BIN} ${TEST}

test: ${BIN}
	${BIN} -b ${TEST}

install:

${TEST}: ${TEST_ASM} ${TEST_DEP} ${AS65}
	${DOSBOX} compile.bat

${AS65}:
	${UNZIP} -x ${AS65_ZIP} $@

include ${ROOT}/mk/bin.mk

