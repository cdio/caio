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

CLEANFILES+=	${OBJS} \
		${HDEPS}

.SUFFIXES:	.cpp .o

.PHONY: check

-include ${HDEPS}

%.o: %.cpp
	${CXX} ${CPPFLAGS} ${CXXFLAGS} -c -o $@ $<
	${MKDEP} ${MKDEP_FLAGS} ${CPPFLAGS} ${CXXFLAGS} $< > ${@:.o=.d}

ifeq (${CXX}, clang++)
_CXX_MIN_VERSION_MAJOR:=${CLANG_MIN_VERSION_MAJOR}
_CXX_MIN_VERSION_MINOR:=${CLANG_MIN_VERSION_MINOR}
_CXX_VERSION_MAJOR:=	${CLANG_VERSION_MAJOR}
_CXX_VERSION_MINOR:=	${CLANG_VERSION_MINOR}
else
ifeq (${CXX}, g++)
_CXX_MIN_VERSION_MAJOR:=${GCC_MIN_VERSION_MAJOR}
_CXX_MIN_VERSION_MINOR:=${GCC_MIN_VERSION_MINOR}
_CXX_VERSION_MAJOR:=	${GCC_VERSION_MAJOR}
_CXX_VERSION_MINOR:=	${GCC_VERSION_MINOR}
endif
endif

check:
ifndef _CXX_VERSION_MAJOR
	@echo "\n==> Compiler not recognised. Bypassing version check...\n\n"
else
	@test \( ${_CXX_VERSION_MAJOR} -gt ${_CXX_MIN_VERSION_MAJOR} \) -o \
	      \( ${_CXX_VERSION_MAJOR} -eq ${_CXX_MIN_VERSION_MAJOR} -a ${_CXX_VERSION_MINOR} -ge ${_CXX_MIN_VERSION_MINOR} \) || \
	    echo "\n==> Minimum ${CXX} compiler version is ${_CXX_MIN_VERSION_MAJOR}.${_CXX_MIN_VERSION_MINOR}." \
	         "Current version is ${_CXX_VERSION_MAJOR}.${_CXX_VERSION_MINOR}\n\n"
endif

include ${ROOT}/mk/clean.mk
