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
# MacOS application bundle and package files .pkg and .dmg.
#
PKG_NAME=		caio_${VERSION}-${ARCH}
PKG_FILE=		${ROOT}/${PKG_NAME}.pkg
DMG_FILE=		${ROOT}/${PKG_NAME}.dmg
DMG_DIR=		${ROOT}/${PKG_NAME}

BUNDLE_NAME=		caio.app
BUNDLE_DIR=		${ROOT}/${BUNDLE_NAME}
BUNDLE_CONTENTS_DIR=	${BUNDLE_DIR}/Contents
BUNDLE_BINDIR=		${BUNDLE_CONTENTS_DIR}/MacOS
BUNDLE_DATADIR=		${BUNDLE_CONTENTS_DIR}/Resources
BUNDLE_ICON_RELATIVE=	icons/caio.png
BUNDLE_INFO_PLIST=	${BUNDLE_CONTENTS_DIR}/Info.plist

INSTALL_DIR=		/Applications
RUNTIME_DATADIR=	../Resources

CLEANFILES+=		${PACKAGES} \
			${BUNDLE_DIR} \
			${PKG_NAME}

MK_PACKAGE?=		productbuild
MK_IMAGE?=		hdiutil

CURRENT_YEAR:=		${shell date +"%Y"}

.PHONY: all bundle install package

package: ${DMG_FILE} ${PKG_FILE}

${DMG_FILE}: bundle
	[ -d ${DMG_DIR} ] || ${INSTALL} -d -m 0755 ${DMG_DIR}
	${CP} -Rp ${BUNDLE_DIR} ${DMG_DIR}
	cd ${DMG_DIR} && ${LN} -sf ${INSTALL_DIR}
	-${RM} $@
	${MK_IMAGE} create -srcfolder ${DMG_DIR} -format UDZO $@

${PKG_FILE}: bundle
	${MK_PACKAGE} --component ${BUNDLE_DIR} ${INSTALL_DIR} $@

bundle: EXTRA_CPPFLAGS+=-DGUI_COMBO_PATH_RELATIVE
bundle: install ${BUNDLE_INFO_PLIST}

install: all
	DST_BINDIR="${BUNDLE_BINDIR}" \
	DST_DATADIR="${BUNDLE_DATADIR}" \
	DST_SYSCONFDIR="${BUNDLE_DATADIR}" \
	PREFIX="${BUNDLE_DATADIR}" \
	VERSION=${VERSION} \
	${MAKE} ${MAKEARGS} $@

all:
	EXTRA_CPPFLAGS='${EXTRA_CPPFLAGS} -DD_SYSCONFDIR=\"${RUNTIME_DATADIR}\" -DD_DATADIR=\"${RUNTIME_DATADIR}\"' \
	PREFIX="${RUNTIME_DATADIR}" \
	VERSION=${VERSION} \
	${MAKE} ${MAKEARGS} $@

${BUNDLE_INFO_PLIST}: ${dir $@} ${ROOT}/mk/dmg-package.mk
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
	"	<string>Copyright © 2020-${CURRENT_YEAR} Claudio Castiglia</string>\n"\
	"</dict>\n"\
	"</plist>\n" > $@

include ${ROOT}/mk/clean.mk
