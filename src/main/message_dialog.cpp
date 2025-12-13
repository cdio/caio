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
#include "message_dialog.hpp"

#include "utils.hpp"

namespace caio {
namespace ui {
namespace sdl2 {

MessageDialogApp::MessageDialogApp(const std::string& title, const std::string& reason, const std::string& message)
    : GuiApp{title, DIALOG_SIZE, MESSAGE_DIALOG_INIFILE},
      _title{title},
      _reason{reason},
      _message{message}
{
}

void MessageDialogApp::render()
{
    begin_window(_title, {}, window_size(), ::ImGuiWindowFlags_NoSavedSettings | ::ImGuiWindowFlags_NoTitleBar);
    message_box(_reason, _message, std::bind(&GuiApp::stop, this, 0));
    end_window();
}

}
}
}
