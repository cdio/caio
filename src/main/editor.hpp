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

#include <string>
#include <string_view>
#include <vector>

#include "config.hpp"
#include "fs.hpp"
#include "keyboard.hpp"
#include "types.hpp"

#include "guiapp.hpp"

namespace caio {
namespace ui {
namespace sdl2 {

/**
 * Configuation File Editor.
 * The configuration file editor is a GUI that allows the user to add,
 * delete, rename and modify configuration parameters for different
 * emulated machines.
 */
class ConfigEditor {
public:
    constexpr static const char* SEC_NAME    = config::SEC_GENERIC;
    constexpr static const char* KEY_MACHINE = "machine";
    constexpr static const char* FNAME_EXT   = "conf";

    /**
     * Instantiate a configuration editor.
     * Find the correct configuration editor based in the
     * parameters stored in the specified configuration file.
     * @param gui   Widget toolkit;
     * @param cfile Configuration file name.
     * @return A unique pointer to the configuration editor on success;
     * an empty pointer if the configuration editor is not found.
     * @exception IOError
     * @see Gui
     */
    static uptr_t<ConfigEditor> make_editor(Gui& gui, const fs::Path& cfile);

    /**
     * Create the default configuration files.
     * @exception IOError If there is an error during the save operation.
     * @see config::save(std::string_view, std::string_view, const Section&)
     */
    static void create_default_configs();

    /**
     * Get the fullpath for a configuration file.
     * @param fname Configuration file name.
     * @return The fullpath for the specified configuration file.
     */
    static fs::Path config_path(const fs::Path& fname);

    virtual ~ConfigEditor();

    /**
     * Save the configuration to file.
     */
    virtual void save();

    /**
     * Get the status of the configuraiton.
     * @return True if the configuraiton was modified; otherwise false.
     */
    virtual bool is_changed() const;

    /**
     * Restore the original configuration values.
     */
    void restore();

    /**
     * Restore the original value of a parameter.
     * @param key Key.
     */
    void restore(const std::string& key);

    /**
     * Render the configuration parameters and let the user edit them.
     * @param ronly True if the configuration cannot be edited; false otherwise.
     * @see render_specifics()
     */
    void render(bool ronly);

protected:
    /**
     * Initialise this editor.
     * @param gui   Widget toolkit;
     * @param cfile Configuration file name;
     * @param sec   Configuration section (stored in the configuration file).
     * @see config::Section
     * @see Gui
     */
    ConfigEditor(Gui& gui, const fs::Path& cfile, config::Section&& sec);

    /**
      * Render the system directories and let the user edit them.
      * @param cfg Generic configuration.
      * @see generic_config()
      */
    virtual void render_directories(config::Config& cfg);

    /**
     * Render the virtual joystick parameters and let the user edit them.
     */
    virtual void render_appearance(config::Config& cfg);

    /**
     * Render the audio parameters and let the user edit them.
     */
    virtual void render_audio(config::Config& cfg);

    /**
     * Render the virtual joystick parameters and let the user edit them.
     */
    virtual void render_vjoy(config::Config& cfg);

    /**
     * Render the machine specific parameters and let the user edit them.
     */
    virtual void render_specific();

    /**
     * Get the generic configuration.
     * @return A reference to the generic configuration.
     * @see config::Config
     */
    virtual config::Config& generic_config() = 0;

    /**
     * Get the full name of the machine.
     * @return The name of the machine.
     */
    virtual std::string machine_name() = 0;

    /**
     * Get the machine's filename prefix.
     * @return The filename prefix.
     */
    virtual std::string machine_prefix() = 0;

    Gui             _gui;           /* Widgets tookit                   */
    fs::Path        _cfile;         /* Configuration file name          */
    config::Section _sec;           /* Configuration section            */
    config::Section _sec_orig;      /* Original configuration section   */

    fs::IDirNav     _romdir{fs::IDir::EntryType::Dir, Gui::ENTRY_EMPTY};
    fs::IDirNav     _keymapsdir{fs::IDir::EntryType::Dir, Gui::ENTRY_EMPTY};
    fs::IDirNav     _palettedir{fs::IDir::EntryType::Dir, Gui::ENTRY_EMPTY};
    fs::IDirNav     _screenshotdir{fs::IDir::EntryType::Dir, Gui::ENTRY_EMPTY};
    fs::IDir        _palette{fs::IDir::EntryType::File, Gui::ENTRY_EMPTY};
    fs::IDir        _keymaps{fs::IDir::EntryType::File, Gui::ENTRY_EMPTY};
};

}
}
}
