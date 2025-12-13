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
#include "message_dialog.hpp"

extern std::string progname;

struct DialogConfig {
    using EntryType = caio::fs::IDir::EntryType;
    using DialogType = caio::ui::sdl2::FileDialogApp::DialogType;

    bool            is_error{};
    std::string     title{};
    std::string     msg{};
    DialogType      dtype{DialogType::Load};
    EntryType       etype{EntryType::All};
    caio::fs::Path  path{};
    caio::fs::Path  ext{};
    caio::fs::Path  selected{};
};

static int usage()
{
    const auto usage = std::format(
        "usage: {} dialog -t <title> -m <msg> [-s] [-d] -p <dir> [-c <path>] [-e <ext>]\n"
        "       {} dialog -E -t <title> -m <msg>\n"
        "where:\n"
        "  -E            Open an error dialog\n"
        "  -t <title>    Window title\n"
        "  -r <reason>   Dialog reason\n"
        "  -m <msg>      Dialog message\n"
        "  -s            Save-As dialog\n"
        "  -d            Select a directory\n"
        "  -p <dir>      Starting directory\n"
        "  -c <path>     Current selected fullpath\n"
        "  -e <ext>      Extension filter (default is .*)\n\n",
        progname,
        progname);

    std::cerr << usage;
    return EXIT_FAILURE;
}

int main_dialog(int argc, const char** argv)
{
    DialogConfig config{};
    int ch{};

    while ((ch = ::getopt(argc - 1, (char* const*)(argv) + 1, "Et:r:m:sdp:c:e:h")) != -1) {
        switch (ch) {
        case 'E':
            config.is_error = true;
            break;
        case 't':
            /* Dialog title */
            config.title = optarg;
            break;
        case 'm':
            /* Dialog message */
            config.msg = optarg;
            break;
        case 's':
            /* Save-As dialog type (allow non-existent entries) */
            config.dtype = DialogConfig::DialogType::SaveAs;
            break;
        case 'd':
            /* Directory selection */
            config.etype = DialogConfig::EntryType::Dir;
            break;
        case 'p':
            /* First directory to traverse (starting point) */
            config.path = optarg;
            break;
        case 'c':
            /* Current selected/default fullpath */
            config.selected = optarg;
            break;
        case 'e':
            /* File extension filter */
            config.ext = optarg;
            break;
        case 'h':
        default:
            return usage();
       }
    }

    if ((config.is_error && (config.title.empty() || config.msg.empty())) ||
        (!config.is_error && (config.title.empty() || config.msg.empty() || config.path.empty()))) {
        return usage();
    }

    using ErrorDialogApp = caio::ui::sdl2::ErrorDialogApp;
    using FileDialogApp = caio::ui::sdl2::FileDialogApp;

    if (config.is_error) {
        return ErrorDialogApp{config.title, config.msg}.run();
    }

    FileDialogApp fdialog{config.title, config.msg, config.dtype,
        config.etype, config.path, config.selected, config.ext};

    return fdialog.run();
}
