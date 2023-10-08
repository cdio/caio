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
CXXFLAGS+=	-fPIC

CLEANFILES+=	${LIB}

.PHONY:		_all all debug

all:
	${MAKE} ${MAKEARGS} -j _all

debug:
	${MAKE} ${MAKEARGS} -j DEBUG=yes _all

install: all

_all: ${LIB}

include ${ROOT}/mk/obj.mk

${LIB}: ${OBJS}
	${AR} ${ARFLAGS} $@ $^

