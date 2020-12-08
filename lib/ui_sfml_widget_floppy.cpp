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
#include "ui_sfml_widget_floppy.hpp"


namespace cemu {
namespace ui {
namespace sfml {
namespace widget {

#include "icons/floppy64.hpp"


void Floppy::load()
{
    WidgetSfml::load(floppy64_png);
    _sprite.setTexture(texture());

    constexpr float x = WIDTH / 2.0f;
    constexpr float y = HEIGHT / 2.0f;

    _sprite.setOrigin(x, y);
    _sprite.setPosition(x, y);
}

sf::Sprite Floppy::sprite()
{
    bool is_idle = static_cast<bool>(update());
    if (is_idle) {
        _sprite.setRotation(0);
    } else {
        _sprite.rotate(5);
    }

    return _sprite;
}

}
}
}
}
