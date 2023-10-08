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

ROMS=		c64_basic.901226-01.bin \
		c64_kernal.901227-03.bin \
		c64_characters.901225-01.bin \
		zx80_rom.bin \
		zx81_rom.bin

README=		README.md

DST_ROMDIR=	${DST_DATADIR}/roms

DST_ROMS=	${ROMS:%=${DST_ROMDIR}/%}

DST_README=	${DST_ROMDIR}/${README}

.PHONY: _all all clean debug install

_all all debug:

install: ${DST_ROMS} ${DST_README}

${DST_ROMS}: ${DST_ROMDIR} ${notdir }/$@
	${INSTALL} -m ${MODE_DATA} -o ${OWNER_DATA} -g ${GROUP_DATA} ${notdir $@} $@

${DST_ROMDIR}:
	${INSTALL} -d -m ${MODE_DATADIR} -o ${OWNER_DATADIR} -g ${GROUP_DATADIR} $@

${DST_README}: ${README}
	${INSTALL} -m ${MODE_DATA} -o ${OWNER_DATA} -g ${GROUP_DATA} $< $@

clean distclean:

