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
#include "ui_sdl2/widget_snapshot.hpp"

namespace caio {
namespace ui {
namespace sdl2 {
namespace widget {

#include "icons/snapshot_128x2.hpp"

Snapshot::Snapshot(const sptr_t<::SDL_Renderer>& renderer, Type type)
    : Widget{renderer},
      _rect{128 * static_cast<int>(type), 0, 128, 128}
{
    Widget::load(snapshot_128x2_png);
}

void Snapshot::render(const ::SDL_Rect& dstrect)
{
    Widget::render(_rect, dstrect, ENABLED_COLOR);
}

}
}
}
}
