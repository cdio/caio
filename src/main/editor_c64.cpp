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
#include "editor_c64.hpp"

namespace caio {
namespace ui {
namespace sdl2 {

ConfigEditorC64::ConfigEditorC64(Gui& gui, const fs::Path& cfile, config::Section&& sec)
    : ConfigEditor{gui, cfile, std::move(sec)},
      _conf{_sec},
      _conf_orig{_conf}
{
}

ConfigEditorC64::~ConfigEditorC64()
{
}

void ConfigEditorC64::save()
{
    _conf.to_section(_sec);
    ConfigEditor::save();
    _conf_orig = _conf;
}

void ConfigEditorC64::render_specific()
{
    _gui.begin_subsection("Joysticks:");
    _gui.checkbox("Swap joysticks", "##swapj", _conf.swapj);
    _gui.end_subsection();

    _gui.newline();

    _gui.begin_subsection("Disk Drives:");
    _gui.combo_directory("Disk drive unit 8", "##unit-8", _conf.unit8, _unit8);
    _gui.combo_directory("Disk drive unit 9", "##unit-9", _conf.unit9, _unit9);
    _gui.end_subsection();

    _gui.newline();

    _gui.begin_subsection("Attachments:");
    _gui.combo_file("Cartridge", "##cartridge", _conf.cartridge, _cartridge);
    _gui.combo_file("Preload program", "##prg", _conf.prgfile, _program);
    _gui.end_subsection();

    _gui.newline();
}

bool ConfigEditorC64::is_changed() const
{
    return ((_conf != _conf_orig) || ConfigEditor::is_changed());
}

config::Config& ConfigEditorC64::generic_config()
{
    return _conf;
}

std::string ConfigEditorC64::machine_name()
{
    return "Commodore 64";
}

std::string ConfigEditorC64::machine_prefix()
{
    return "c64_";
}

}
}
}
