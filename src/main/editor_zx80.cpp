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
#include "editor_zx80.hpp"

namespace caio {
namespace ui {
namespace sdl2 {

ConfigEditorZX80::ConfigEditorZX80(Gui& gui, const fs::Path& cfile, config::Section&& sec)
    : ConfigEditor{gui, cfile, std::move(sec)},
      _conf{_sec},
      _conf_orig{_conf}
{
}

void ConfigEditorZX80::save()
{
    _conf.to_section(_sec);
    ConfigEditor::save();
    _conf_orig = _conf;
}

void ConfigEditorZX80::render_audio(config::Config& cfg)
{
    _gui.begin_disabled();
    ConfigEditor::render_audio(cfg);
    _gui.end_disabled();
}

void ConfigEditorZX80::render_keyboard(config::Config& cfg)
{
    _gui.checkbox("Enable keyboard", "##kbd-enabled", cfg.keyboard);
    _gui.combo_keymaps(cfg.keymaps, _keymaps, machine_prefix());
}

void ConfigEditorZX80::render_appearance(config::Config& cfg)
{
    ConfigEditor::render_appearance(cfg);
    _gui.checkbox("Reverse video", "##rvideo", _conf.rvideo);
}

void ConfigEditorZX80::render_specific()
{
    _gui.begin_subsection("Memory Configuration:");
    _gui.checkbox("16K RAM", "##ram16", _conf.ram16);
    _gui.checkbox("8K ROM", "##rom8", _conf.rom8);
    _gui.end_subsection();

    _gui.newline();

    _gui.begin_subsection("Attached devices:");
    _gui.combo_path("Tape directory", "##cassdir", _conf.cassdir, _cassdir);
    _gui.combo_path("Preload program", "##prgfile", _conf.prgfile, _prgfile);
    _gui.end_subsection();
}

bool ConfigEditorZX80::is_changed() const
{
    return ((_conf != _conf_orig) || ConfigEditor::is_changed());
}

config::Config& ConfigEditorZX80::generic_config()
{
    return _conf;
}

std::string ConfigEditorZX80::machine_name()
{
    return "Sinclair ZX-80";
}

std::string ConfigEditorZX80::machine_prefix()
{
    return "zx80_";
}

}
}
}
