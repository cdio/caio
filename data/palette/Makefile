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

PALETTES=	c64_vice-rgb.plt \
		c64_vice.plt \
		c64_rgb-pal.plt \
		c64_pepto.plt \
		c64_godot.plt \
		c64_frodo.plt \
		c64_colodore.plt \
		c64_c64-wiki.plt \
		c64_arne.plt

PALETTES+=	zx80_bw.plt \
		zx80_green.plt

DST_PALETTEDIR=	${DST_DATADIR}/palette

DST_PALETTES=	${PALETTES:%=${DST_PALETTEDIR}/%}

.PHONY: _all all clean debug install

_all all debug:

install: ${DST_PALETTES}

${DST_PALETTES}: ${DST_PALETTEDIR} ${notdir $@}
	${INSTALL} -m ${MODE_DATA} -o ${OWNER_DATA} -g ${GROUP_DATA} ${notdir $@} $@

${DST_PALETTEDIR}:
	${INSTALL} -d -m ${MODE_DATADIR} -o ${OWNER_DATADIR} -g ${GROUP_DATADIR} $@

clean distclean:

