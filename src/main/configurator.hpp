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

#include <map>
#include <string>
#include <string_view>
#include <vector>
#include <tuple>
#include <utility>

#include "editor.hpp"
#include "guiapp.hpp"

namespace caio {
namespace ui {
namespace sdl2 {

/**
 * Configurator Application.
 * The Configurator is a GUI application that allows the user to add,
 * delete, edit, and run different machine configurations.
 * @see ConfigEditor
 * @see GuiApp
 */
class ConfiguratorApp : public GuiApp {
public:
    constexpr static const char* FNAME_PATTERN      = "*.conf";
    constexpr static const char* MACHINES_DIR       = D_DATADIR "/machines";
    constexpr static const char* SEC_NAME           = ConfigEditor::SEC_NAME;
    constexpr static const char* KEY_MACHINE        = ConfigEditor::KEY_MACHINE;
    constexpr static const uint64_t UPDATE_INTERVAL = 1'000'000;
    constexpr static const char* ID_ADD_CONFIG      = "Add configuration";
    constexpr static const char* ID_RENAME_CONFIG   = "Rename configuration";
    constexpr static const char* ID_DELETE_CONFIG   = "Delete configuration";

    /**
     * Get the instance to the configurator.
     * @return A reference to the configurator.
     */
    static ConfiguratorApp& instance();

    virtual ~ConfiguratorApp();

private:
    /* ConfigEntry = std::tuple<read-only, name, path> */
    using ConfigEntry = std::tuple<bool, std::string, fs::Path>;
    using ConfigVector = std::vector<ConfigEntry>;
    using LoadedConfigMap = std::map<fs::Path, uptr_t<ConfigEditor>>;

    /**
     * Initialise this configurator.
     * @exception IOError
     * @see ConfigEditor::create_default_configurations()
     */
    ConfiguratorApp();

    /**
     * Render the main window.
     */
    void render() override;

    /**
     * Render the buttons pane.
     */
    bool buttons_pane();

    /**
     * Render the configuration selection pane.
     */
    void selector_pane();

    /**
     * Render the editor pane.
     */
    void editor_pane();

    /**
     * Render the popup that allows the user to add a new configuraiton.
     */
    void add_config_popup();

    /**
     * Render the popup that allows the user to rename a configuraitons.
     */
    void rename_config_popup();

    /**
     * Render the popup that allows the user to delete a machine configuration.
     */
    void delete_config_popup();

    /**
     * Render the popup of an error message.
     * The popup is rendered only if an error message is set.
     * @see error_message(const std::string&, const std::string&)
     */
    void error_message_popup();

    /**
     * Set an error message.
     * The error message is shown as soon as a new frame is rendered.
     * @param title Title;
     * @param msg   Error message.
     * @see error_message_popup()
     * @see render()
     */
    void error_message(const std::string& title, const std::string& msg);

    /**
     * Run the emulator using the current (user selected) configuration.
     */
    void run_machine();

    /**
     * Update the list of configurations kept in memory.
     * @see _configs
     */
    void update_configs();

    /**
     * Get a configuration editor.
     * @param cfile Configuration file name.
     * @return The configuration editor.
     */
    uptr_t<ConfigEditor>& editor_instance(const fs::Path& cfile);

    ConfigVector        _configs{};             /* List of configurations                   */
    ssize_t             _centry{};              /* Selected configuration                   */
    uint64_t            _last_update{};         /* List of configurations last update time  */
    LoadedConfigMap     _loaded_configs{};      /* Loaded configurations                    */

    ssize_t             _add_centry{};          /* Add config entry to copy from            */
    std::string         _add_cname{};           /* Add config name                          */
    std::string         _rename_cname{};        /* Rename config name                       */
    bool                _rename_set{};          /* Rename config name set flag              */
    std::string         _error_title{};         /* Error message title                      */
    std::string         _error_message{};       /* Error message                            */
    Size                _selector_pane_size{};  /* Size of the selector pane                */

    int                 _child_pipe[2]{-1, -1}; /* Error messages from child processes      */
};

}
}
}
