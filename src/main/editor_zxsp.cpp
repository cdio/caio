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
#include "editor_zxsp.hpp"

namespace caio {
namespace ui {
namespace sdl2 {

ConfigEditorZXSpectrum::ConfigEditorZXSpectrum(Gui& gui, const fs::Path& cfile, config::Section&& sec)
    : ConfigEditor{gui, cfile, std::move(sec)},
      _conf{_sec},
      _conf_orig{_conf}
{
}

ConfigEditorZXSpectrum::~ConfigEditorZXSpectrum()
{
}

void ConfigEditorZXSpectrum::save()
{
    _conf.to_section(_sec);
    ConfigEditor::save();
    _conf_orig = _conf;
}

void ConfigEditorZXSpectrum::render_specific()
{
    _gui.begin_subsection("Attachments:");
    _gui.checkbox("Input tape fastload", "##fastload", _conf.fastload);
    _gui.combo_file("Input tape", "##itape", _conf.itape, _itape);
    _gui.combo_directory("Output tape", "##otape", _conf.otape, _otape);
    _gui.combo_file("Load snapshot", "##snapshot", _conf.snap, _snapshot);
    _gui.end_subsection();
}

bool ConfigEditorZXSpectrum::is_changed() const
{
    return ((_conf != _conf_orig) || ConfigEditor::is_changed());
}

config::Config& ConfigEditorZXSpectrum::generic_config()
{
    return _conf;
}

std::string ConfigEditorZXSpectrum::machine_name()
{
    return "Sinclair ZX-Spectrum";
}

std::string ConfigEditorZXSpectrum::machine_prefix()
{
    return "zxspectrum_";
}

}
}
}
