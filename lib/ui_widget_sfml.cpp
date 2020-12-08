/*
 * Copyright (C) 2020 Claudio Castiglia
 *
 * This file is part of CEMU.
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
#include "ui_widget_sfml.hpp"

#include "types.hpp"


namespace cemu {
namespace ui {
namespace sfml {

extern std::stringstream sfml_err;


void WidgetSfml::load(const std::string &fname)
{
    if (!_texture.loadFromFile(fname)) {
        throw UIError{"Can't load texture: " + fname + ": " + sfml_err.str()};
    }
}

void WidgetSfml::load(const gsl::span<const uint8_t> &data)
{
    if (!_texture.loadFromMemory(data.data(), data.size())) {
        throw UIError{"Can't load texture from data: " + sfml_err.str()};
    }
}

}
}
}
