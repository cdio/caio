/*
 * Copyright (C) 2020 Claudio Castiglia
 *
 * This file is part of caio.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see http://www.gnu.org/licenses/
 */
#include "ui_config.hpp"

#include <sstream>

#include "types.hpp"
#include "utils.hpp"

namespace caio {
namespace ui {

SLEffect to_sleffect(std::string_view str)
{
    if (str.empty()) {
        return SLEffect::None;
    }

    if (str.size() == 1 && str.find_first_of("nhvHV") != std::string_view::npos) {
        return static_cast<SLEffect>(+str[0]);
    }

    throw InvalidArgument{"Invalid scanlines effect: \"{}\"", str};
}

}
}
