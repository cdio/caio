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
#include "dialog.hpp"

#include "fs.hpp"
#include "logger.hpp"
#include "utils.hpp"

#include <sys/wait.h>
#include <unistd.h>

namespace caio {
namespace ui {
namespace sdl2 {

std::string dialog_exec(const std::string& args)
{
    const auto cmd = std::format("{} dialog {}", fs::exec_path().string(), args);
    const auto [err, ss] = fs::shell(cmd);
    return (err ? "" : utils::trim(ss.str()));
}

std::string dialog_saveas(const std::string& msg, const std::string& dir, const std::string& fname,
    const std::string& ext)
{
    const auto extopt = (ext.empty() ? "" : "-e " + ext);
    const auto args = std::format("-s -t 'Select file to save' -m '{}' -p '{}' -c '{}' {}", msg, dir, fname, extopt);
    return dialog_exec(args);
}

std::string dialog_pick_file(const std::string& msg, const std::string& dir, const std::string& fname,
    const std::string& ext)
{
    const auto extopt = (ext.empty() ? "" : "-e " + ext);
    const auto args = std::format("-t 'Select file to load' -m '{}' -p '{}' -c '{}' {}", msg, dir, fname, extopt);
    return dialog_exec(args);
}

void dialog_error(const std::string& reason, const std::string& errmsg)
{
    const auto& args = std::format("-E -t '{}' -m '{}'", reason, errmsg);
    dialog_exec(args);
}

}
}
}
