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
#include "editor.hpp"

#include "fs.hpp"
#include "utils.hpp"

#include "editor_c64.hpp"
#include "editor_nes.hpp"
#include "editor_zx80.hpp"
#include "editor_zxsp.hpp"

using namespace std::string_literals;

namespace caio {
namespace ui {
namespace sdl2 {

ConfigEditor::ConfigEditor(Gui& gui, const fs::Path& cfile, config::Section&& sec)
    : _gui{gui},
      _cfile{cfile},
      _sec{std::move(sec)},
      _sec_orig{_sec}
{
}

ConfigEditor::~ConfigEditor()
{
    try {
        if (is_changed()) {
            save();
        }
    } catch (...) {
    }
}

void ConfigEditor::save()
{
    config::save(_cfile, SEC_NAME, _sec);
    _sec_orig = _sec;
}

bool ConfigEditor::is_changed() const
{
    return ((_sec.size() != _sec_orig.size()) || !std::equal(_sec.begin(), _sec.end(), _sec_orig.begin()));
}

void ConfigEditor::restore()
{
    _sec = _sec_orig;
}

void ConfigEditor::restore(const std::string& key)
{
    const auto it = _sec_orig.find(key);
    if (it != _sec_orig.end()) {
        const auto& [key, value] = *it;
        _sec.insert_or_assign(key, value);
    }
}

void ConfigEditor::render(bool ronly)
{
    config::Config& cfg = generic_config();
    config::Config prev = cfg;

    _gui.begin_section("##config-editor");

    if (ronly) {
        _gui.begin_disabled();
    }

    _gui.begin_subsection("Machine:");
    _gui.print(machine_name());
    _gui.end_subsection();

    _gui.newline();

    _gui.begin_subsection("System Directories:");
    render_directories(cfg);
    _gui.end_subsection();

    _gui.newline();

    _gui.begin_subsection("Appearance:");
    render_appearance(cfg);
    _gui.end_subsection();

    _gui.newline();

    _gui.begin_subsection("Audio:");
    render_audio(cfg);
    _gui.end_subsection();

    _gui.newline();

    _gui.begin_subsection("Keyboard:");
    render_keyboard(cfg);
    _gui.end_subsection();

    _gui.newline();

    render_specific();

    _gui.newline();
    _gui.newline();

    if (ronly) {
        _gui.end_disabled();
    }

    _gui.end_section();

    if (is_changed()) {
        save();
    }
}

void ConfigEditor::render_directories(config::Config& cfg)
{
    auto& keymapsdir = _sec[config::KEY_KEYMAPSDIR];
    auto& palettedir = _sec[config::KEY_PALETTEDIR];

    _gui.combo_directory("ROMs directory", "##romdir", cfg.romdir, _romdir);
    _gui.combo_directory("Key Mappings directory", "##keymapsdir", keymapsdir, _keymapsdir);
    _gui.combo_directory("Palette directory", "##palettedir", palettedir, _palettedir);
    _gui.combo_directory("Screenshot directory", "##screenshotdir", cfg.screenshotdir, _screenshotdir);

    if (palettedir != _palette.path()) {
        _palette.reset(palettedir);
    }

    if (keymapsdir != _keymaps.path()) {
        _keymaps.reset(keymapsdir);
    }
}

void ConfigEditor::render_appearance(config::Config& cfg)
{
    constexpr static const size_t FPS_WIDTH = 4;
    constexpr static const int FPS_MAX = 100;
    constexpr static const int FPS_MIN = 1;
    constexpr static const size_t SCALE_WIDTH = 4;
    constexpr static const int SCALE_MAX = 10;
    constexpr static const int SCALE_MIN = 1;
    static const auto fps_cond = [](int fps) { return (fps >= FPS_MIN && fps <= FPS_MAX); };
    static const auto scale_cond = [](int scale) { return (scale >= SCALE_MIN && scale <= SCALE_MAX); };

    _gui.input_int("Frames per second", "##fps", reinterpret_cast<int&>(cfg.fps), FPS_WIDTH, fps_cond);
    _gui.checkbox("Fullscreen", "##fullscreen", cfg.fullscreen);
    _gui.checkbox("Smooth window resize", "##sresize", cfg.sresize);
    _gui.input_int("Scale", "##scale", reinterpret_cast<int&>(cfg.scale), SCALE_WIDTH, scale_cond);

    std::string aspect{ui::to_string(cfg.aspect)};
    _gui.combo_aspect(aspect);
    cfg.aspect = ui::to_aspect_ratio(aspect);

    std::string scanlines{ui::to_string(cfg.scanlines)};
    _gui.combo_scanlines(scanlines);
    cfg.scanlines = ui::to_sleffect(scanlines);

    _gui.combo_palette(cfg.palette, _palette, machine_prefix());
    _gui.combo_statusbar(cfg.statusbar);
}

void ConfigEditor::render_audio(config::Config& cfg)
{
    _gui.checkbox("Enable audio", "##audio", cfg.audio);
}

void ConfigEditor::render_keyboard(config::Config& cfg)
{
    config::VJoyConfig& vjoy = cfg.vjoy;

    _gui.checkbox("Enable keyboard", "##kbd-enabled", cfg.keyboard);
    _gui.combo_keymaps(cfg.keymaps, _keymaps, machine_prefix());
    _gui.newline();
    _gui.checkbox("Enable virtual joystick", "##vjoy-enabled", vjoy.enabled);
    _gui.begin_disabled(!vjoy.enabled);
    _gui.combo_key("UP key", "##key-up", vjoy.up);
    _gui.combo_key("DOWN Key", "##key-down", vjoy.down);
    _gui.combo_key("LEFT Key", "##key-left", vjoy.left);
    _gui.combo_key("RIGHT Key", "##key-right", vjoy.right);
    _gui.combo_key("FIRE Key", "##key-fire", vjoy.fire);
    _gui.combo_key("A Key", "##key-a", vjoy.a);
    _gui.combo_key("B Key", "##key-b", vjoy.b);
    _gui.combo_key("X Key", "##key-x", vjoy.x);
    _gui.combo_key("Y Key", "##key-y", vjoy.y);
    _gui.combo_key("BACK Key", "##key-back", vjoy.back);
    _gui.combo_key("GUIDE Key", "##key-guide", vjoy.guide);
    _gui.combo_key("START Key", "##key-start", vjoy.start);
    _gui.end_disabled();
}

void ConfigEditor::render_specific()
{
}

uptr_t<ConfigEditor> ConfigEditor::make_editor(Gui& gui, const fs::Path& cfile)
{
#define INSTANTIATOR(editor_type)                                                               \
        [](Gui& gui, const fs::Path& cfile, config::Section&& sec) -> uptr_t<ConfigEditor> {    \
            return std::make_unique<editor_type>(gui, cfile, std::move(sec));                   \
    }

    static const std::unordered_map<std::string, uptr_t<ConfigEditor>(*)(Gui&, const fs::Path&, config::Section&&)> editors{
        { "c64",        INSTANTIATOR(ConfigEditorC64)          },
        { "nes",        INSTANTIATOR(ConfigEditorNES)          },
        { "zx80",       INSTANTIATOR(ConfigEditorZX80)         },
        { "zxspectrum", INSTANTIATOR(ConfigEditorZXSpectrum)   }
    };

    /*
     * Load the configuration file.
     */
    config::Confile cf{cfile};
    config::Section sec = cf.extract(config::SEC_GENERIC);

    /*
     * Get the name of the emulated machine and instantiate the proper configuration editor.
     */
    if (auto mit = sec.find(KEY_MACHINE); mit != sec.end()) {
        const auto& mname = mit->second;
        if (auto eit = editors.find(mname); eit != editors.end()) {
            auto& instantiator = eit->second;
            return instantiator(gui, cfile, std::move(sec));
        }
    }

    return {};
}

template<typename CMDLINE_TYPE>
void default_config(std::string_view description, std::string_view secname)
{
    auto cfname = ConfigEditor::config_path(description);
    if (!fs::exists(cfname)) {
        CMDLINE_TYPE cmdline{};
        config::Section sec = config::parse(0, nullptr, cmdline, false).first;
        sec.emplace(ConfigEditor::KEY_MACHINE, secname);
        config::save(cfname, config::SEC_GENERIC, sec);
    }
}

void ConfigEditor::create_default_configs()
{
    /* The ^ prefix is a hack to mark a default configuration */
    default_config<commodore::c64::C64Cmdline>("^Commodore 64", commodore::c64::SEC_C64);
    default_config<nintendo::nes::NESCmdline>("^NES", nintendo::nes::SEC_NES);
    default_config<sinclair::zx80::ZX80Cmdline>("^Sinclair ZX-80", sinclair::zx80::SEC_ZX80);
    default_config<sinclair::zxspectrum::ZXSpectrumCmdline>("^Sinclair ZX-Spectrum 48K",
        sinclair::zxspectrum::SEC_ZXSPECTRUM);
}

fs::Path ConfigEditor::config_path(const fs::Path& fname)
{
    return std::format("{}/{}.{}", GuiApp::confdir(), fname.string(), FNAME_EXT);
}

}
}
}
