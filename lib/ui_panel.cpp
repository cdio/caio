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
#include "ui_panel.hpp"


namespace cemu {
namespace ui {

void Panel::add(const std::shared_ptr<Widget> &widget, bool right)
{
    if (find(widget) == _widgets.end()) {
        _widgets.push_back({widget, right});
    }
}

void Panel::del(const std::shared_ptr<Widget> &widget)
{
    auto it = find(widget);
    if (it != _widgets.end()) {
        _widgets.erase(it);
    }
}

}
}
