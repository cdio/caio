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


namespace caio {

std::map<std::string, Keyboard::Key> Keyboard::name_to_key{
    { "KEY_ESC",                Keyboard::KEY_ESC               },
    { "KEY_F1",                 Keyboard::KEY_F1                },
    { "KEY_F2",                 Keyboard::KEY_F2                },
    { "KEY_F3",                 Keyboard::KEY_F3                },
    { "KEY_F4",                 Keyboard::KEY_F4                },
    { "KEY_F5",                 Keyboard::KEY_F5                },
    { "KEY_F6",                 Keyboard::KEY_F6                },
    { "KEY_F7",                 Keyboard::KEY_F7                },
    { "KEY_F8",                 Keyboard::KEY_F8                },
    { "KEY_F9",                 Keyboard::KEY_F9                },
    { "KEY_F10",                Keyboard::KEY_F10               },
    { "KEY_F11",                Keyboard::KEY_F11               },
    { "KEY_F12",                Keyboard::KEY_F12               },
    { "KEY_PAUSE",              Keyboard::KEY_PAUSE             },

    { "KEY_GRAVE_ACCENT",       Keyboard::KEY_GRAVE_ACCENT      },
    { "KEY_1",                  Keyboard::KEY_1                 },
    { "KEY_2",                  Keyboard::KEY_2                 },
    { "KEY_3",                  Keyboard::KEY_3                 },
    { "KEY_4",                  Keyboard::KEY_4                 },
    { "KEY_5",                  Keyboard::KEY_5                 },
    { "KEY_6",                  Keyboard::KEY_6                 },
    { "KEY_7",                  Keyboard::KEY_7                 },
    { "KEY_8",                  Keyboard::KEY_8                 },
    { "KEY_9",                  Keyboard::KEY_9                 },
    { "KEY_0",                  Keyboard::KEY_0                 },
    { "KEY_MINUS",              Keyboard::KEY_MINUS             },
    { "KEY_EQUAL",              Keyboard::KEY_EQUAL             },
    { "KEY_BACKSPACE",          Keyboard::KEY_BACKSPACE         },

    { "KEY_TAB",                Keyboard::KEY_TAB               },
    { "KEY_Q",                  Keyboard::KEY_Q                 },
    { "KEY_W",                  Keyboard::KEY_W                 },
    { "KEY_E",                  Keyboard::KEY_E                 },
    { "KEY_R",                  Keyboard::KEY_R                 },
    { "KEY_T",                  Keyboard::KEY_T                 },
    { "KEY_Y",                  Keyboard::KEY_Y                 },
    { "KEY_U",                  Keyboard::KEY_U                 },
    { "KEY_I",                  Keyboard::KEY_I                 },
    { "KEY_O",                  Keyboard::KEY_O                 },
    { "KEY_P",                  Keyboard::KEY_P                 },
    { "KEY_OPEN_BRACKET",       Keyboard::KEY_OPEN_BRACKET      },
    { "KEY_CLOSE_BRACKET",      Keyboard::KEY_CLOSE_BRACKET     },
    { "KEY_BACKSLASH",          Keyboard::KEY_BACKSLASH         },

    { "KEY_A",                  Keyboard::KEY_A                 },
    { "KEY_S",                  Keyboard::KEY_S                 },
    { "KEY_D",                  Keyboard::KEY_D                 },
    { "KEY_F",                  Keyboard::KEY_F                 },
    { "KEY_G",                  Keyboard::KEY_G                 },
    { "KEY_H",                  Keyboard::KEY_H                 },
    { "KEY_J",                  Keyboard::KEY_J                 },
    { "KEY_K",                  Keyboard::KEY_K                 },
    { "KEY_L",                  Keyboard::KEY_L                 },
    { "KEY_SEMICOLON",          Keyboard::KEY_SEMICOLON         },
    { "KEY_APOSTROPHE",         Keyboard::KEY_APOSTROPHE        },
    { "KEY_ENTER",              Keyboard::KEY_ENTER             },

    { "KEY_Z",                  Keyboard::KEY_Z                 },
    { "KEY_X",                  Keyboard::KEY_X                 },
    { "KEY_C",                  Keyboard::KEY_C                 },
    { "KEY_V",                  Keyboard::KEY_V                 },
    { "KEY_B",                  Keyboard::KEY_B                 },
    { "KEY_N",                  Keyboard::KEY_N                 },
    { "KEY_M",                  Keyboard::KEY_M                 },
    { "KEY_COMMA",              Keyboard::KEY_COMMA             },
    { "KEY_DOT",                Keyboard::KEY_DOT               },
    { "KEY_SLASH",              Keyboard::KEY_SLASH             },

    { "KEY_SPACE",              Keyboard::KEY_SPACE             },

    { "KEY_INSERT",             Keyboard::KEY_INSERT            },
    { "KEY_DELETE",             Keyboard::KEY_DELETE            },
    { "KEY_HOME",               Keyboard::KEY_HOME              },
    { "KEY_END",                Keyboard::KEY_END               },
    { "KEY_PAGE_UP",            Keyboard::KEY_PAGE_UP           },
    { "KEY_PAGE_DOWN",          Keyboard::KEY_PAGE_DOWN         },

    { "KEY_CURSOR_UP",          Keyboard::KEY_CURSOR_UP         },
    { "KEY_CURSOR_DOWN",        Keyboard::KEY_CURSOR_DOWN       },
    { "KEY_CURSOR_LEFT",        Keyboard::KEY_CURSOR_LEFT       },
    { "KEY_CURSOR_RIGHT",       Keyboard::KEY_CURSOR_RIGHT      },

    { "KEY_LT",                 Keyboard::KEY_LT                },

    { "KEY_NUMPAD_SLASH",       Keyboard::KEY_NUMPAD_SLASH      },
    { "KEY_NUMPAD_ASTERISK",    Keyboard::KEY_NUMPAD_ASTERISK   },
    { "KEY_NUMPAD_MINUS",       Keyboard::KEY_NUMPAD_MINUS      },
    { "KEY_NUMPAD_PLUS",        Keyboard::KEY_NUMPAD_PLUS       },
    { "KEY_NUMPAD_ENTER",       Keyboard::KEY_NUMPAD_ENTER      },
    { "KEY_NUMPAD_DOT",         Keyboard::KEY_NUMPAD_DOT        },
    { "KEY_NUMPAD_1",           Keyboard::KEY_NUMPAD_1          },
    { "KEY_NUMPAD_2",           Keyboard::KEY_NUMPAD_2          },
    { "KEY_NUMPAD_3",           Keyboard::KEY_NUMPAD_3          },
    { "KEY_NUMPAD_4",           Keyboard::KEY_NUMPAD_4          },
    { "KEY_NUMPAD_5",           Keyboard::KEY_NUMPAD_5          },
    { "KEY_NUMPAD_6",           Keyboard::KEY_NUMPAD_6          },
    { "KEY_NUMPAD_7",           Keyboard::KEY_NUMPAD_7          },
    { "KEY_NUMPAD_8",           Keyboard::KEY_NUMPAD_8          },
    { "KEY_NUMPAD_9",           Keyboard::KEY_NUMPAD_9          },
    { "KEY_NUMPAD_0",           Keyboard::KEY_NUMPAD_0          },

    { "KEY_LEFT_SHIFT",         Keyboard::KEY_LEFT_SHIFT        },
    { "KEY_RIGHT_SHIFT",        Keyboard::KEY_RIGHT_SHIFT       },
    { "KEY_LEFT_CTRL" ,         Keyboard::KEY_LEFT_CTRL         },
    { "KEY_RIGHT_CTRL",         Keyboard::KEY_RIGHT_CTRL        },
    { "KEY_LEFT_ALT"  ,         Keyboard::KEY_LEFT_ALT          },
    { "KEY_RIGHT_ALT" ,         Keyboard::KEY_RIGHT_ALT         },
    { "KEY_FN" ,                Keyboard::KEY_FN                }
};

Keyboard::Key Keyboard::to_key(const std::string& name)
{
    auto it = name_to_key.find(name);
    return (it == name_to_key.end() ? Key::KEY_NONE : it->second);
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

}
