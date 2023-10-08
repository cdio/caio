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
LDFLAGS+=	-shared

LDADD+=		${UI_LDADD}

CLEANFILES+=	${SHLIB}

.PHONY:		_all all debug ${LIBS}

all: TARGET=all
all: _all

debug: TARGET=debug
debug: _all

install: all

_all: ${SHLIB}

${SHLIB}: ${LIBS}
	${LD} ${LDFLAGS} -o $@ ${LIB_INCLUDE_BEGIN} $^ ${LIB_INCLUDE_END} ${LDADD}

${LIBS}:
	${MAKE} ${MAKEARGS} -C ${dir $@} ${TARGET}

include ${ROOT}/mk/clean.mk
