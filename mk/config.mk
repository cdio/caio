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
MKDIR?=			${abspath ${dir ${lastword ${MAKEFILE_LIST}}}}
OS?=			${shell uname -s}
ARCH?=			${shell uname -m}
SYSDEP_MK?=		${MKDIR}/config.${OS}.mk
CWD:=			${abspath .}

include ${SYSDEP_MK}

AWK?=			awk
CAT?=			cat
CP?=			cp
CXX?=			clang++
CUT?=			cut
CMAKE?=			cmake
DIRNAME?=		dirname
DU?=			du
ECHO?=			echo
GIT?=			git
GREP?=			grep
HEAD?=			head
ID?=			id
INSTALL?=		install
override LD=		${CXX}
LN?=			ln
LNDIR?=			lndir
override MKDEP=		${CXX}
PERL?=			perl
PKG_CONFIG?=		pkg-config
PRINTF?=		printf
RM?=			rm
SDL2_CONFIG?=		sdl2-config
SED?=			sed
TAR?=			tar
TOUCH?=			touch
STRIP?=			strip
SUDO?=			sudo
UNAME?=			uname

TOOLS_DIR=		${abspath ${ROOT}}/3rdparty/tools
TOOLS_BINDIR=		${TOOLS_DIR}/bin

LIBDIR=			${abspath ${PREFIX}}/lib

INCDIR=			${abspath ${PREFIX}}/include

CPPFLAGS+=		${SYSDEP_CPPFLAGS}

CPPFLAGS+=		-DD_VERSION='"${VERSION}"' \
			-DD_PREFIX='"${PREFIX}"'

CPPFLAGS+=		-I${CWD} \
			-I${ROOT} \
			-I${ROOT}/src \
			-I${ROOT}/src/core \
			-I${ROOT}/3rdparty/sha2

CXXFLAGS+=		${UI_CXXFLAGS} \
			${SYSDEP_CXXFLAGS} \
			-std=c++20 \
			-Wall \
			-Werror \
			-Wno-unused-result

ifeq (${DEBUG}, yes)
CPPFLAGS+=		-DD_DEBUG
CXXFLAGS+=		-O0 \
			-g \
			-fstandalone-debug
else
CXXFLAGS+=		-O3
endif

ARFLAGS=		crs

LDFLAGS?=
LDFLAGS+=		${SYSDEP_LDFLAGS}

MKDEP_FLAGS=		-MMD

LN_FLAGS?=		-sf

LNDIR_FLAGS?=		-silent

VERSION?=		${shell cat ${ROOT}/VERSION.txt}

PREFIX?=		/opt/caio
DST_BINDIR?=		${PREFIX}/bin
DST_DATADIR?=		${PREFIX}/share/caio
DST_SYSCONFDIR?=	${PREFIX}/etc/caio
DST_HOMECONFDIR?=	${HOME}/.config/caio
DST_DESKTOPDIR?=	${PREFIX}/share/applications

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
