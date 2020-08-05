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
VERSION:=		${shell cat ${ROOT}/VERSION}

LIBDIR=			${abspath ${PREFIX}}/lib

INCDIR=			${abspath ${PREFIX}}/include

CPPFLAGS=		-DD_VERSION='"${VERSION}"' \
			-DD_PREFIX='"${PREFIX}"'

CPPFLAGS+=		-I${ROOT}/3rdparty/GSL/include \
			-I.

CXXFLAGS=		-Wall \
			-Werror \
			-std=c++17

ifeq (${DEBUG}, yes)
CPPFLAGS+=		-DD_DEBUG
CXXFLAGS+=		-O0 \
			-g \
			-fstandalone-debug
else
CXXFLAGS+=		-O3
LDFLAGS+=		-Wl,-s
endif

ARFLAGS=		crs

LDFLAGS?=

MKDEP_FLAGS=		-MM \
			-MG

LN_FLAGS?=		-sf

NPROC:=			${shell ${NPROC}}

override CXX=		clang++
GIT?=			git
HEAD?=			head
ID?=			id
INSTALL?=		install
override LD=		${CXX}
LN?=			ln
override MKDEP=		${CXX}
NPROC?=			nproc
PERL?=			perl
PKG_CONFIG?=		pkg-config
SED?=			sed

PREFIX?=		/opt/cemu
DST_BINDIR?=		${PREFIX}/bin
DST_DATADIR?=		${PREFIX}/share/cemu
DST_SYSCONFDIR?=	${PREFIX}/etc/cemu
DST_HOMECONFDIR?=	${HOME}/.config/cemu

OWNER:=			${shell ${ID} -u}
GROUP:=			${shell ${ID} -g}

OWNER_BINDIR?=		${OWNER}
GROUP_BINDIR?=		${GROUP}

OWNER_DATADIR?=		${OWNER}
GROUP_DATADIR?=		${GROUP}

OWNER_BIN?=		${OWNER}
GROUP_BIN?=		${GROUP}

OWNER_DATA?=		${OWNER}
GROUP_DATA?=		${GROUP}

MODE_BINDIR?=		0755
MODE_DATADIR?=		0755

MODE_BIN?=		0755
MODE_DATA?=		0644

HOME:=			${shell echo ~}

CLANG_MIN_VERSION:=	6
CLANG_VERSION:=		${shell ${CXX} --version | ${HEAD} -1 | ${SED} -e 's,.* \([0-9]*\)\.[0-9]*\.[0-9]*.*$$,\1,'}
CLANG_CHECK_VERSION:=	${shell test ${CLANG_VERSION} -ge ${CLANG_MIN_VERSION} && echo OK || echo NOK}
