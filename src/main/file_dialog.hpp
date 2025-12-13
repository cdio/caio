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
 * File Dialog Application.
 * The File Dialog is a GUI application that allows the user to select a file.
 * @see GuiApp
 */
class FileDialogApp : public GuiApp {
public:
    constexpr static const char* DIALOG_INIFILE = "file_dialog.ini";

    /**
     * Type of dialog.
     */
    enum class DialogType {
        Load,       /* Select an existing path                  */
        SaveAs      /* Select an existing or non-existing path  */
    };

    /**
     * Get the instance to the file dialog.
     * @param title   Window title;
     * @param message Window message;
     * @param dtype   Dialog type;
     * @param etype   File Entry type;
     * @param dir     Initial traversed directory;
     * @param fname   Initial selected path name;
     * @param ext     File extension (used to filter file names);
     * @exception IOError
     * @see Type
     * @see GuiApp::GuiApp
     * @see IDir::EntryType
     */
    FileDialogApp(const std::string& title, const std::string& message, DialogType dtype, fs::IDir::EntryType etype,
        const std::string& dir, const std::string& fname, const std::string& ext);

    virtual ~FileDialogApp() = default;

    /**
     * Start this File Dialog application.
     * This method returns when the user selects an entry or cancels the dialog.
     * If an entry is selected it sent to standard output.
     * @return 0 if an entry is selected; -1 if the operation is cancelled.
     * @exception UIError
     * @see GuiApp::run()
     */
    int run();

private:
    /**
     * Reset this dialog.
     * @param dir   Directory to traverse;
     * @param fname selected path name;
     */
    void reset(const std::string& dir, const std::string& fname);

    /**
     * Render the main window.
     */
    void render() override;

    std::string _message;   /* Message          */
    IDirNavGui  _idir;      /* Traversed path   */
    std::string _fname;     /* Selected entry   */
};

}
}
}
