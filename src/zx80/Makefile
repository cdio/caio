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

LIB=		libcaio_zx80.a

#
# Sinclair ZX80 Specifics
#
CXXSRCS=	zx80.cpp \
		zx80_aspace.cpp \
		zx80_config.cpp \
		zx80_keyboard.cpp \
		zx80_video.cpp

INCS=		zx80.hpp \
		zx80_aspace.hpp \
		zx80_config.hpp \
		zx80_keyboard.hpp \
		zx80_video.hpp

include ${ROOT}/mk/lib.mk
