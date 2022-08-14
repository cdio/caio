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
#include "ui_sfml_widget_gamepad.hpp"


namespace caio {
namespace ui {
namespace sfml {
namespace widget {

#include "icons/gamepad_64x2.hpp"


Gamepad::Gamepad(const std::function<Status()> &upd)
    : _update{upd}
{
    WidgetSfml::load(gamepad_64x2_png);
}

sf::Sprite Gamepad::make_sprite()
{
    Status st{};
    if (_update) {
        st = _update();
    }

    const Rgba &color = (st.is_connected ? GAMEPAD_PRESENT_COLOR : GAMEPAD_MISSING_COLOR);
    auto sprite = rect(st.is_swapped ? sf::IntRect{64, 0, 128, 64} : sf::IntRect{0, 0, 64, 64});
    sprite.setColor(sf::Color{color.to_host_u32()});
    return sprite;
}

}
}
}
}
