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

#include "types.hpp"
#include "utils.hpp"

#include <unordered_map>

namespace caio {
namespace ui {

static const std::unordered_map<std::string, AspectRatio> aspects{
    { "16:9",   AspectRatio::_16_9  },
    { "8:7",    AspectRatio::_8_7   },
    { "6:5",    AspectRatio::_6_5   },
    { "5:3",    AspectRatio::_5_3   },
    { "4:3",    AspectRatio::_4_3   },
    { "system", AspectRatio::System }
};

AspectRatio to_aspect_ratio(std::string_view str)
{
    if (str.empty()) {
        return AspectRatio::System;
    }

    const auto it = aspects.find(utils::tolow(str));
    if (it == aspects.end()) {
        throw InvalidArgument{"Invalid aspect ratio: \"{}\"", str};
    }

    return it->second;
}

std::string to_string(AspectRatio ratio)
{
    const auto it = std::find_if(aspects.begin(), aspects.end(), [ratio](const auto& entry) {
        return (entry.second == ratio);
    });

    return (it == aspects.end() ? "" : it->first);
}

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

std::string to_string(SLEffect effect)
{
    switch (effect) {
    case SLEffect::Horizontal:
    case SLEffect::Vertical:
    case SLEffect::Adv_Horizontal:
    case SLEffect::Adv_Vertical:
        return std::string{static_cast<char>(effect)};

    case SLEffect::None:;
    }

    return "n";
}

}
}
