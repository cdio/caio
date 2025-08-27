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
#include "ui_sdl2/widget_empty.hpp"

namespace caio {
namespace ui {
namespace sdl2 {
namespace widget {

Empty::Empty(const sptr_t<::SDL_Renderer>& renderer)
    : Widget{renderer}
{
}

bool Empty::enabled() const
{
    return false;
}

bool Empty::is_idle()
{
    return true;
}

void Empty::render(const ::SDL_Rect& dstrect)
{
}

}
}
}
}
