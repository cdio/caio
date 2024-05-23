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

TESTSUITE_DIR=	${ROOT}/3rdparty/tests/6502/testsuite-2.15/bin

include ${ROOT}/mk/config.mk

CAIO_BIN=	${ROOT}/src/main/caio

C64_ARGS=	c64 \
		--romdir ${ROOT}/data/roms \
		--scale 2 \
		--loglevel all \
		--unit8 ${UNIT8_DIR}

UNIT8_DIR=	./unit8

TEST_PRG=	./wload.prg

CLEANFILES+=	${UNIT8_DIR}

.PHONY: all clean debug install test

all: ${CAIO_BIN} ${UNIT8_DIR}

test: ${CAIO_BIN} ${UNIT8_DIR} ${TEST_PRG}
	${CAIO_BIN} ${C64_ARGS} ${TEST_PRG}

${CAIO_BIN}:
	${MAKE} -C ${dir $@}

${UNIT8_DIR}:
	mkdir -p $@
	find ${TESTSUITE_DIR} -type f -exec sh -c 'eval $$(printf "ln -sf %q unit8/%q.prg" "{}" "`basename \"{}\"`")' \;

install:

include ${ROOT}/mk/clean.mk
