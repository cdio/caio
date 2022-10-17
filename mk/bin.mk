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
CXXFLAGS+=	-I${ROOT}/lib

CXXFLAGS+=	${UI_CXXFLAGS}

LDADD+=		-L${ROOT}/src/lib \
		-lcaio

LDADD+=		${UI_LDADD}

LDADD+=		-lpthread

CLEANFILES+=	${BIN} \
		${LN_BINS}

.PHONY:		_all all debug install

all:
	${MAKE} ${MAKEARGS} -j${NPROC} _all

debug:
	${MAKE} ${MAKEARGS} -j${NPROC} DEBUG=yes _all

${LIB}: ${LIBRESID}
	${MAKE} ${MAKEARGS} -j${NPROC} -C${dir $@} ${notdir $@}

${LIBRESID}:
	${MAKE} ${MAKEARGS} -j${NPROC} -C${ROOT}/3rdparty

_all: ${LN_BINS}

${LN_BINS}: ${BIN}
	${LN} -fs ${BIN} $@

include ${ROOT}/mk/obj.mk

${BIN}: ${OBJS} ${LIB}
	${LD} ${LDFLAGS} -o $@ $^ ${LDADD}
