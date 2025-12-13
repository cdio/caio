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
#include "file_dialog.hpp"

namespace caio {
namespace ui {
namespace sdl2 {

FileDialogApp::FileDialogApp(const std::string& title, const std::string& message, DialogType dtype,
    fs::IDir::EntryType etype, const std::string& dir, const std::string& fname, const std::string& ext)
    : GuiApp{title, DIALOG_INIFILE},
      _message{message},
      _idir{IDirNavGui::APP,
            (dtype == DialogType::SaveAs ? IDirNavGui::ALLOW_NONEXISTENT : IDirNavGui::ONLY_EXISTENT),
            etype},
      _fname{fname}
{
    if (!ext.empty()) {
        const auto extension_filter = [ext](const fs::Path& path) {
            return (path.extension() == ext ? path : fs::Path{});
        };

        _idir.filter(extension_filter);
    }
}

int FileDialogApp::run()
{
    const int eval = GuiApp::run();
    if (eval == 0) {
        std::cout << _fname;
    }

    return eval;
}

void FileDialogApp::reset(const std::string& dir, const std::string& fname)
{
    _idir.reset(dir);
    _idir.position({0, 0});
    _idir.size(window_size());
    _idir.show(true);
    _fname = fname;
}

void FileDialogApp::render()
{
    const auto wsize = window_size();
    begin_window("##FileDialogApp", wsize);

    theme_banner();

    const Size navpos{0, font_height() * 2.0f};
    const Size navsiz{wsize.x, wsize.y - font_height() * 1.5f};
    _idir.render(_message, "##file-dialog", _fname, navpos, navsiz, 0);

    /*
     * The renderer makes IDirNavGui non-visible
     * when the dialog is closed.
     */
    if (!_idir.is_visible()) {
        const int eval = (_idir.is_cancelled() ? -1 : 0);
        stop(eval);
    }

    end_window();
}

}
}
}
