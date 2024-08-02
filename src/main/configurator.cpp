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
#include "configurator.hpp"

#include <unistd.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>

#include "config.hpp"
#include "fs.hpp"
#include "logger.hpp"
#include "utils.hpp"

namespace caio {
namespace ui {
namespace sdl2 {

ConfiguratorApp::ConfiguratorApp(const fs::Path& progname)
    : GuiApp{"caio - Machine configurator"},
      _progname{progname}
{
    ConfigEditor::create_default_configs();
}

ConfiguratorApp::~ConfiguratorApp()
{
}

void ConfiguratorApp::render()
{
    begin_window("##configurator", window_size());

    bool run = buttons_pane();
    selector_pane();
    sameline();
    editor_pane();

    if (run) {
        run_machine();
    }

    add_config_popup();
    rename_config_popup();
    delete_config_popup();
    error_message_popup();

    end_window();
}

bool ConfiguratorApp::buttons_pane()
{
    button("Add", [](){ open_popup(ID_ADD_CONFIG); });
    sameline();
    button("Rename", [](){ open_popup(ID_RENAME_CONFIG); });
    sameline();
    button("Delete", [](){ open_popup(ID_DELETE_CONFIG); });
    sameline();
    print(" "); sameline();//to_column(8);
    bool run = button("Run");
    to_column(-6);
    button((style() == Style::Dark ? "Light Mode" : "Dark Mode"), [this](){
        style((style() == Style::Dark) ? Style::Light : Style::Dark);
    });
    sameline();
    button("+", [](){ process_font_incdec(true); });
    sameline();
    button("-", [](){ process_font_incdec(false); });

    return run;
}

void ConfiguratorApp::selector_pane()
{
    const auto [width, _] = window_size();

    begin_section("##config-list", {width * 0.2f, 0.0f}, SectionFlags::Border | SectionFlags::ResizeX);

    update_configs();

    for (ssize_t i = 0; i < _configs.size(); ++i) {
        const auto& [name, _] = _configs[i];
        select_table(name, _centry == i, [this, &i]() { _centry = i; });
    }

    _selector_pane_size = section_size();

    end_section();
}

void ConfiguratorApp::editor_pane()
{
    const float width = window_size().x - _selector_pane_size.x - FONT_SIZE * 2;
    const float height = _selector_pane_size.y;

    begin_section("##editor-pane", {width, height});

    if (_centry >= 0) {
        const auto& [cname, cfile] = _configs[_centry];
        print("Configuration: {}", cname);
        separator();
        try {
            auto& editor = editor_instance(cfile);
            if (editor) {
                editor->render();
            } else {
                print("Can't load configurator editor\nMalformed configuration file: {}", cfile.string());
            }
        } catch (const std::exception& err) {
            print("Can't load configurator editor\n{}", err.what());
        }
    }

    end_section();
}

void ConfiguratorApp::add_config_popup()
{
    define_popup(ID_ADD_CONFIG, [this]() {
        if (_add_cname.empty()) {
            /*
             * Step 1: The user must select a base configuration.
             */
            print("Select machine to copy from: ");
            separator();

            for (ssize_t i = 0; i < _configs.size(); ++i) {
                const auto& [name, _] = _configs[i];
                select_table(name, _add_centry == i, [this, &i]() { _add_centry = i; });
            }

            separator();

            buttons_ok_cancel(
                [this]() { _add_cname = _configs[_add_centry].first; },
                close_popup
            );

        } else {
            /*
             * Step 2: The user must set the name for the new configuration.
             */
            print("Enter machine name:");
            sameline();
            keyboard_focus();

            const auto flags = InputFlags::EnterReturnsTrue | InputFlags::AutoSelectAll | InputFlags::EscapeClearsAll;
            if (input("##enter-config-name", _add_cname, flags)) {
                const auto cfile = ConfigEditor::config_path(_add_cname);
                const auto& [_, base_cfile] = _configs[_add_centry];
                fs::unlink(cfile);
                fs::concat(cfile, base_cfile);
                _add_cname = {};
                _add_centry = 0;
                close_popup();
            }
        }
    });
}

void ConfiguratorApp::rename_config_popup()
{
    static const auto close = [this]() {
        close_popup();
        _rename_cname = {};
        _rename_set = {};
    };

    define_popup(ID_RENAME_CONFIG, [this]() {
        const auto it = _configs.begin() + _centry;
        const auto& [cname, cfile] = *it;

        if (_rename_cname.empty()) {
            _rename_cname = cname;
        }

        print("New name:");
        sameline();

        if (input("##enter-config-rename", _rename_cname, InputFlags::AutoSelectAll | InputFlags::EscapeClearsAll)) {
            _rename_set = true;
        }

        separator();

        const auto do_rename = [this, &cfile, &cname, &it]() {
            if (_rename_set && !_rename_cname.empty()) {
                auto newcfile = ConfigEditor::config_path(_rename_cname);
                if (newcfile != cfile) {
                    std::error_code ec{};
                    std::filesystem::rename(cfile, newcfile, ec);
                    if (ec) {
                        error_message(std::format("Can't rename: {}", cname), ec.message());
                    } else {
                        _loaded_configs.erase(cfile);
                        _configs.erase(it);
                        ConfigEditor::create_default_configs();     /* Re-create default configurations */
                        close_popup();
                    }
                }
            }
            close();
        };

        buttons_ok_cancel(do_rename, close);
    });
}

void ConfiguratorApp::delete_config_popup()
{
    define_popup(ID_DELETE_CONFIG, [this]() {
        auto it = _configs.begin() + _centry;
        const auto& [cname, cfile] = *it;

        print("Confirm delete configuration: {} ", cname);
        separator();

        const auto do_delete = [this, &cfile, &it]() {
            fs::unlink(cfile);
            _loaded_configs.erase(cfile);
            _configs.erase(it);
            ConfigEditor::create_default_configs();     /* Re-create default configurations */
            if (_centry > 0) {
                --_centry;
            }
            close_popup();
        };

        buttons_ok_cancel(do_delete, close_popup);
    });
}

void ConfiguratorApp::error_message_popup()
{
    if (!_error_message.empty()) {
        open_popup(_error_title);
    }

    define_popup(_error_title, [this]() {
        print(_error_message);
        separator();
        button_ok([this]() {
            _error_title = {};
            _error_message = {};
        });
    });
}

void ConfiguratorApp::error_message(const std::string& title, const std::string& msg)
{
    _error_title = "Error: " + title;
    _error_message = msg;
}

void ConfiguratorApp::run_machine()
{
    const pid_t pid = ::fork();
    if (pid < 0) {
        error_message("Can't run machine", std::format("Can't fork: {}\n", Error::to_string()));
        return;
    }

    if (pid != 0) {
        return;
    }

    const auto& cfile = _configs[_centry].second;
    config::Confile cf{cfile};
    auto& sec = cf[SEC_NAME];
    auto& machine = sec[KEY_MACHINE];

    if (!machine.empty()) {
        const char* argv[] = {
            _progname.c_str(),
            machine.c_str(),
            "--conf",
            cfile.c_str(),
            nullptr
        };

        const char* home = std::getenv("HOME");
        if (home == NULL || *home == '\0') {
            home = "/";
        }
        const auto env_home = std::format("HOME={}", home);

#ifdef __APPLE__
        const char* envp[] = {
            env_home.c_str(),
            nullptr
        };
#else
        const char* display = std::getenv("DISPLAY");
        if (display == NULL || *display == '\0') {
            display = ":0";
        }
        const auto env_display = std::format("DISPLAY={}", display);
        const char* envp[] = {
            env_home.c_str(),
            env_display.c_str(),
            nullptr
        };
#endif
        ::close(STDOUT_FILENO);
        ::close(STDIN_FILENO);
        ::close(STDERR_FILENO);
        ::execve(argv[0], const_cast<char**>(argv), const_cast<char**>(envp));
    }

    ::exit(EXIT_FAILURE);
}

void ConfiguratorApp::update_configs()
{
    const uint64_t now = utils::now();
    const bool update_time = ((now - _last_update) > UPDATE_INTERVAL) || !_configs.size();

    if (update_time) {
        auto dir = fs::directory(confdir(), FNAME_PATTERN, fs::MATCH_CASE_SENSITIVE);
        if (dir.size() == 0) {
            _centry = -1;
            return;
        }

        _configs.clear();

        std::for_each(dir.begin(), dir.end(), [this](const fs::DirEntry& entry) {
            const auto& cfile = entry.first;
            const std::string cname = cfile.stem();
            _configs.push_back({cname, cfile});
        });

        std::sort(_configs.begin(), _configs.end());
        _last_update = now;
    }
}

uptr_t<ConfigEditor>& ConfiguratorApp::editor_instance(const fs::Path& cfile)
{
    if (auto it = _loaded_configs.find(cfile); it != _loaded_configs.end()) {
        return it->second;
    }

    auto editor = ConfigEditor::make_editor(*this, cfile);
    auto [it, _] = _loaded_configs.emplace(cfile, std::move(editor));
    return it->second;
}

ConfiguratorApp& ConfiguratorApp::instance(const fs::Path& progname)
{
    static uptr_t<ConfiguratorApp> inst{};
    if (!inst) {
        inst = uptr_t<ConfiguratorApp>{new ConfiguratorApp{progname}};
    }
    return *inst;
}

}
}
}
