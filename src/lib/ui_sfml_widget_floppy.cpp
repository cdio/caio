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
#include "ui_sfml_widget_floppy.hpp"


namespace caio {
namespace ui {
namespace sfml {
namespace widget {

#include "icons/floppy_64.hpp"


Floppy::Floppy(const std::function<Status()> &upd)
    : _update{upd}
{
    WidgetSfml::load(floppy_64_png);
    _sprite = rect({0, 0, 64, 64});
}

sf::Sprite Floppy::make_sprite()
{
    Status st{};
    if (_update) {
       st = _update();
    }

    const Rgba &color = (st.is_attached ? DISK_ATTACHED_COLOR : DISK_MISSING_COLOR);
    _sprite.setColor(sf::Color{color.to_host_u32()});
    if (st.is_idle) {
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
