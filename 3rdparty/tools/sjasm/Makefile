#
# Copyright (C) 2020-2022 Claudio Castiglia
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
ROOT=		${abspath ../../..}

include ${ROOT}/mk/config.mk

BIN=		${BINDIR}/sjasm
BINDIR?=	${abspath ..}/bin

BUILDIR=	build
SRCBIN=		${BUILDIR}/sjasm

TARGETDIR=	${abspath .}/sjasm/src

MAKEFILE=	files/Makefile

CLEANFILES=	${BUILDIR}

.PHONY: all debug

all debug: ${BIN}

${BIN}: ${SRCBIN}
	[ -d ${dir $@} ] || ${INSTALL} -m ${MODE_BINDIR} -d ${dir $@}
	${INSTALL} -m ${MODE_BIN} $< $@

${SRCBIN}: ${MAKEFILE}
	${MAKE} -C ${dir $@} all

${MAKEFILE}: ${BUILDIR}
	${INSTALL} -m ${MODE_DATA} $@ $<

${BUILDIR}:
	${INSTALL} -m ${MODE_BIN} -d $@
	cd $@ && ${LN} -sf ${TARGETDIR}/* .

include ${ROOT}/mk/clean.mk
