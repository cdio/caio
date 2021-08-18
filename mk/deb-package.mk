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

#
# RELEASE=	Required. Git branch or tag to build.
#
ROOT=		${abspath ..}

include ${ROOT}/mk/config.mk

DEB_PKGNAME=	cemu_${VERSION}_${ARCH}
DEB_PKGFILE=	${BUILD_PREFIX}/${DEB_PKGNAME}.deb

BUILD_DIR=	${BUILD_PREFIX}/${DEB_PKGNAME}_deb

DEB_ROOT=	${BUILD_INSTALL_ROOT}
DEB_CONTROL=	${DEB_ROOT}/DEBIAN/control
DEB_SIZE=	${shell ${DU} --exclude ${DEB_CONTROL} -sk ${DEB_ROOT} | ${CUT} -f 1}

.PHONY: deb-package

include ${ROOT}/mk/build.mk

deb-package: PREFIX=/usr
deb-package: MAKEARGS+="DST_SYSCONFDIR=${DEB_ROOT}/etc/cemu"
deb-package: MAKEARGS+="EXTRA_CPPFLAGS=-DD_SYSCONFDIR='\"/etc/cemu\"'"
deb-package: ${DEB_PKGFILE}

${DEB_PKGFILE}: ${DEB_CONTROL}
	${DPKG} -b ${DEB_ROOT} $@

${DEB_CONTROL}: ${BUILD_INSTALL_DONE}
	[ -d ${dir $@} ] || ${INSTALL} -d ${dir $@}
	${PRINTF} \
	"Package: cemu\n"\
	"Version: ${VERSION}\n"\
	"Architecture: ${ARCH}\n"\
	"Priority: optional\n"\
	"Essential: no\n"\
	"Maintainer: CEMU Developers <>\n"\
	"Installed-Size: ${DEB_SIZE}\n"\
	"Depends: libsfml-graphics2.5 (>= 2.5.1), libsfml-audio2.5 (>= 2.5.1), libstdc++6 (>= 10.3.0)\n"\
	"Replaces: cemu\n"\
	"Homepage: http://github.com/cdio/CEMU\n"\
	"Description: 8 bit Home Computers Emulator.\n" > $@

