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

FONT_SRC=	DejaVuSansMNerdFontMono_Regular_stripped.ttf

ICONS_SRC=	cassette_128x23.png \
		floppy_128x2.png \
		fullscreen_128x2.png \
		gamepad_128x2.png \
		keyboard_128x2.png \
		pause_128x2.png \
		photocamera_128x1.png \
		reset_128.png \
		snapshot_128x2.png \
		volume_128x21.png

FONT_HPP=	${FONT_SRC:%.ttf=%.hpp}
ICONS_HPP=	${ICONS_SRC:%.png=%.hpp}

ALL=		${FONT_HPP} \
		${ICONS_HPP}

PNG2CPP=	./png2cpp.sh
TTF2CPP=	./ttf2cpp.sh

.SUFFIXES=	.hpp .png .ttf

.PHONY: all clean debug install

all clean debug distclean:

all debug install: ${ALL}

%.hpp: %.png
	${PNG2CPP} $< > $@

%.hpp: %.ttf
	${TTF2CPP} $< ${@:%.hpp=%} > $@

include ${ROOT}/mk/clean.mk
