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
.PHONY: all clean debug install distclean test ${DIRS}

all debug install test:
	for i in ${DIRS}; do \
		${MAKE} -C $$i ${MAKEARGS} $@ || exit 1; \
	done

${DIRS}:
	${MAKE} -C $@ ${MAKEARGS}

include ${ROOT}/mk/clean.mk
