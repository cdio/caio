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
#pragma once

#include <memory>

#include "ui_sdl2/ui.hpp"
#include "ui_sdl2/widget.hpp"
#include "ui_sdl2/widget_empty.hpp"
#include "ui_sdl2/widget_floppy.hpp"
#include "ui_sdl2/widget_fullscreen.hpp"
#include "ui_sdl2/widget_gamepad.hpp"
#include "ui_sdl2/widget_pause.hpp"
#include "ui_sdl2/widget_reset.hpp"
#include "ui_sdl2/widget_volume.hpp"


namespace caio {
namespace ui {

using AudioBuffer = sdl2::AudioBuffer;
using Scanline    = sdl2::Scanline;

using UI          = sdl2::UI;
using Panel       = sdl2::Panel;
using Widget      = sdl2::Widget;

namespace widget  = sdl2::widget;


template<typename W, class... A>
std::shared_ptr<W> make_widget(UI &ui, const A&... args)
{
    return std::make_shared<W>(ui.renderer(), args...);
}

template<typename W, class... A>
std::shared_ptr<W> make_widget(const std::shared_ptr<UI> &ui, const A&... args)
{
    return make_widget<W>(*ui, args...);
}

}
}
