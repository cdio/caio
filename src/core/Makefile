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

LIB=		libcaio_core.a

CXXSRCS=	aspace.cpp \
		clock.cpp \
		cmdline.cpp \
		config.cpp \
		confile.cpp \
		device.cpp \
		device_none.cpp \
		ram.cpp \
		rom.cpp \
		fs.cpp \
		icon.cpp \
		joystick.cpp \
		keyboard.cpp \
		logger.cpp \
		monitor.cpp \
		readline.cpp \
		rgb.cpp \
		signal.cpp \
		types.cpp \
		ui_config.cpp \
		utils.cpp \
		version.cpp

INCS=		aspace.hpp \
		clock.hpp \
		config.hpp \
		device.hpp \
		device_none.hpp \
		ram.hpp \
		ram_bank.hpp \
		rom.hpp \
		dispatcher.hpp \
		endian.hpp \
		fs.hpp \
		gpio.hpp \
		icon.hpp \
		image.hpp \
		joystick.hpp \
		keyboard.hpp \
		latch.hpp \
		locked_queue.hpp \
		logger.hpp \
		monitor.hpp \
		name.hpp \
		nibble_ram.hpp \
		pin.hpp \
		pullup.hpp \
		readline.hpp \
		rgb.hpp \
		signal.hpp \
		types.hpp \
		ui.hpp \
		ui_config.hpp \
		utils.hpp \
		version.hpp

INCS+=		kempston.hpp

CXXSRCS+=	mos_6502.cpp \
		mos_6502_i_arith.cpp \
		mos_6502_i_branch.cpp \
		mos_6502_i_flags.cpp \
		mos_6502_i_invalid.cpp \
		mos_6502_i_move.cpp \
		mos_6510.cpp \
		mos_6526.cpp \
		mos_6569.cpp \
		mos_6581.cpp

INCS+=		mos_6502.hpp \
		mos_6510.hpp \
		mos_6526.hpp \
		mos_6569.hpp \
		mos_6581.hpp

CXXSRCS+=	ricoh_2a03.cpp \
		ricoh_2a03_apu.cpp \
		ricoh_2c02.cpp

INCS+=		ricoh_2a03.hpp \
		ricoh_2c02.hpp

CXXSRCS+=	zilog_z80.cpp \
		zilog_z80_i_arith.cpp \
		zilog_z80_i_bit.cpp \
		zilog_z80_i_branch.cpp \
		zilog_z80_i_ex.cpp \
		zilog_z80_i_flags.cpp \
		zilog_z80_i_ix.cpp \
		zilog_z80_i_ix_bit.cpp \
		zilog_z80_i_iy.cpp \
		zilog_z80_i_mi.cpp \
		zilog_z80_i_port.cpp \
		zilog_z80_i_shift.cpp \
		zilog_z80_i_move.cpp

INCS+=		zilog_z80.hpp

include ${ROOT}/mk/lib.mk
