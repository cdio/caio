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
include ${ROOT}/mk/config.mk

ifeq (${VERSION}, 0.0.0)
VERSION=	${shell date "+%Y%m%d"}-snapshot
endif

ifeq (${OS}, Darwin)
include ${ROOT}/mk/dmg-package.mk

else ifeq (${OS}, Linux)
include ${ROOT}/mk/deb-package.mk

else
package:
	@echo
	@echo "No package file defined for Operating System \"${OS}\""
	@echo
	@exit 1

endif
