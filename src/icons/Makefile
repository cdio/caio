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

# Application icon
SRC_ICON=	caio.png
DST_ICONDIR=	${DST_DATADIR}/icons
DST_ICON=	${DST_ICONDIR}/caio.png

# Internal icons (widgets)
ICONS_PNG=	cassette_128x24.png \
		floppy_128x2.png \
		fullscreen_128x2.png \
		gamepad_128x3.png \
		pause_128x2.png \
		reset_128.png \
		volume_128x21.png

ICONS_HPP=	${ICONS_PNG:%.png=%.hpp}

ALL=		${ICONS_HPP}

CONVERT=	./png2cpp.sh

.SUFFIXES=	.png .hpp

.PHONY: all clean debug install

all clean debug distclean:

all: ${ALL}

install: ${DST_ICON}

${DST_ICON}: ${SRC_ICON} ${DST_ICONDIR}
	${INSTALL} -m ${MODE_DATA} -o ${OWNER_DATA} -g ${GROUP_DATA} ${SRC_ICON} $@

${DST_ICONDIR}:
	${INSTALL} -d -m ${MODE_DATADIR} -o ${OWNER_DATADIR} -g ${GROUP_DATADIR} $@

%.hpp: %.png
	${CONVERT} $< > $@

