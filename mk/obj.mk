#
# Copyright (C) 2020 Claudio Castiglia
#
# This file is part of CEMU.
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
OBJS=		${CXXSRCS:%.cpp=%.o}

HDEPS+=		${OBJS:%.o=%.d}

.SUFFIXES:	.cpp .o

.PHONY: check clean

-include ${HDEPS}

%.o: %.cpp
	${CXX} ${CPPFLAGS} ${CXXFLAGS} -c -o $@ $<
	${MKDEP} ${MKDEP_FLAGS} ${CPPFLAGS} ${CXXFLAGS} $< > ${@:.o=.d}

clean:
	-rm -f ${OBJS} ${HDEPS} ${CLEANFILES}

check:
ifeq (${CLANG_CHECK_VERSION},NOK)
	@printf "\n==> Minimum compiler version is ${CLANG_MIN_VERSION}, current version is ${CLANG_VERSION}\n\n"
	exit 1
else
endif
