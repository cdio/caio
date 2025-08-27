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
#pragma once

#include "dearimgui.hpp"
#include "ui_sdl2/sdl2.hpp"

namespace caio {
namespace ui {
namespace sdl2 {

constexpr static const uint32_t UNI32_LOCK_CLOSED = 0x0000F456;
constexpr static const char* UNI_LOCK_CLOSED      = "\U0000F456";

using Gui = dearimgui::Gui;

}
}
}
