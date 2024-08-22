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
# MacOS application bundle and packages .pkg and .dmg.
#
PKGNAME=		caio_${VERSION}-${ARCH}
PKGFILE=		${ROOT}/${PKGNAME}.pkg
DMGFILE=		${ROOT}/${PKGNAME}.dmg
DMGDIR=			${ROOT}/${PKGNAME}

BUNDLE_NAME=		caio.app
BUNDLE_DIR=		${ROOT}/${BUNDLE_NAME}
BUNDLE_CONTENTS_DIR=	${BUNDLE_DIR}/Contents
BUNDLE_BINDIR=		${BUNDLE_CONTENTS_DIR}/MacOS
BUNDLE_RESOURCES_DIR=	${BUNDLE_CONTENTS_DIR}/Resources
BUNDLE_ICON_RELATIVE=	icons/caio.png
BUNDLE_INFO_PLIST=	${BUNDLE_CONTENTS_DIR}/Info.plist

INSTALL_DIR=		/Applications
RUNTIME_RESOURCES_DIR=	${BUNDLE_RESOURCES_DIR:${ROOT}/%=${INSTALL_DIR}/%}

CLEANFILES+=		${PACKAGES} \
			${BUNDLE_DIR} \
			${PKGNAME}

MK_PACKAGE?=		productbuild
MK_IMAGE?=		hdiutil

.PHONY: all bundle install package

package: ${DMGFILE} ${PKGFILE}

${DMGFILE}: bundle
	[ -d ${DMGDIR} ] || ${INSTALL} -d -m 0755 ${DMGDIR}
	${CP} -Rp ${BUNDLE_DIR} ${DMGDIR}
	cd ${DMGDIR} && ${LN} -sf ${INSTALL_DIR}
	${MK_IMAGE} create -srcfolder ${DMGDIR} -format UDZO $@

${PKGFILE}: bundle
	${MK_PACKAGE} --component ${BUNDLE_DIR} ${INSTALL_DIR} $@

bundle: install ${BUNDLE_INFO_PLIST}

install: all
	DST_BINDIR="${BUNDLE_BINDIR}" \
	DST_DATADIR="${BUNDLE_RESOURCES_DIR}" \
	DST_SYSCONFDIR="${BUNDLE_RESOURCES_DIR}" \
	PREFIX="${BUNDLE_RESOURCES_DIR}" \
	VERSION=${VERSION} \
	${MAKE} ${MAKEARGS} $@

all:
	EXTRA_CPPFLAGS='-DD_SYSCONFDIR=\"${RUNTIME_RESOURCES_DIR}\" -DD_DATADIR=\"${RUNTIME_RESOURCES_DIR}\"' \
	PREFIX="${RUNTIME_RESOURCES_DIR}" \
	VERSION=${VERSION} \
	${MAKE} ${MAKEARGS} $@

${BUNDLE_INFO_PLIST}: ${dir $@}
	${PRINTF} \
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"\
	"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"\
	"<plist version=\"1.0\">\n"\
	"<dict>\n"\
	"	<key>CFBundleName</key>\n"\
	"	<string>caio</string>\n"\
	"	<key>CFBundleDisplayName</key>\n"\
	"	<string>caio emulator</string>\n"\
	"	<key>CFBundleIdentifier</key>\n"\
	"	<string>caio</string>\n"\
	"	<key>CFBundleVersion</key>\n"\
	"	<string>${VERSION}</string>\n"\
	"	<key>CFBundlePackageType</key>\n"\
	"	<string>APPL</string>\n"\
	"	<key>CFBundleSignature</key>\n"\
	"	<string>caio</string>\n"\
	"	<key>CFBundleExecutable</key>\n"\
	"	<string>caio</string>\n"\
	"	<key>CFBundleIconFile</key>\n"\
	"	<string>${BUNDLE_ICON_RELATIVE}</string>\n"\
	"	<key>NSHumanReadableCopyright</key>\n"\
	"	<string>Copyright © 2020-2024 Claudio Castiglia</string>\n"\
	"</dict>\n"\
	"</plist>\n" > $@

include ${ROOT}/mk/clean.mk
