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
#include "editor_nes.hpp"

namespace caio {
namespace ui {
namespace sdl2 {

ConfigEditorNES::ConfigEditorNES(Gui& gui, const fs::Path& cfile, config::Section&& sec)
    : ConfigEditor{gui, cfile, std::move(sec)},
      _conf{_sec},
      _conf_orig{_conf}
{
}

void ConfigEditorNES::save()
{
    _conf.to_section(_sec);
    ConfigEditor::save();
    _conf_orig = _conf;
}

void ConfigEditorNES::render_appearance(config::Config& cfg)
{
    ConfigEditor::render_appearance(cfg);
    _gui.newline();
    _gui.checkbox("NTSC mode", "##ntsc", _conf.ntsc);
}

void ConfigEditorNES::render_specific()
{
    static const char* gamepad_button_list[] = {
        "UP", "DOWN", "LEFT", "RIGHT", "FIRE", "A", "B", "X", "Y", "BACK", "START", "GUIDE"
    };

    _gui.begin_subsection("Controllers:");
    _gui.checkbox("Swap controllers", "##swapj", _conf.swapj);
    _gui.newline();

    auto bt_up = joystick::port_offset_to_string(_conf.buttons.up);
    auto bt_down = joystick::port_offset_to_string(_conf.buttons.down);
    auto bt_left = joystick::port_offset_to_string(_conf.buttons.left);
    auto bt_right = joystick::port_offset_to_string(_conf.buttons.right);
    auto bt_a = joystick::port_offset_to_string(_conf.buttons.a);
    auto bt_b = joystick::port_offset_to_string(_conf.buttons.b);
    auto bt_select = joystick::port_offset_to_string(_conf.buttons.select);
    auto bt_start = joystick::port_offset_to_string(_conf.buttons.start);

    _gui.combo_select("UP button", "##bt-up", gamepad_button_list, std::size(gamepad_button_list), bt_up);
    _gui.combo_select("DOWN button", "##bt-down", gamepad_button_list, std::size(gamepad_button_list), bt_down);
    _gui.combo_select("LEFT button", "##bt-left", gamepad_button_list, std::size(gamepad_button_list), bt_left);
    _gui.combo_select("RIGHT button", "##bt-right", gamepad_button_list, std::size(gamepad_button_list), bt_right);
    _gui.combo_select("A button", "##bt-a", gamepad_button_list, std::size(gamepad_button_list), bt_a);
    _gui.combo_select("B button", "##bt-b", gamepad_button_list, std::size(gamepad_button_list), bt_b);
    _gui.combo_select("SELECT button", "##bt-select", gamepad_button_list, std::size(gamepad_button_list), bt_select);
    _gui.combo_select("START button", "##bt-start", gamepad_button_list, std::size(gamepad_button_list), bt_start);

    _conf.buttons.up = joystick::port_name_to_offset(bt_up);
    _conf.buttons.down = joystick::port_name_to_offset(bt_down);
    _conf.buttons.left = joystick::port_name_to_offset(bt_left);
    _conf.buttons.right = joystick::port_name_to_offset(bt_right);
    _conf.buttons.a = joystick::port_name_to_offset(bt_a);
    _conf.buttons.b = joystick::port_name_to_offset(bt_b);
    _conf.buttons.select = joystick::port_name_to_offset(bt_select);
    _conf.buttons.start = joystick::port_name_to_offset(bt_start);

    _gui.end_subsection();
    _gui.newline();

    _gui.begin_subsection("Attachments:");
    _gui.combo_file("Cartridge", "##cartridge", _conf.cartridge, _cartridge);
    _gui.end_subsection();

    _gui.newline();
}

bool ConfigEditorNES::is_changed() const
{
    return ((_conf != _conf_orig) || ConfigEditor::is_changed());
}

config::Config& ConfigEditorNES::generic_config()
{
    return _conf;
}

std::string ConfigEditorNES::machine_name()
{
    return "NES";
}

std::string ConfigEditorNES::machine_prefix()
{
    return "nes_";
}

}
}
}
