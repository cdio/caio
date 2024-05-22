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
#include "keyboard.hpp"

#include <iomanip>
#include <fstream>
#include <regex>
#include <sstream>

#include "types.hpp"
#include "utils.hpp"
#include "logger.hpp"

#define KEY_ENTRY(key)  { CAIO_STR(key), key }

namespace caio {
namespace keyboard {

static std::map<std::string, Key> name_to_key{
    KEY_ENTRY(KEY_ESC),
    KEY_ENTRY(KEY_F1),
    KEY_ENTRY(KEY_F2),
    KEY_ENTRY(KEY_F3),
    KEY_ENTRY(KEY_F4),
    KEY_ENTRY(KEY_F5),
    KEY_ENTRY(KEY_F6),
    KEY_ENTRY(KEY_F7),
    KEY_ENTRY(KEY_F8),
    KEY_ENTRY(KEY_F9),
    KEY_ENTRY(KEY_F10),
    KEY_ENTRY(KEY_F11),
    KEY_ENTRY(KEY_F12),
    KEY_ENTRY(KEY_PAUSE),

    KEY_ENTRY(KEY_GRAVE_ACCENT),
    KEY_ENTRY(KEY_1),
    KEY_ENTRY(KEY_2),
    KEY_ENTRY(KEY_3),
    KEY_ENTRY(KEY_4),
    KEY_ENTRY(KEY_5),
    KEY_ENTRY(KEY_6),
    KEY_ENTRY(KEY_7),
    KEY_ENTRY(KEY_8),
    KEY_ENTRY(KEY_9),
    KEY_ENTRY(KEY_0),
    KEY_ENTRY(KEY_MINUS),
    KEY_ENTRY(KEY_EQUAL),
    KEY_ENTRY(KEY_BACKSPACE),

    KEY_ENTRY(KEY_TAB),
    KEY_ENTRY(KEY_Q),
    KEY_ENTRY(KEY_W),
    KEY_ENTRY(KEY_E),
    KEY_ENTRY(KEY_R),
    KEY_ENTRY(KEY_T),
    KEY_ENTRY(KEY_Y),
    KEY_ENTRY(KEY_U),
    KEY_ENTRY(KEY_I),
    KEY_ENTRY(KEY_O),
    KEY_ENTRY(KEY_P),
    KEY_ENTRY(KEY_OPEN_BRACKET),
    KEY_ENTRY(KEY_CLOSE_BRACKET),
    KEY_ENTRY(KEY_BACKSLASH),

    KEY_ENTRY(KEY_A),
    KEY_ENTRY(KEY_S),
    KEY_ENTRY(KEY_D),
    KEY_ENTRY(KEY_F),
    KEY_ENTRY(KEY_G),
    KEY_ENTRY(KEY_H),
    KEY_ENTRY(KEY_J),
    KEY_ENTRY(KEY_K),
    KEY_ENTRY(KEY_L),
    KEY_ENTRY(KEY_SEMICOLON),
    KEY_ENTRY(KEY_APOSTROPHE),
    KEY_ENTRY(KEY_ENTER),

    KEY_ENTRY(KEY_Z),
    KEY_ENTRY(KEY_X),
    KEY_ENTRY(KEY_C),
    KEY_ENTRY(KEY_V),
    KEY_ENTRY(KEY_B),
    KEY_ENTRY(KEY_N),
    KEY_ENTRY(KEY_M),
    KEY_ENTRY(KEY_COMMA),
    KEY_ENTRY(KEY_DOT),
    KEY_ENTRY(KEY_SLASH),

    KEY_ENTRY(KEY_SPACE),

    KEY_ENTRY(KEY_INSERT),
    KEY_ENTRY(KEY_DELETE),
    KEY_ENTRY(KEY_HOME),
    KEY_ENTRY(KEY_END),
    KEY_ENTRY(KEY_PAGE_UP),
    KEY_ENTRY(KEY_PAGE_DOWN),

    KEY_ENTRY(KEY_CURSOR_UP),
    KEY_ENTRY(KEY_CURSOR_DOWN),
    KEY_ENTRY(KEY_CURSOR_LEFT),
    KEY_ENTRY(KEY_CURSOR_RIGHT),

    KEY_ENTRY(KEY_LT),

    KEY_ENTRY(KEY_NUMPAD_SLASH),
    KEY_ENTRY(KEY_NUMPAD_ASTERISK),
    KEY_ENTRY(KEY_NUMPAD_MINUS),
    KEY_ENTRY(KEY_NUMPAD_PLUS),
    KEY_ENTRY(KEY_NUMPAD_ENTER),
    KEY_ENTRY(KEY_NUMPAD_DOT),
    KEY_ENTRY(KEY_NUMPAD_1),
    KEY_ENTRY(KEY_NUMPAD_2),
    KEY_ENTRY(KEY_NUMPAD_3),
    KEY_ENTRY(KEY_NUMPAD_4),
    KEY_ENTRY(KEY_NUMPAD_5),
    KEY_ENTRY(KEY_NUMPAD_6),
    KEY_ENTRY(KEY_NUMPAD_7),
    KEY_ENTRY(KEY_NUMPAD_8),
    KEY_ENTRY(KEY_NUMPAD_9),
    KEY_ENTRY(KEY_NUMPAD_0),

    KEY_ENTRY(KEY_LEFT_SHIFT),
    KEY_ENTRY(KEY_RIGHT_SHIFT),
    KEY_ENTRY(KEY_LEFT_CTRL),
    KEY_ENTRY(KEY_RIGHT_CTRL),
    KEY_ENTRY(KEY_LEFT_ALT),
    KEY_ENTRY(KEY_RIGHT_ALT),
    KEY_ENTRY(KEY_FN)
};

Key to_key(const std::string& name)
{
    auto it = name_to_key.find(name);
    return (it == name_to_key.end() ? Key::KEY_NONE : it->second);
}

std::string to_string(Key key)
{
    auto it = std::find_if(name_to_key.begin(), name_to_key.end(),
        [key](const std::pair<std::string, Key>& pair) {
            return (pair.second == key);
    });

    return (it == name_to_key.end() ? "" : it->first);
}

void Keyboard::load(std::string_view fname)
{
    static const std::regex re_comment{"^[ \t]*#.*$", std::regex::extended};
    static const std::regex re_line{"^[ \t]*([^ \t]+)[ \t]+(SHIFT)?[ \t]*(ALTGR)?[ \t]*([^ \t]+)[ \t]*(SHIFT)?[ \t]*$",
        std::regex::extended};

    clear_key_map();

    //FIXME stdc++ not there yet std::ifstream is{fname, std::ios::binary | std::ios::in};
    std::ifstream is{std::string{fname}, std::ios::binary | std::ios::in};
    if (!is) {
        throw IOError{"Can't open: {}: {}", fname, Error::to_string()};
    }

    std::string line{};
    size_t lineno = 0;

    /*
     * Line format:
     *  "key_name [SHIFT] [ALTGR] impl_key_name [SHIFT]"
     */
    while (std::getline(is, line)) {
        ++lineno;

        std::smatch result{};
        if (line.empty() || std::regex_match(line, result, re_comment)) {
            continue;
        }

        try {
            line = caio::toup(line);
            if (!std::regex_match(line, result, re_line) || result.size() != 6) {
                throw InvalidArgument{};
            }

            const auto& key_name  = result.str(1);
            bool key_shift        = (result.str(2) == "SHIFT" ? true : false);
            bool key_altgr        = (result.str(3) == "ALTGR" ? true : false);
            const auto& impl_name = result.str(4);
            bool impl_shift       = (result.str(5) == "SHIFT" ? true : false);

            add_key_map(key_name, key_shift, key_altgr, impl_name, impl_shift);

        } catch (const InvalidArgument& err) {
            throw InvalidArgument{"{}: Invalid entry at line #{}: \"{}\"", fname, lineno,
                (err.what() == nullptr || *err.what() == '\0' ? line : err.what())};
        }
    }

    is.close();
}

void Keyboard::vjoystick(const VJoyKeys& vjoykeys, const sptr_t<Joystick>& vjoy)
{
    if (_vjoy) {
        _vjoy->reset();
    }

    _vjoy = vjoy;
    if (_vjoy) {
        _vjoy->reset(Joystick::JOYID_VIRTUAL);
        _vjoykeys = vjoykeys;
    }
}

void Keyboard::key_pressed(Key key)
{
    if (_vjoy) {
        const auto& jport = _vjoy->port();
        uint8_t pos = _vjoy->position();

        pos |= (key == _vjoykeys.up     ? jport.up :
               (key == _vjoykeys.down   ? jport.down :
               (key == _vjoykeys.left   ? jport.left :
               (key == _vjoykeys.right  ? jport.right :
               (key == _vjoykeys.fire   ? jport.fire :
               (key == _vjoykeys.fire_b ? jport.fire_b : 0))))));

        _vjoy->position(pos);
    }

    if (_kbd_active) {
        pressed(key);
    }
}

void Keyboard::key_released(Key key)
{
    if (_vjoy) {
        const auto& jport = _vjoy->port();
        uint8_t pos = _vjoy->position();

        pos &= ~(key == _vjoykeys.up     ? jport.up :
                (key == _vjoykeys.down   ? jport.down :
                (key == _vjoykeys.left   ? jport.left :
                (key == _vjoykeys.right  ? jport.right :
                (key == _vjoykeys.fire   ? jport.fire :
                (key == _vjoykeys.fire_b ? jport.fire_b : 0))))));

        _vjoy->position(pos);
    }

    if (_kbd_active) {
        released(key);
    }
}

}
}
