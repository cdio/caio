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
ROOT=		${abspath .}

include ${ROOT}/mk/config.mk

DEBUG?=

BUILD_DIR=	${ROOT}/build

ifeq (${DEBUG}, yes)
BUILD_ROOT=	${BUILD_DIR}/${OS}_${ARCH}_debug
else
BUILD_ROOT=	${BUILD_DIR}/${OS}_${ARCH}
endif

LNDIRS=		3rdparty \
		data \
		mk \
		src

DST_LNDIRS=	${LNDIRS:%=${BUILD_ROOT}/%}
DST_MAKEFILE=	${BUILD_ROOT}/Makefile
DST_VERSION=	${BUILD_ROOT}/VERSION.txt

DISTCLEANFILES=	${BUILD_DIR}

.PHONY: all build bundle clean debug distclean install package ${DST_LNDIRS}

all bundle install clean package: build
	${MAKE} ${MAKEARGS} -C ${BUILD_ROOT} $@

debug:
	${MAKE} ${MAKEARGS} DEBUG=yes all

build: ${DST_LNDIRS} ${DST_MAKEFILE} ${DST_VERSION}

${DST_MAKEFILE}:
	${LN} ${LN_FLAGS} ${ROOT}/mk/root.mk $@

${DST_VERSION}:
	${LN} ${LN_FLAGS} ${ROOT}/${notdir $@} $@

# Linux lacks lndir(1)
ifeq (${OS}, Linux)
${LNDIR}:
	${MAKE} ${MAKEARGS} -C 3rdparty/tools/lndir install

${DST_LNDIRS}: ${LNDIR}
else
${DST_LNDIRS}:
endif
	${INSTALL} -d -m 0755 $@
	srcdir=`echo $@ | ${SED} -e 's,^${BUILD_ROOT},${ROOT},'`; ${LNDIR} ${LNDIR_FLAGS} $$srcdir $@

distclean:
	${RM} -rf ${DISTCLEANFILES}
	for d in ${LNDIRS}; do ${MAKE} ${MAKEARGS} -C $$d distclean; done

