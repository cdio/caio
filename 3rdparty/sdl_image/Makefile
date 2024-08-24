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
ROOT=		${abspath ../../}

include ${ROOT}/mk/config.mk

LIB=		${BUILDIR}/SDL_image.a

SRCDIR=		${abspath .}/SDL_image.subtree
BUILDIR=	${SRCDIR:%.subtree=%}

MAKEFILE=	${BUILDIR}/Makefile

CMAKE_FLAGS=	-DBUILD_SHARED_LIBS=NO \
		-DSDL2IMAGE_AVIF=NO \
		-DSDL2IMAGE_BMP=NO \
		-DSDL2IMAGE_GIF=NO \
		-DSDL2IMAGE_JPG=NO \
		-DSDL2IMAGE_JXL=NO \
		-DSDL2IMAGE_LBM=NO \
		-DSDL2IMAGE_PCX=NO \
		-DSDL2IMAGE_PNG=YES \
		-DSDL2IMAGE_PNM=NO \
		-DSDL2IMAGE_QOI=NO \
		-DSDL2IMAGE_SVG=NO \
		-DSDL2IMAGE_TGA=NO \
		-DSDL2IMAGE_TIF=NO \
		-DSDL2IMAGE_WEBP=NO \
		-DSDL2IMAGE_XCF=NO \
		-DSDL2IMAGE_XPM=NO \
		-DSDL2IMAGE_XV=NO

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

