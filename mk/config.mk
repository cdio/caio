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
CXX=			clang++
CUT?=			cut
DPKG?=			dpkg
DU?=			du
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
PRINTF?=		printf
RM?=			rm
SED?=			sed
TAR?=			tar
TOUCH?=			touch
UNAME?=			uname

LIBDIR=			${abspath ${PREFIX}}/lib

INCDIR=			${abspath ${PREFIX}}/include

CPPFLAGS+=		-DD_VERSION='"${VERSION}"' \
			-DD_PREFIX='"${PREFIX}"'

CPPFLAGS+=		-I${ROOT} \
			-I${ROOT}/3rdparty/GSL/include \
			-I.

CXXFLAGS+=		-Wall \
			-Werror

ifeq (${CXX}, clang++)
CXXFLAGS+=		-std=c++20
else
CXXFLAGS+=		-std=c++2a
endif

ifeq (${DEBUG}, yes)
CPPFLAGS+=		-DD_DEBUG
CXXFLAGS+=		-O0 \
			-g
ifeq (${CXX}, clang++)
CXXFLAGS+=		-fstandalone-debug
endif
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

ARCH:=			${shell ${UNAME} -m}
ifeq (${ARCH}, x86_64)
ARCH:=			amd64
endif

ifdef RELEASE
VERSION=		${RELEASE:v%=%}
else
VERSION=		0.0.0
endif

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

CLANG_MIN_VERSION_MAJOR=10
CLANG_MIN_VERSION_MINOR=0
CLANG_VERSION_MAJOR=	${shell ${CXX} --version | ${HEAD} -1 | ${SED} -e 's,.* \([0-9]*\)\.[0-9]*\.[0-9]*.*$$,\1,'}
CLANG_VERSION_MINOR=	${shell ${CXX} --version | ${HEAD} -1 | ${SED} -e 's,.* [0-9]*\.\([0-9]*\)\.[0-9]*.*$$,\1,'}

GCC_MIN_VERSION_MAJOR=	9
GCC_MIN_VERSION_MINOR=	3
GCC_VERSION_MAJOR=	${shell ${CXX} --version | ${HEAD} -1 | ${SED} -e 's,.* \([0-9]*\)\.[0-9]*\.[0-9]*$$,\1,'}
GCC_VERSION_MINOR=	${shell ${CXX} --version | ${HEAD} -1 | ${SED} -e 's,.* [0-9]*\.\([0-9]*\)\.[0-9]*$$,\1,'}
