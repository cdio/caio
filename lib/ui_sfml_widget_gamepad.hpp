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
#pragma once

#include "ui_widget_sfml.hpp"


namespace cemu {
namespace ui {
namespace sfml {
namespace widget {

class Gamepad : public WidgetSfml {
public:
    using WidgetSfml::WidgetSfml;

    void load() override;

    sf::Sprite sprite() override;

private:
    bool       _is_swapped{};
    sf::Sprite _sprite{};
};

}
}
}
}
