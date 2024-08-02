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

#include "config.hpp"
#include "c64/c64_config.hpp"

#include "editor.hpp"

namespace caio {
namespace ui {
namespace sdl2 {

class ConfigEditorC64 : public ConfigEditor {
public:
    ConfigEditorC64(Gui& gui, const fs::Path& cfile, config::Section&& sec);

    virtual ~ConfigEditorC64();

    void save() override;

private:
    void            render_specific() override;
    bool            is_changed() const override;
    config::Config& generic_config() override;
    std::string     machine_name() override;
    std::string     machine_prefix() override;

    commodore::c64::C64Config _conf;
    commodore::c64::C64Config _conf_orig;

    fs::IDirNav _unit8{fs::IDir::EntryType::Dir, Gui::ENTRY_EMPTY};
    fs::IDirNav _unit9{fs::IDir::EntryType::Dir, Gui::ENTRY_EMPTY};
    fs::IDirNav _cartridge{fs::IDir::EntryType::All, Gui::ENTRY_EMPTY};
    fs::IDirNav _program{fs::IDir::EntryType::All, Gui::ENTRY_EMPTY};
};

}
}
}
