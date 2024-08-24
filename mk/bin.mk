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
CXXFLAGS+=	-I${ROOT}/lib

CLEANFILES+=	${BIN} \
		${LN_BINS}

.PHONY:		_all all debug install

all:
	${MAKE} ${MAKEARGS} -j _all

debug:
	${MAKE} ${MAKEARGS} -j DEBUG=yes _all

${EXTRA_DEPS}:
	${MAKE} ${MAKEARGS} -j -C${dir $@} ${notdir $@}

_all: ${BIN}

include ${ROOT}/mk/obj.mk

${BIN}: ${OBJS} ${EXTRA_DEPS}
	${LD} ${LDFLAGS} -o $@ ${OBJS} ${LDADD}
ifneq (${DEBUG}, yes)
	${STRIP} $@
endif
