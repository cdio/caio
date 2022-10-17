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
_CXXSRCS=		${abspath ${CXXSRCS}}

OBJS=			${_CXXSRCS:%.cpp=%.o}

HDEPS+=			${_CXXSRCS:%.cpp=%.d}

EXTRA_CPPFLAGS?=

CPPFLAGS+=		${EXTRA_CPPFLAGS}

CLEANFILES+=		${OBJS} \
			${HDEPS}

.SUFFIXES:		.cpp .d .o

%.o: %.cpp
	${MKDEP} ${CPPFLAGS} ${CXXFLAGS} ${MKDEP_FLAGS} $< > ${@:.o=.d}
	${CXX} ${CPPFLAGS} ${CXXFLAGS} -c -o $@ $<

-include ${HDEPS}

include ${ROOT}/mk/clean.mk
