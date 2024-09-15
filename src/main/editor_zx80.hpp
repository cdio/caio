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
#include "zx80/zx80_config.hpp"

#include "editor.hpp"

namespace caio {
namespace ui {
namespace sdl2 {

class ConfigEditorZX80 : public ConfigEditor {
public:
    ConfigEditorZX80(Gui& gui, const fs::Path& cfile, config::Section&& sec);

    virtual ~ConfigEditorZX80();

    void save() override;

private:
    void            render_audio(config::Config& cfg) override;
    void            render_keyboard(config::Config& cfg) override;
    void            render_appearance(config::Config& cfg) override;
    void            render_specific() override;
    bool            is_changed() const override;
    config::Config& generic_config() override;
    std::string     machine_name() override;
    std::string     machine_prefix() override;

    sinclair::zx80::ZX80Config _conf;
    sinclair::zx80::ZX80Config _conf_orig;

    fs::IDirNav _cassdir{fs::IDir::EntryType::Dir, Gui::ENTRY_EMPTY};
    fs::IDirNav _prgfile{fs::IDir::EntryType::All, Gui::ENTRY_EMPTY};
};

}
}
}
