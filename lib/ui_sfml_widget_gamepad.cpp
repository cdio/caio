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
#include "ui_sfml_widget_gamepad.hpp"


namespace cemu {
namespace ui {
namespace sfml {
namespace widget {

#include "icons/gamepad64x2.hpp"


void Gamepad::load()
{
    WidgetSfml::load(gamepad64x2_png);
    _sprite = sf::Sprite(texture(), sf::IntRect{0, 0, 64, 64});
}

sf::Sprite Gamepad::sprite()
{
    bool is_swapped = static_cast<bool>(update());
    if (is_swapped != _is_swapped) {
        _sprite = sf::Sprite(texture(), (is_swapped ? sf::IntRect{64, 0, 128, 64} : sf::IntRect{0, 0, 64, 64}));
        _is_swapped = is_swapped;
    }

    return _sprite;
}

}
}
}
}
