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
ROOT=		${abspath ../..}

KEYSYMS=	c64_ch.kbd \
		c64_de.kbd \
		c64_it.kbd \
		c64_gb.kbd \
		c64_vice.kbd

DST_KEYSYMSDIR=	${DST_DATADIR}/keymaps

DST_KEYSYMS=	${KEYSYMS:%=${DST_KEYSYMSDIR}/%}

.PHONY: _all all clean debug install

include ${ROOT}/mk/config.mk

_all all debug:

install: ${DST_KEYSYMS}

${DST_KEYSYMS}: ${DST_KEYSYMSDIR} ${notdir $@}
	${INSTALL} -m ${MODE_DATA} -o ${OWNER_DATA} -g ${GROUP_DATA} ${notdir $@} $@

${DST_KEYSYMSDIR}:
	${INSTALL} -d -m ${MODE_DATADIR} -o ${OWNER_DATADIR} -g ${GROUP_DATADIR} $@

clean:
