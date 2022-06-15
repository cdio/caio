#
# Copyright (C) 2020-2022 Claudio Castiglia
#
# This file is part of CAIO.
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

PALETTES=	vice-rgb.plt \
		vice.plt \
		rgb-pal.plt \
		pepto.plt \
		godot.plt \
		frodo.plt \
		colodore.plt \
		c64-wiki.plt \
		arne.plt

DST_PALETTEDIR=	${DST_DATADIR}/palette

DST_PALETTES=	${PALETTES:%=${DST_PALETTEDIR}/%}

.PHONY: _all all clean debug install

_all all debug:

install: ${DST_PALETTES}

${DST_PALETTES}: ${DST_PALETTEDIR} ${notdir $@}
	${INSTALL} -m ${MODE_DATA} -o ${OWNER_DATA} -g ${GROUP_DATA} ${notdir $@} $@

${DST_PALETTEDIR}:
	${INSTALL} -d -m ${MODE_DATADIR} -o ${OWNER_DATADIR} -g ${GROUP_DATADIR} $@

clean:

