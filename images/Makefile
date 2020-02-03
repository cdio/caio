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
ROOT=		${abspath ..}

IMAGES=		c64.gif \
		cemu.png \
		cemu.svg \
		compopicasso.png \
		gyruss.gif \
		q-bert.gif \
		zauberwald.png

README=		${ROOT}/README.md

DST_IMGDIR=	${DST_DATADIR}/images

DST_IMAGES=	${IMAGES:%=${DST_IMGDIR}/%}

DST_README=	${DST_DATADIR}/${notdir ${README}}

ALL=		${DST_IMAGES} \
		${DST_README}

include ${ROOT}/mk/config.mk

.PHONY: all clean debug install

all clean debug:

install: ${DST_IMGDIR} ${ALL}

${DST_IMAGES}: ${notdir $@}
	${INSTALL} -m ${MODE_DATA} -o ${OWNER_DATA} -g ${GROUP_DATA} ${notdir $@} $@

${DST_README}: ${README}
	${INSTALL} -m ${MODE_DATA} -o ${OWNER_DATA} -g ${GROUP_DATA} $< $@

${DST_IMGDIR}:
	${INSTALL} -d -m ${MODE_DATADIR} -o ${OWNER_DATADIR} -g ${GROUP_DATADIR} $@
