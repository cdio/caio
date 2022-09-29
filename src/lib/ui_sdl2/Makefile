#
# Copyright (C) 2020-2022 Claudio Castiglia
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
ROOT=		${abspath ../../..}

include ${ROOT}/mk/config.mk

OBJTXT=		objs.txt

CPPFLAGS+=	-I../core

CXXSRCS+=	sdl2.cpp \
		ui.cpp \
		audio.cpp \
		panel.cpp \
		widget.cpp \
		widget_empty.cpp \
		widget_floppy.cpp \
		widget_fullscreen.cpp \
		widget_gamepad.cpp \
		widget_pause.cpp \
		widget_reset.cpp \
		widget_volume.cpp

INCS+=		sdl2.hpp \
		ui.hpp \
		audio.hpp \
		panel.hpp \
		widget.hpp \
		widget_empty.hpp \
		widget_floppy.hpp \
		widget_fullscreen.hpp \
		widget_gamepad.hpp \
		widget_pause.hpp \
		widget_reset.hpp \
		widget_volume.hpp

include ${ROOT}/mk/objtxt.mk
