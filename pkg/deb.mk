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
ROOT=			${abspath ..}

include ${ROOT}/mk/config.mk

BIN_PKGNAME=		cemu_${VERSION}_${ARCH}
BIN_PKGFILE=		${BUILD_PREFIX}/${BIN_PKGNAME}.deb
BUILD_DIR=		${BUILD_PREFIX}/${BIN_PKGNAME}_deb

DEB_CONTROL_SED=	DEBIAN/control.sed
DEB_CONTROL=		${BUILD_INSTALL_ROOT}/${DEB_CONTROL_SED:%.sed=%}
DEB_SIZE=		${shell ${DU} --exclude DEBIAN -sk ${BUILD_INSTALL_ROOT} | ${CUT} -f 1}

.PHONY: deb-package

include ${ROOT}/mk/build.mk

deb-package: PREFIX=/usr
deb-package: MAKEARGS+="DST_SYSCONFDIR=${BUILD_INSTALL_ROOT}/etc/cemu"
deb-package: CPPFLAGS+=-DD_SYSCONFDIR='"/etc/cemu"'
deb-package: ${BIN_PKGFILE}

${BIN_PKGFILE}: ${DEB_CONTROL}
	${DPKG} -b ${BUILD_INSTALL_ROOT} $@

${DEB_CONTROL}: ${DEB_CONTROL_SED} ${BUILD_INSTALL_DONE}
	[ -d ${dir $@} ] || ${INSTALL} -d ${dir $@}
	${SED} -e 's,@VERSION@,${VERSION},' \
	       -e 's,@SIZE@,${DEB_SIZE},' \
	       ${DEB_CONTROL_SED} > $@

