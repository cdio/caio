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

#include "ui_sfml.hpp"
#include "ui_sfml_widget.hpp"
#include "ui_sfml_widget_floppy.hpp"
#include "ui_sfml_widget_fullscreen.hpp"
#include "ui_sfml_widget_gamepad.hpp"


namespace caio {
namespace ui {

using AudioBuffer = ui::sfml::AudioBuffer;
using Scanline    = ui::sfml::Scanline;
using Panel       = ui::sfml::PanelSfml;
using UI          = ui::sfml::UISfml;
using Widget      = ui::sfml::WidgetSfml;

namespace widget  = ui::sfml::widget;

}
}
