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

LIB=		libcemu.a
SOLIB=		${LIB:%.a=%.so}

include ${ROOT}/mk/config.mk

#
# Core
#
CXXSRCS=	aspace.cpp \
		clock.cpp \
		config.cpp \
		confile.cpp \
		device.cpp \
		device_none.cpp \
		device_rom.cpp \
		fs.cpp \
		icon.cpp \
		keyboard.cpp \
		logger.cpp \
		mos_6510.cpp \
		mos_6510_i_arith.cpp \
		mos_6510_i_branch.cpp \
		mos_6510_i_flags.cpp \
		mos_6510_i_invalid.cpp \
		mos_6510_i_move.cpp \
		mos_6510_monitor.cpp \
		mos_6526.cpp \
		mos_6569.cpp \
		mos_6581.cpp \
		mos_6581_i.cpp \
		mos_6581_resid.cpp \
		name.cpp \
		prg.cpp \
		rgb.cpp \
		signal.cpp \
		socket.cpp \
		types.cpp \
		ui.cpp \
		ui_config.cpp \
		utils.cpp

INCS=		aspace.hpp \
		cbmbus.hpp \
		clock.hpp \
		config.hpp \
		confile.hpp \
		device.hpp \
		device_none.hpp \
		device_ram.hpp \
		device_rom.hpp \
		dispatcher.hpp \
		fs.hpp \
		gpio.hpp \
		icon.hpp \
		image.hpp \
		input_pin.hpp \
		irq_pin.hpp \
		joystick.hpp \
		keyboard.hpp \
		locked_queue.hpp \
		logger.hpp \
		mos_6510.hpp \
		mos_6510_monitor.hpp \
		mos_6526.hpp \
		mos_6569.hpp \
		mos_6581.hpp \
		mos_6581_i.hpp \
		mos_6581_resid.hpp \
		name.hpp \
		nibble_ram.hpp \
		prg.hpp \
		rgb.hpp \
		signal.hpp \
		socket.hpp \
		types.hpp \
		ui.hpp \
		ui_config.hpp \
		utils.hpp \
		version.hpp

#
# C64 Specifics
#
CXXSRCS+=	c64.cpp \
		c64_aspace.cpp \
		c64_config.cpp \
		c64_crt.cpp \
		c64_io.cpp \
		c64_joystick.cpp \
		c64_keyboard.cpp \
		c64_vic2_aspace.cpp

INCS+=		c64.hpp \
		c64_aspace.hpp \
		c64_config.hpp \
		c64_crt.hpp \
		c64_io.hpp \
		c64_joystick.hpp \
		c64_keyboard.hpp \
		c64_vic2_aspace.hpp

#
# UI Drivers
#
CXXSRCS+=	ui_sfml.cpp

INCS+=		ui_sfml.hpp

include ${ROOT}/mk/lib.mk
