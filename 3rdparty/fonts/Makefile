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
ROOT=		${abspath ../../}

include ${ROOT}/mk/config.mk

FONT_SRC=	UbuntuSansMono-Regular.ttf

FONT_H=		font_${subst -,,${FONT_SRC:%.ttf=%.h}}

SYMBOL_NAME=	${FONT_H:%.h=%}

FONT2C=		../imgui/font2c/font2c

#CLEANFILES+=	${FONT_H}

all debug install: ${FONT_H}

${FONT_H}: ${FONT2C} ${FONT_SRC}
	${FONT2C} -base85 ${FONT_SRC} ${SYMBOL_NAME} > $@

${FONT2C}:
	${MAKE} ${MAKEARGS} -C ${dir $@}

include ${ROOT}/mk/clean.mk
