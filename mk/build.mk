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
# BUILD_DIR=	Required. Build directory: ${BUILD_PREFIX}/${BIN_PKGNAME}_<binary_type>
#
SRC_PKGNAME=		cemu-${RELEASE}
SRC_PKGFILE=		${BUILD_PREFIX}/${SRC_PKGNAME}.${SRC_PKGFMT}

SRC_PKGFMT=		tgz
SRC_EXTRACT_FLAGS=	-zxvf

BUILD_PREFIX=		${abspath .}/build
BUILD_DIR?=		${BUILD_PREFIX}

BUILD_SRCDIR=		${BUILD_DIR}/${SRC_PKGNAME}
BUILD_INSTALL_ROOT=	${BUILD_DIR}/root
BUILD_EXTRACT_DONE=	${BUILD_DIR}/.extract.done
BUILD_COMPILE_DONE=	${BUILD_DIR}/.compile.done
BUILD_INSTALL_DONE=	${BUILD_DIR}/.install.done
BUILD_INSTALL_DIR=	${BUILD_INSTALL_ROOT}/${PREFIX}

DISTCLEANFILES+=	${BUILD_PREFIX}

.PHONY: build package src-package

build: ${BUILD_INSTALL_DONE}

src-package package: ${SRC_PKGFILE}

${BUILD_INSTALL_DONE}: ${BUILD_COMPILE_DONE}
	${MAKE} ${MAKEARGS} -C ${BUILD_SRCDIR} PREFIX=${BUILD_INSTALL_DIR} install
	${TOUCH} $@

${BUILD_COMPILE_DONE}: ${BUILD_EXTRACT_DONE}
	${MAKE} ${MAKEARGS} -C ${BUILD_SRCDIR} PREFIX=${PREFIX} all
	${TOUCH} $@

${BUILD_EXTRACT_DONE}: ${SRC_PKGFILE} ${BUILD_DIR}
	${TAR} -C ${BUILD_DIR} ${SRC_EXTRACT_FLAGS} ${SRC_PKGFILE}
	${TOUCH} $@

${SRC_PKGFILE}: ${BUILD_PREFIX}
	cd ${ROOT} && ${GIT} archive --format=${SRC_PKGFMT} --prefix=${SRC_PKGNAME}/ ${RELEASE} > $@ || ${RM} $@

${BUILD_DIR}:
	${INSTALL} -d $@

ifneq (${BUILD_DIR}, ${BUILD_PREFIX})
${BUILD_PREFIX}:
	${INSTALL} -d $@
endif

include ${ROOT}/mk/clean.mk
