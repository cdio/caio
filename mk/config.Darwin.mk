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
NPROC:=			${shell sysctl -n hw.ncpu}

EXTRA_3RDPARTY_DIRS+=

SYSDEP_CPPFLAGS+=	-I/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include

# OpenAL 3D-Audio framework (required by SFML) provided by MACOS
#SFML_CXXFLAGS+=		${shell ${PKG_CONFIG} --cflags sfml-graphics sfml-audio}
SFML_CXXFLAGS+=		-I/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks/OpenAL.framework/Headers/
SFML_CXXFLAGS+=		${shell ${PKG_CONFIG} --cflags sfml-graphics}

SFML_LDADD+=		${shell ${PKG_CONFIG} --libs sfml-graphics sfml-audio}
