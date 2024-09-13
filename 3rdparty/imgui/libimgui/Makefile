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
ROOT=		${abspath ../../../}

#DEBUG=		yes
include ${ROOT}/mk/config.mk

LIB=		libimgui.a

CXXSRCS=	imgui.cpp \
		imgui_draw.cpp \
		imgui_tables.cpp \
		imgui_widgets.cpp \
		imgui_impl_sdl2.cpp \
		imgui_impl_sdlrenderer2.cpp \
		imgui_stdlib.cpp

INCS=		imconfig.h \
		imgui.h \
		imgui_internal.h \
		imstb_rectpack.h \
		imstb_textedit.h \
		imstb_truetype.h \
		imgui_impl_sdl2.h \
		imgui_impl_sdlrenderer2.h \
		imgui_stdlib.h

CPPFLAGS+=	-DIMGUI_USE_WCHAR32

include ${ROOT}/mk/lib.mk
