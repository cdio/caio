/*
 * Copyright (C) 2020-2022 Claudio Castiglia
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

SLEffect to_sleffect(const std::string &str)
{
    if (str.empty()) {
        return SLEffect::NONE;
    }

    if (str.size() == 1 && utils::tolow(str).find_first_of("nhv") != std::string::npos) {
        return static_cast<SLEffect>(+str[0]);
    }

    std::stringstream ss{};
    ss << "Invalid scanline effect: " << std::quoted(str);
    throw InvalidArgument{ss.str()};
}

}
}
