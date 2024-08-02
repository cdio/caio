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
ROOT=		${abspath ../..}

include ${ROOT}/mk/config.mk

BIN=		caio
CONFIG=		caio.conf

DST_BIN=	${DST_BINDIR}/${BIN}
DST_CONFIG=	${DST_SYSCONFDIR}/${CONFIG}

CXXSRCS=	configurator.cpp \
		editor.cpp \
		editor_c64.cpp \
		editor_zx80.cpp \
		editor_zxsp.cpp \
		guiapp.cpp \
		main.cpp

INCS=		configurator.hpp \
		editor.hpp \
		editor_c64.hpp \
		editor_zx80.hpp \
		editor_zxsp.hpp \
		guiapp.hpp \
		gui.hpp

EXTRA_DEPS+=	${ROOT}/src/c64/libcaio_c64.a \
		${ROOT}/src/zx80/libcaio_zx80.a \
		${ROOT}/src/zxspectrum/libcaio_zxspectrum.a \
		${ROOT}/src/ui_sdl2/libcaio_ui_sdl2.a \
		${ROOT}/src/core/libcaio_core.a \
		${ROOT}/3rdparty/sha2/libsha2.a

# Widget Toolkit
CXXSRCS+=	dearimgui.cpp
INCS+=		dearimgui.hpp
EXTRA_DEPS+=	${ROOT}/3rdparty/imgui/libimgui/libimgui.a \

CPPFLAGS+=	-I${ROOT}/src/c64 \
		-I${ROOT}/src/zx80 \
		-I${ROOT}/src/zxspectrum

LDADD+=		${EXTRA_DEPS}

include ${ROOT}/mk/bin.mk

install: ${DST_BIN} ${DST_CONFIG} ${DST_README}

${DST_BIN}: ${BIN}
	[ -d ${dir $@} ] || ${INSTALL} -d -m ${MODE_BINDIR} -o ${OWNER_BINDIR} -g ${GROUP_BINDIR} ${dir $@}
	${INSTALL} -s -m ${MODE_BIN} -o ${OWNER_BIN} -g ${GROUP_BIN} ${notdir $@} $@

${DST_CONFIG}: ${CONFIG}
	[ -d ${dir $@} ] || ${INSTALL} -d -m ${MODE_DATADIR} -o ${OWNER_DATADIR} -g ${GROUP_DATADIR} ${dir $@}
	${INSTALL} -m ${MODE_DATA} -o ${OWNER_DATA} -g ${GROUP_DATA} ${notdir $@} $@
