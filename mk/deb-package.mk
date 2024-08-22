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
PKGNAME=		caio_${VERSION}-${ARCH}
PKGFILE=		${ROOT}/${PKGNAME}.deb

FAKE_INSTALL_DIR=	${ROOT}/${PKGNAME}_deb
FAKE_PREFIX=		${FAKE_INSTALL_DIR}/usr
DEB_CONTROL=		${FAKE_INSTALL_DIR}/DEBIAN/control
DEB_SIZE=		${shell ${DU} --exclude ${DEB_CONTROL} -sk ${FAKE_INSTALL_DIR} | ${CUT} -f 1}

DPKG?=			dpkg

.PHONY: all bundle install package

package: ${PKGFILE}

${PKGFILE}: bundle
	${DPKG} -b ${FAKE_INSTALL_DIR} $@

bundle: ${DEB_CONTROL}

${DEB_CONTROL}: install
	[ -d ${dir $@} ] || ${INSTALL} -d -m 0755 ${dir $@}
	${PRINTF} \
	"Package: caio\n"\
	"Version: ${VERSION}\n"\
	"Architecture: ${ARCH}\n"\
	"Priority: optional\n"\
	"Essential: no\n"\
	"Maintainer: caio developers\n"\
	"Installed-Size: ${DEB_SIZE}\n"\
	"Depends: libsdl2-2.0-0 (>= 2.0.0), libsdl2-image-2.0-0 (>= 2.0.0), libstdc++6 (>= 10.3.0)\n"\
	"Replaces: caio\n"\
	"Homepage: http://github.com/cdio/caio\n"\
	"Description: 8-bit Home Computers Emulator.\n" > $@

install: all
	PREFIX=${FAKE_PREFIX} \
	VERSION=${VERSION} \
	${MAKE} ${MAKEARGS} $@

all:
	EXTRA_CPPFLAGS=-DD_SYSCONFDIR='\"/etc/caio\"' \
	PREFIX="/usr" \
	VERSION=${VERSION} \
	${MAKE} ${MAKEARGS} $@

