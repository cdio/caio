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
ROOT=		${abspath ..}

include ${ROOT}/mk/config.mk

IMAGES=		c64-basic.gif \
		c64-giana.gif \
		c64-gyruss.gif \
		c64-q-bert.gif \
		c64-turrican.gif \
		c64-uridium.gif \
		zx80-4K-breakout.gif \
		zx80-4K-layout.jpg \
		zx80-4K-space-invaders.gif \
		zx80-8K-kong.gif \
		zx80-8K-layout.jpg \
		zx80-8K-pacman.gif \
		zxspectrum-48k-extreme.gif \
		zxspectrum-48k-jetset-willy.gif \
		zxspectrum-48k-olli_n_lissa.gif \
		zxspectrum-48k-starquake.gif

# External
IMAGES+=	640px-ZXSpectrum48k.jpg

README=		${ROOT}/README.md

DST_IMGDIR=	${DST_DATADIR}/images

DST_IMAGES=	${IMAGES:%=${DST_IMGDIR}/%}

DST_README=	${DST_DATADIR}/${notdir ${README}}

ALL=		${DST_IMAGES} \
		${DST_README}

.PHONY: all clean debug install

all clean debug:

install: ${ALL}

${DST_IMAGES}: ${DST_IMGDIR} ${notdir $@}
	${INSTALL} -m ${MODE_DATA} -o ${OWNER_DATA} -g ${GROUP_DATA} ${notdir $@} $@

${DST_README}: ${README}
	${INSTALL} -m ${MODE_DATA} -o ${OWNER_DATA} -g ${GROUP_DATA} $< $@

${DST_IMGDIR}:
	${INSTALL} -d -m ${MODE_DATADIR} -o ${OWNER_DATADIR} -g ${GROUP_DATADIR} $@
