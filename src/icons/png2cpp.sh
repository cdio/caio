#!/bin/sh
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
PNG="$1"

NAME="${PNG%%.png}_png"
SIZE=$(ls -l ${PNG} | awk '{print $5'})

cat <<-END
/*
 * Automatically generated from ${PNG}
 */
static const std::array<uint8_t, ${SIZE}> ${NAME}{
END

printf "    "
hexdump -e '16/1 "0x%02x, " "\n    "' ${PNG} | sed -e 's/, 0x  //g' -e 's/    $//'

cat <<-END
};
END
