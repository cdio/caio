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

#include "guiapp.hpp"

namespace caio {
namespace ui {
namespace sdl2 {

/**
 * Message Dialog Application.
 * The Message Dialog is a GUI application that shows some information.
 * @see GuiApp
 */
class MessageDialogApp : public GuiApp {
public:
    constexpr static const char* MESSAGE_DIALOG_INIFILE = "message_dialog.ini";
    constexpr static const Size DIALOG_SIZE = {480, 320};

    /**
     * Get the instance to the message dialog.
     * #param title   Window title;
     * @param reason  Reason message;
     * @param message Actual message;
     * @exception UIError
     * @see GuiApp::GuiApp
     */
    MessageDialogApp(const std::string& title, const std::string& reason, const std::string& message);

    virtual ~MessageDialogApp() = default;

private:
    /**
     * Render the main window.
     */
    void render() override;

    std::string _title;
    std::string _reason;
    std::string _message;
};

class ErrorDialogApp : public MessageDialogApp {
public:
    constexpr static const char* ERROR_STR = "ERROR";

    ErrorDialogApp(const std::string& reason, const std::string& message)
        : MessageDialogApp{ERROR_STR, reason, message}
    {
    }
};

}
}
}
