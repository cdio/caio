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

DIRS=		3rdparty \
		src \
		data

CLEANFILES+=	caio.app \
		caio_*

CLEANDIRS=	${DIRS}

DISTCLEANDIRS=	${DIRS}

.PHONY: distclean bundle pacakge test dtest _test

include ${ROOT}/mk/dir.mk

bundle package:
	ROOT=${ROOT} PREFIX=${PREFIX} ${MAKE} ${MAKEARGS} -f ${ROOT}/mk/package.mk $@

dtest: TARGET=debug
dtest: _test

test: TARGET=all
test: _test

_test:
	${MAKE} ${MAKEARGS} -C 3rdparty/tests
	${MAKE} ${MAKEARGS} -C src ${TARGET}
	${MAKE} ${MAKEARGS} -C src/test ${TARGET}
	${MAKE} ${MAKEARGS} -C src/test test

