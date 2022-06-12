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
ROOT=		${abspath ../../}

include ${ROOT}/mk/config.mk

LIB=		libresid.a

CXXSRCS=	envelope.cc \
		extfilt.cc \
		filter.cc \
		pot.cc \
		sid.cc \
		version.cc \
		voice.cc \
		wave.cc

INCS=		envelope.h \
		extfilt.h \
		filter.h \
		pot.h \
		siddefs.h \
		sid.h \
		siddefs.h \
		spline.h \
		voice.h \
		wave.h

DATA=		wave6581_PST.dat \
		wave6581_PS_.dat \
		wave6581_P_T.dat \
		wave6581__ST.dat \
		wave8580_PST.dat \
		wave8580_PS_.dat \
		wave8580_P_T.dat \
		wave8580__ST.dat

DATASRCS=	${DATA:%.dat=%.cc}

CXXSRCS+=	${DATASRCS}

OBJS=		${CXXSRCS:%.cc=%.o}

HDEPS=		${OBJS:%.o=%.d}

CXXFLAGS=	-Wall \
		-Werror \
		-std=c++17 \
		-fPIC \
		-I. \
		-O3

SAMP2SRC=	${PERL} samp2src.pl

CLEANFILES+=	${LIB} ${OBJS} ${HDEPS}
#CLEANFILES+=	${LIB} ${OBJS} ${HDEPS} ${DATASRCS}

.SUFFIXES:	.cc .h .dat

.PHONY:		_all all debug clean install

all debug install: _all
	${MAKE} ${MAKEARGS} -j${NPROC} _all

_all: ${LIB}

${LIB}: ${OBJS}
	${AR} ${ARFLAGS} $@ $^

-include ${HDEPS}

%.cc: %.dat
	${SAMP2SRC} ${@:%.cc=%} $< $@

%.o: %.cc
	${CXX} ${CPPFLAGS} ${CXXFLAGS} -c -o $@ $<
	${MKDEP} ${MKDEP_FLAGS} ${CPPFLAGS} ${CXXFLAGS} $< > ${@:.o=.d}

include ${ROOT}/mk/clean.mk
