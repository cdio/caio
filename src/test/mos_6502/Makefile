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
TESTS=		test_mos6510_arith_adc.mon \
		test_mos6510_arith_and.mon \
		test_mos6510_arith_asl.mon \
		test_mos6510_arith_bit.mon \
		test_mos6510_arith_cmp.mon \
		test_mos6510_arith_dec.mon \
		test_mos6510_arith_eor.mon \
		test_mos6510_arith_ora.mon \
		test_mos6510_arith_sbc.mon \
		test_mos6510_branch.mon \
		test_mos6510_move.mon

# TODO: illegal instructions
# TODO: ghost instructions

BIN=		../../main/c64

ROMPATH=	../../../data/roms

TEST_FLAGS=	--audio no \
		--monitor \
		--romdir ${ROMPATH} \
		--loglevel ${LOGLEVEL}

V?=		0

ifeq (${V}, 1)
LOGLEVEL=	all
STDOUT=
else
LOGLEVEL=	"none"
STDOUT=		> /dev/null
endif

.PHONY: clean install test ${TESTS}

all: test

test: ${TESTS}

${TESTS}: ${BIN}
	@echo "======================================================================"
	@echo "=> LAUNCHING TEST: $@"
	@${BIN} ${TEST_FLAGS} < $@ ${STDOUT} && { \
		echo "=> TEST FAILED: $@"; \
		echo "======================================================================"; \
		exit 1; \
	} || { \
		echo "=> TEST SUCCESS: $@"; \
		echo "======================================================================"; \
	}

${BIN}:
	${MAKE} ${MAKEARGS} -C ${dir $@} ${notdir $@}

install clean:
