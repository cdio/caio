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
ifeq (${ARCH}, x86_64)
ARCH=			amd64
endif

LNDIR=			${TOOLS_BINDIR}/lndir

SYSDEP_CPPFLAGS+=

LIB_INCLUDE_BEGIN=	-Wl,--whole-archive
LIB_INCLUDE_END=	-Wl,--no-whole-archive

UI_CXXFLAGS+=		-I${ROOT}/3rdparty/sdl2/sdl_image/SDL_image.subtree/include \
			-I${ROOT}/3rdparty/sdl2/sdl_ttf/SDL_ttf.subtree

UI_CXXFLAGS+=		${shell ${PKG_CONFIG} --cflags freetype2} \
			${shell ${SDL2_CONFIG} --cflags}

UI_LDADD+=		${ROOT}/3rdparty/sdl2/sdl_image/SDL_image/libSDL2_image.a \
			${ROOT}/3rdparty/sdl2/sdl_ttf/SDL_ttf/libSDL2_ttf.a \
			${ROOT}/3rdparty/sdl2/sdl_ttf/SDL_ttf/libSDL2_ttf.a \
			${shell ${PKG_CONFIG} --libs freetype2}

UI_LDADD+=		${shell ${SDL2_CONFIG} --libs}

override AWK=		gawk
