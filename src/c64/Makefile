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

LIB=		libcaio_c64.a

#
# Commodore Specifics
#
CXXSRCS+=	cbm_bus.cpp \
		c1541.cpp \
		c1541_factory.cpp \
		c1541_fs.cpp

INCS+=		cbm_bus.hpp \
		c1541.hpp \
		c1541_factory.hpp \
		c1541_fs.hpp

#
# C64 Specifics
#
CXXSRCS+=	c64.cpp \
		c64_bus_controller.cpp \
		c64_cartridge.cpp \
		c64_config.cpp \
		c64_crt.cpp \
		c64_io.cpp \
		c64_joystick.cpp \
		c64_keyboard.cpp \
		c64_pla.cpp \
		c64_vic2_aspace.cpp

INCS+=		c64.hpp \
		c64_bus_controller.hpp \
		c64_cartridge.hpp \
		c64_config.hpp \
		c64_crt.hpp \
		c64_io.hpp \
		c64_joystick.hpp \
		c64_keyboard.hpp \
		c64_pla.hpp \
		c64_vic2_aspace.hpp

CXXSRCS+=	c64_cartridge/cart_generic.cpp \
		c64_cartridge/cart_simons_basic.cpp \
		c64_cartridge/cart_ocean_type_1.cpp \
		c64_cartridge/cart_easy_flash.cpp \
		c64_cartridge/cart_magic_desk.cpp \
		c64_cartridge/cart_c64_game_system_3.cpp

INCS+=		c64_cartridge/cart_generic.hpp \
		c64_cartridge/cart_simons_basic.hpp \
		c64_cartridge/cart_ocean_type_1.hpp \
		c64_cartridge/cart_easy_flash.hpp \
		c64_cartridge/cart_magic_desk.hpp \
		c64_cartridge/cart_c64_game_system_3.cpp

include ${ROOT}/mk/lib.mk
