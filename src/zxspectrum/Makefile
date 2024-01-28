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

LIB=		libcaio_zxspectrum.a

CXXSRCS=	snapshot.cpp \
		snap_z80.cpp \
		snap_sna.cpp \
		tapfile.cpp \
		ula.cpp \
		ula_aspace.cpp \
		ula_audio.cpp \
		ula_video.cpp \
		zxsp.cpp \
		zxsp_config.cpp \
		zxsp_keyboard.cpp \
		zxsp_tape.cpp

INCS=		snapshot.hpp \
		snap_sna.hpp \
		snap_z80.hpp \
		tapfile.hpp \
		ula.hpp \
		ula_aspace.hpp \
		ula_audio.hpp \
		ula_video.hpp \
		zxsp.hpp \
		zxsp_config.hpp \
		zxsp_keyboard.hpp \
		zxsp_tape.hpp

include ${ROOT}/mk/lib.mk
