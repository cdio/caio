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
ROOT=		${abspath ../../..}

include ${ROOT}/mk/config.mk

LIB=		${BUILDIR}/libSDL2_ttf.a

SRCDIR=		${abspath .}/SDL_ttf.subtree
BUILDIR=	${SRCDIR:%.subtree=%}

MAKEFILE=	${BUILDIR}/Makefile

CMAKE_FLAGS+=	-DBUILD_SHARED_LIBS=OFF \
		-DSDL2TTF_SAMPLES=OFF

DISTCLEANFILES=	${BUILDIR}

.PHONY: all debug clean distclean install

all debug install: ${LIB}

${LIB}: ${MAKEFILE}
	${MAKE} ${MAKEARGS} -j -C ${dir $@} all

${MAKEFILE}: ${BUILDIR}
	cd $<; ${CMAKE} ${CMAKE_FLAGS} ${SRCDIR}

${BUILDIR}:
	[ -d $@ ] || ${INSTALL} -d -m 0755 $@

clean:

distclean:
	-${RM} -rf ${BUILDIR}

