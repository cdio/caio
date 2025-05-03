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
ifeq (${ARCH}, arm64)
HOMEBREW_DIR=		/opt/homebrew
else
HOMEBREW_DIR=		/usr/local/Homebrew
CFLAGS+=		-arch x86_64
CXXFLAGS+=		-arch x86_64
CXXFLAGS+=		-Wno-nullability-completeness
LDFLAGS+=		-arch x86_64
endif

override SDL2_CONFIG=	${HOMEBREW_DIR}/bin/sdl2-config

SYSDEP_CPPFLAGS+=	-I/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include

LIB_INCLUDE_BEGIN=	-Wl,-all_load
LIB_INCLUDE_END=

UI_CXXFLAGS+=		-I${ROOT}/3rdparty/sdl2/sdl_image/SDL_image.subtree/include \
			-I${ROOT}/3rdparty/sdl2/sdl_ttf/SDL_ttf.subtree

UI_CXXFLAGS+=		${shell ${PKG_CONFIG} --cflags freetype2 libpng} \
			${shell ${SDL2_CONFIG} --cflags}

UI_LDADD+=		${ROOT}/3rdparty/sdl2/sdl_image/SDL_image/libSDL2_image.a \
			${ROOT}/3rdparty/sdl2/sdl_ttf/SDL_ttf/libSDL2_ttf.a \
			${HOMEBREW_DIR}/opt/freetype/lib/libfreetype.a \
			${HOMEBREW_DIR}/opt/libpng/lib/libpng.a

UI_LDADD+=		${shell ${SDL2_CONFIG} --static-libs}

UI_LDADD+=		-lbz2 \
			-lz
