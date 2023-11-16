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

#include "utils.hpp"
#include "logger.hpp"


namespace caio {
namespace keyboard {

static std::map<std::string, Key> name_to_key{
    { "KEY_ESC",                keyboard::KEY_ESC               },
    { "KEY_F1",                 keyboard::KEY_F1                },
    { "KEY_F2",                 keyboard::KEY_F2                },
    { "KEY_F3",                 keyboard::KEY_F3                },
    { "KEY_F4",                 keyboard::KEY_F4                },
    { "KEY_F5",                 keyboard::KEY_F5                },
    { "KEY_F6",                 keyboard::KEY_F6                },
    { "KEY_F7",                 keyboard::KEY_F7                },
    { "KEY_F8",                 keyboard::KEY_F8                },
    { "KEY_F9",                 keyboard::KEY_F9                },
    { "KEY_F10",                keyboard::KEY_F10               },
    { "KEY_F11",                keyboard::KEY_F11               },
    { "KEY_F12",                keyboard::KEY_F12               },
    { "KEY_PAUSE",              keyboard::KEY_PAUSE             },

    { "KEY_GRAVE_ACCENT",       keyboard::KEY_GRAVE_ACCENT      },
    { "KEY_1",                  keyboard::KEY_1                 },
    { "KEY_2",                  keyboard::KEY_2                 },
    { "KEY_3",                  keyboard::KEY_3                 },
    { "KEY_4",                  keyboard::KEY_4                 },
    { "KEY_5",                  keyboard::KEY_5                 },
    { "KEY_6",                  keyboard::KEY_6                 },
    { "KEY_7",                  keyboard::KEY_7                 },
    { "KEY_8",                  keyboard::KEY_8                 },
    { "KEY_9",                  keyboard::KEY_9                 },
    { "KEY_0",                  keyboard::KEY_0                 },
    { "KEY_MINUS",              keyboard::KEY_MINUS             },
    { "KEY_EQUAL",              keyboard::KEY_EQUAL             },
    { "KEY_BACKSPACE",          keyboard::KEY_BACKSPACE         },

    { "KEY_TAB",                keyboard::KEY_TAB               },
    { "KEY_Q",                  keyboard::KEY_Q                 },
    { "KEY_W",                  keyboard::KEY_W                 },
    { "KEY_E",                  keyboard::KEY_E                 },
    { "KEY_R",                  keyboard::KEY_R                 },
    { "KEY_T",                  keyboard::KEY_T                 },
    { "KEY_Y",                  keyboard::KEY_Y                 },
    { "KEY_U",                  keyboard::KEY_U                 },
    { "KEY_I",                  keyboard::KEY_I                 },
    { "KEY_O",                  keyboard::KEY_O                 },
    { "KEY_P",                  keyboard::KEY_P                 },
    { "KEY_OPEN_BRACKET",       keyboard::KEY_OPEN_BRACKET      },
    { "KEY_CLOSE_BRACKET",      keyboard::KEY_CLOSE_BRACKET     },
    { "KEY_BACKSLASH",          keyboard::KEY_BACKSLASH         },

    { "KEY_A",                  keyboard::KEY_A                 },
    { "KEY_S",                  keyboard::KEY_S                 },
    { "KEY_D",                  keyboard::KEY_D                 },
    { "KEY_F",                  keyboard::KEY_F                 },
    { "KEY_G",                  keyboard::KEY_G                 },
    { "KEY_H",                  keyboard::KEY_H                 },
    { "KEY_J",                  keyboard::KEY_J                 },
    { "KEY_K",                  keyboard::KEY_K                 },
    { "KEY_L",                  keyboard::KEY_L                 },
    { "KEY_SEMICOLON",          keyboard::KEY_SEMICOLON         },
    { "KEY_APOSTROPHE",         keyboard::KEY_APOSTROPHE        },
    { "KEY_ENTER",              keyboard::KEY_ENTER             },

    { "KEY_Z",                  keyboard::KEY_Z                 },
    { "KEY_X",                  keyboard::KEY_X                 },
    { "KEY_C",                  keyboard::KEY_C                 },
    { "KEY_V",                  keyboard::KEY_V                 },
    { "KEY_B",                  keyboard::KEY_B                 },
    { "KEY_N",                  keyboard::KEY_N                 },
    { "KEY_M",                  keyboard::KEY_M                 },
    { "KEY_COMMA",              keyboard::KEY_COMMA             },
    { "KEY_DOT",                keyboard::KEY_DOT               },
    { "KEY_SLASH",              keyboard::KEY_SLASH             },

    { "KEY_SPACE",              keyboard::KEY_SPACE             },

    { "KEY_INSERT",             keyboard::KEY_INSERT            },
    { "KEY_DELETE",             keyboard::KEY_DELETE            },
    { "KEY_HOME",               keyboard::KEY_HOME              },
    { "KEY_END",                keyboard::KEY_END               },
    { "KEY_PAGE_UP",            keyboard::KEY_PAGE_UP           },
    { "KEY_PAGE_DOWN",          keyboard::KEY_PAGE_DOWN         },

    { "KEY_CURSOR_UP",          keyboard::KEY_CURSOR_UP         },
    { "KEY_CURSOR_DOWN",        keyboard::KEY_CURSOR_DOWN       },
    { "KEY_CURSOR_LEFT",        keyboard::KEY_CURSOR_LEFT       },
    { "KEY_CURSOR_RIGHT",       keyboard::KEY_CURSOR_RIGHT      },

    { "KEY_LT",                 keyboard::KEY_LT                },

    { "KEY_NUMPAD_SLASH",       keyboard::KEY_NUMPAD_SLASH      },
    { "KEY_NUMPAD_ASTERISK",    keyboard::KEY_NUMPAD_ASTERISK   },
    { "KEY_NUMPAD_MINUS",       keyboard::KEY_NUMPAD_MINUS      },
    { "KEY_NUMPAD_PLUS",        keyboard::KEY_NUMPAD_PLUS       },
    { "KEY_NUMPAD_ENTER",       keyboard::KEY_NUMPAD_ENTER      },
    { "KEY_NUMPAD_DOT",         keyboard::KEY_NUMPAD_DOT        },
    { "KEY_NUMPAD_1",           keyboard::KEY_NUMPAD_1          },
    { "KEY_NUMPAD_2",           keyboard::KEY_NUMPAD_2          },
    { "KEY_NUMPAD_3",           keyboard::KEY_NUMPAD_3          },
    { "KEY_NUMPAD_4",           keyboard::KEY_NUMPAD_4          },
    { "KEY_NUMPAD_5",           keyboard::KEY_NUMPAD_5          },
    { "KEY_NUMPAD_6",           keyboard::KEY_NUMPAD_6          },
    { "KEY_NUMPAD_7",           keyboard::KEY_NUMPAD_7          },
    { "KEY_NUMPAD_8",           keyboard::KEY_NUMPAD_8          },
    { "KEY_NUMPAD_9",           keyboard::KEY_NUMPAD_9          },
    { "KEY_NUMPAD_0",           keyboard::KEY_NUMPAD_0          },

    { "KEY_LEFT_SHIFT",         keyboard::KEY_LEFT_SHIFT        },
    { "KEY_RIGHT_SHIFT",        keyboard::KEY_RIGHT_SHIFT       },
    { "KEY_LEFT_CTRL" ,         keyboard::KEY_LEFT_CTRL         },
    { "KEY_RIGHT_CTRL",         keyboard::KEY_RIGHT_CTRL        },
    { "KEY_LEFT_ALT"  ,         keyboard::KEY_LEFT_ALT          },
    { "KEY_RIGHT_ALT" ,         keyboard::KEY_RIGHT_ALT         },
    { "KEY_FN" ,                keyboard::KEY_FN                }
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

void Keyboard::load(const std::string& fname)
{
    static const std::regex re_comment{"^[ \t]*#.*$", std::regex::extended};
    static const std::regex re_line{"^[ \t]*([^ \t]+)[ \t]+(SHIFT)?[ \t]*(ALTGR)?[ \t]*([^ \t]+)[ \t]*(SHIFT)?[ \t]*$",
        std::regex::extended};

    clear_key_map();

    std::ifstream is{fname, std::ios::binary | std::ios::in};
    if (!is) {
        throw IOError{"Can't open: " + fname + ": " + Error::to_string()};
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
            line = utils::toup(line);
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
            std::stringstream ss{};
            ss << fname << ": Invalid entry at line " << lineno << ": ";
            if (std::string{err.what()}.empty()) {
                ss << std::quoted(line);
            } else {
                ss << err.what();
            }

            throw InvalidArgument{ss.str()};
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
    uint8_t pos = _vjoy->position();

    pos |= (key == _vjoykeys.up    ? Joystick::JOY_UP :
           (key == _vjoykeys.down  ? Joystick::JOY_DOWN :
           (key == _vjoykeys.left  ? Joystick::JOY_LEFT :
           (key == _vjoykeys.right ? Joystick::JOY_RIGHT :
           (key == _vjoykeys.fire  ? Joystick::JOY_FIRE : 0)))));

    _vjoy->position(pos);

    if (_kbd_active) {
        pressed(key);
    }
}

void Keyboard::key_released(Key key)
{
    uint8_t pos = _vjoy->position();

    pos &= ~(key == _vjoykeys.up    ? Joystick::JOY_UP :
            (key == _vjoykeys.down  ? Joystick::JOY_DOWN :
            (key == _vjoykeys.left  ? Joystick::JOY_LEFT :
            (key == _vjoykeys.right ? Joystick::JOY_RIGHT :
            (key == _vjoykeys.fire  ? Joystick::JOY_FIRE : 0)))));

    _vjoy->position(pos);

    if (_kbd_active) {
        released(key);
    }
}

}
}
