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

#
# Debian package.
#

# Naming convention: <name>_<version>-<rev>_<arch>.deb
PKG_NAME=		caio_${PKG_VERSION}_${ARCH}
PKG_FILE=		${ROOT}/${PKG_NAME}.deb
PKG_VERSION=		${VERSION}-${REV}
REV=			0

FAKE_INSTALL_DIR=	${ROOT}/${PKG_NAME}_deb
FAKE_PREFIX=		${FAKE_INSTALL_DIR}/usr
DEB_CONTROL=		${FAKE_INSTALL_DIR}/DEBIAN/control

DEB_SIZE:=		${shell ${DU} --exclude ${DEB_CONTROL} -sk ${FAKE_INSTALL_DIR} | ${CUT} -f 1}
LIBSDL2_VERSION?=	${shell ${DPKG} --list | ${AWK} '$$2 ~ /libsdl2-2/ { sub("+.*", "", $$3); print $$3 }'}
LIBSTDCPP_VERSION?=	${shell ${DPKG} --list | ${AWK} '$$2 ~ /libstdc\+\+6/ { sub("-.*", "", $$3); print $$3 }'}
FREETYPE_VERSION?=	${shell ${DPKG} --list | ${AWK} '$$2 ~ /libfreetype6/ { sub("+.*", "", $$3); print $$3 }'}

DPKG?=			dpkg

.PHONY: all bundle install package

package: ${PKG_FILE}

${PKG_FILE}: bundle
	${SUDO} chown -R root:root ${FAKE_INSTALL_DIR}
	${SUDO} ${DPKG} -b ${FAKE_INSTALL_DIR} $@

bundle: ${DEB_CONTROL}

${DEB_CONTROL}: install ${ROOT}/mk/deb-package.mk
	[ -d ${dir $@} ] || ${INSTALL} -d -m 0755 ${dir $@}
	${PRINTF} \
	"Package: caio\n"\
	"Version: ${PKG_VERSION}\n"\
	"Architecture: ${ARCH}\n"\
	"Maintainer: caio developers\n"\
	"Installed-Size: ${DEB_SIZE}\n"\
	"Depends: libsdl2-2.0-0 (>= ${LIBSDL2_VERSION}), libfreetype6 (>= ${FREETYPE_VERSION}), libstdc++6 (>= ${LIBSTDCPP_VERSION})\n"\
	"Replaces: caio\n"\
	"Section: games\n"\
	"Priority: optional\n"\
	"Essential: no\n"\
	"Homepage: http://github.com/cdio/caio\n"\
	"Description: 8-bit home computers emulator\n"\
	" caio is an emulator of hardware platforms with a\n"\
	" strong focus on old 8-bit home computers.\n" > $@

install: all
	PREFIX=${FAKE_PREFIX} \
	VERSION=${VERSION} \
	${MAKE} ${MAKEARGS} $@

all:
	EXTRA_CPPFLAGS='${EXTRA_CPPFLAGS} -DD_SYSCONFDIR=\"/etc/caio\"' \
	PREFIX="/usr" \
	VERSION=${VERSION} \
	${MAKE} ${MAKEARGS} $@

