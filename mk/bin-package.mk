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

#
# RELEASE=	Required. Git branch or tag to build.
#
ROOT=		${abspath ..}

include ${ROOT}/mk/config.mk

BIN_PKGNAME=	caio_${VERSION}_${ARCH}
BIN_PKGFILE=	${BUILD_PREFIX}/${BIN_PKGNAME}.tgz

BUILD_DIR=	${BUILD_PREFIX}/${BIN_PKGNAME}_tgz

.PHONY: bin-package

include ${ROOT}/mk/build.mk

bin-package: ${BIN_PKGFILE}

${BIN_PKGFILE}: ${BUILD_INSTALL_DONE}
	${TAR} -zcf $@ -C ${BUILD_INSTALL_ROOT} .

