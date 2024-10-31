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

LIB=		libcaio_nes.a

CXXSRCS+=	ines.cpp \
		nes.cpp \
		nes_aspace.cpp \
		nes_cartridge.cpp \
		nes_config.cpp \
		nes_joystick.cpp \
		nes_keyboard.cpp \
		nes_ppu_aspace.cpp \

INCS+=		ines.hpp \
		nes.hpp \
		nes_aspace.hpp \
		nes_cartridge.hpp \
		nes_config.hpp \
		nes_joystick.hpp \
		nes_keyboard.hpp \
		nes_params.hpp \
		nes_ppu_aspace.hpp

CXXSRCS+=	mapper_000.cpp \
		mapper_001.cpp \
		mapper_002.cpp

INCS+=		mapper_000.hpp \
		mapper_001.hpp \
		mapper_002.hpp

include ${ROOT}/mk/lib.mk
