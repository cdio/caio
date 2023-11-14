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
#include "c64_keyboard.hpp"

#include "logger.hpp"
#include "utils.hpp"


namespace caio {
namespace commodore {
namespace c64 {

/*
 * +------------------------------------------------------------------------------+-------+
 * |                             CIA 1 Port B ($DC01)                             | Joy 2 |
 * +-------------+----------------------------------------------------------------+-------+
 * |             | PB7     PB6     PB5     PB4     PB3     PB2     PB1     PB0    |       |
 * +-------------+----------------------------------------------------------------+-------+
 * | CIA1    PA7 | STOP    Q       C=      SPACE   2       CTRL    <-      1      |       |
 * | Port A  PA6 | /       ^       =       RSHIFT  HOME    ;       *       £      |       |
 * | ($DC00) PA5 | ,       @       :       .       -       L       P       +      |       |
 * |         PA4 | N       O       K       M       0       J       I       9      | Fire  |
 * |         PA3 | V       U       H       B       8       G       Y       7      | Right |
 * |         PA2 | X       T       F       C       6       D       R       5      | Left  |
 * |         PA1 | LSHIFT  E       S       Z       4       A       W       3      | Down  |
 * |         PA0 | CRSR DN F5      F3      F1      F7      CRSR RT RETURN  DELETE | Up    |
 * +-------------+----------------------------------------------------------------+-------+
 * | Joy 1       |                         Fire    Right   Left    Down    Up     |       |
 * +-------------+----------------------------------------------------------------+-------+
 *
 * Source: https://www.c64-wiki.com/wiki/Keyboard
 */
using MatrixKey = C64Keyboard::MatrixKey;

std::map<std::string, MatrixKey> C64Keyboard::name_to_c64{
    { "KEY_RUNSTOP",      MatrixKey::KEY_RUNSTOP      },
    { "KEY_Q",            MatrixKey::KEY_Q            },
    { "KEY_CBM",          MatrixKey::KEY_CBM          },
    { "KEY_SPACE",        MatrixKey::KEY_SPACE        },
    { "KEY_2",            MatrixKey::KEY_2            },
    { "KEY_CTRL",         MatrixKey::KEY_CTRL         },
    { "KEY_LEFT_ARROW",   MatrixKey::KEY_LEFT_ARROW   },
    { "KEY_1",            MatrixKey::KEY_1            },

    { "KEY_SLASH",        MatrixKey::KEY_SLASH        },
    { "KEY_UP_ARROW",     MatrixKey::KEY_UP_ARROW     },
    { "KEY_EQUAL",        MatrixKey::KEY_EQUAL        },
    { "KEY_RIGHT_SHIFT",  MatrixKey::KEY_RIGHT_SHIFT  },
    { "KEY_HOME",         MatrixKey::KEY_HOME         },
    { "KEY_SEMICOLON",    MatrixKey::KEY_SEMICOLON    },
    { "KEY_ASTERISK",     MatrixKey::KEY_ASTERISK     },
    { "KEY_POUND",        MatrixKey::KEY_POUND        },

    { "KEY_COMMA",        MatrixKey::KEY_COMMA        },
    { "KEY_AT",           MatrixKey::KEY_AT           },
    { "KEY_COLON",        MatrixKey::KEY_COLON        },
    { "KEY_DOT",          MatrixKey::KEY_DOT          },
    { "KEY_MINUS",        MatrixKey::KEY_MINUS        },
    { "KEY_L",            MatrixKey::KEY_L            },
    { "KEY_P",            MatrixKey::KEY_P            },
    { "KEY_PLUS",         MatrixKey::KEY_PLUS         },

    { "KEY_N",            MatrixKey::KEY_N            },
    { "KEY_O",            MatrixKey::KEY_O            },
    { "KEY_K",            MatrixKey::KEY_K            },
    { "KEY_M",            MatrixKey::KEY_M            },
    { "KEY_0",            MatrixKey::KEY_0            },
    { "KEY_J",            MatrixKey::KEY_J            },
    { "KEY_I",            MatrixKey::KEY_I            },
    { "KEY_9",            MatrixKey::KEY_9            },

    { "KEY_V",            MatrixKey::KEY_V            },
    { "KEY_U",            MatrixKey::KEY_U            },
    { "KEY_H",            MatrixKey::KEY_H            },
    { "KEY_B",            MatrixKey::KEY_B            },
    { "KEY_8",            MatrixKey::KEY_8            },
    { "KEY_G",            MatrixKey::KEY_G            },
    { "KEY_Y",            MatrixKey::KEY_Y            },
    { "KEY_7",            MatrixKey::KEY_7            },

    { "KEY_X",            MatrixKey::KEY_X            },
    { "KEY_T",            MatrixKey::KEY_T            },
    { "KEY_F",            MatrixKey::KEY_F            },
    { "KEY_C",            MatrixKey::KEY_C            },
    { "KEY_6",            MatrixKey::KEY_6            },
    { "KEY_D",            MatrixKey::KEY_D            },
    { "KEY_R",            MatrixKey::KEY_R            },
    { "KEY_5",            MatrixKey::KEY_5            },

    { "KEY_LEFT_SHIFT",   MatrixKey::KEY_LEFT_SHIFT   },
    { "KEY_E",            MatrixKey::KEY_E            },
    { "KEY_S",            MatrixKey::KEY_S            },
    { "KEY_Z",            MatrixKey::KEY_Z            },
    { "KEY_4",            MatrixKey::KEY_4            },
    { "KEY_A",            MatrixKey::KEY_A            },
    { "KEY_W",            MatrixKey::KEY_W            },
    { "KEY_3",            MatrixKey::KEY_3            },

    { "KEY_CURSOR_DOWN",  MatrixKey::KEY_CURSOR_DOWN  },
    { "KEY_F5",           MatrixKey::KEY_F5           },
    { "KEY_F3",           MatrixKey::KEY_F3           },
    { "KEY_F1",           MatrixKey::KEY_F1           },
    { "KEY_F7",           MatrixKey::KEY_F7           },
    { "KEY_CURSOR_RIGHT", MatrixKey::KEY_CURSOR_RIGHT },
    { "KEY_RETURN",       MatrixKey::KEY_RETURN       },
    { "KEY_DELETE",       MatrixKey::KEY_DELETE       },
};

std::map<std::tuple<keyboard::Key, bool, bool>, std::pair<MatrixKey, bool>> C64Keyboard::default_key_to_c64{
    {{ keyboard::KEY_ESC,           NONE,   NONE    }, { MatrixKey::KEY_RUNSTOP,        NONE  }},
    {{ keyboard::KEY_ESC,           SHIFT,  NONE    }, { MatrixKey::KEY_RUNSTOP,        SHIFT }},

    {{ keyboard::KEY_F1,            NONE,   NONE    }, { MatrixKey::KEY_F1,             NONE  }},
    {{ keyboard::KEY_F2,            SHIFT,  NONE    }, { MatrixKey::KEY_F1,             SHIFT }},
    {{ keyboard::KEY_F3,            NONE,   NONE    }, { MatrixKey::KEY_F3,             NONE  }},
    {{ keyboard::KEY_F4,            NONE,   NONE    }, { MatrixKey::KEY_F3,             SHIFT }},
    {{ keyboard::KEY_F5,            NONE,   NONE    }, { MatrixKey::KEY_F5,             NONE  }},
    {{ keyboard::KEY_F6,            NONE,   NONE    }, { MatrixKey::KEY_F5,             SHIFT }},
    {{ keyboard::KEY_F7,            NONE,   NONE    }, { MatrixKey::KEY_F7,             NONE  }},
    {{ keyboard::KEY_F8,            NONE,   NONE    }, { MatrixKey::KEY_F7,             SHIFT }},

    {{ keyboard::KEY_GRAVE_ACCENT,  NONE,   NONE    }, { MatrixKey::KEY_LEFT_ARROW,     NONE  }},
    {{ keyboard::KEY_GRAVE_ACCENT,  SHIFT,  NONE    }, { MatrixKey::KEY_LEFT_ARROW,     SHIFT }},

    {{ keyboard::KEY_1,             NONE,   NONE    }, { MatrixKey::KEY_1,              NONE  }},
    {{ keyboard::KEY_2,             NONE,   NONE    }, { MatrixKey::KEY_2,              NONE  }},
    {{ keyboard::KEY_3,             NONE,   NONE    }, { MatrixKey::KEY_3,              NONE  }},
    {{ keyboard::KEY_4,             NONE,   NONE    }, { MatrixKey::KEY_4,              NONE  }},
    {{ keyboard::KEY_5,             NONE,   NONE    }, { MatrixKey::KEY_5,              NONE  }},
    {{ keyboard::KEY_6,             NONE,   NONE    }, { MatrixKey::KEY_6,              NONE  }},
    {{ keyboard::KEY_7,             NONE,   NONE    }, { MatrixKey::KEY_7,              NONE  }},
    {{ keyboard::KEY_8,             NONE,   NONE    }, { MatrixKey::KEY_8,              NONE  }},
    {{ keyboard::KEY_9,             NONE,   NONE    }, { MatrixKey::KEY_9,              NONE  }},
    {{ keyboard::KEY_0,             NONE,   NONE    }, { MatrixKey::KEY_0,              NONE  }},

    {{ keyboard::KEY_1,             SHIFT,  NONE    }, { MatrixKey::KEY_1,              SHIFT }},
    {{ keyboard::KEY_2,             SHIFT,  NONE    }, { MatrixKey::KEY_AT,             NONE  }},
    {{ keyboard::KEY_3,             SHIFT,  NONE    }, { MatrixKey::KEY_3,              SHIFT }},
    {{ keyboard::KEY_4,             SHIFT,  NONE    }, { MatrixKey::KEY_4,              SHIFT }},
    {{ keyboard::KEY_5,             SHIFT,  NONE    }, { MatrixKey::KEY_5,              SHIFT }},
    {{ keyboard::KEY_6,             SHIFT,  NONE    }, { MatrixKey::KEY_UP_ARROW,       NONE  }},
    {{ keyboard::KEY_7,             SHIFT,  NONE    }, { MatrixKey::KEY_6,              SHIFT }},
    {{ keyboard::KEY_8,             SHIFT,  NONE    }, { MatrixKey::KEY_ASTERISK,       NONE  }},
    {{ keyboard::KEY_9,             SHIFT,  NONE    }, { MatrixKey::KEY_8,              SHIFT }},
    {{ keyboard::KEY_0,             SHIFT,  NONE    }, { MatrixKey::KEY_9,              SHIFT }},

    {{ keyboard::KEY_MINUS,         NONE,   NONE    }, { MatrixKey::KEY_MINUS,          NONE  }},
    {{ keyboard::KEY_MINUS,         SHIFT,  NONE    }, { MatrixKey::KEY_LEFT_ARROW,     NONE  }},

    {{ keyboard::KEY_EQUAL,         NONE,   NONE    }, { MatrixKey::KEY_EQUAL,          NONE  }},
    {{ keyboard::KEY_EQUAL,         SHIFT,  NONE    }, { MatrixKey::KEY_PLUS,           NONE  }},

    {{ keyboard::KEY_BACKSPACE,     NONE,   NONE    }, { MatrixKey::KEY_DELETE,         NONE  }},
    {{ keyboard::KEY_BACKSPACE,     SHIFT,  NONE    }, { MatrixKey::KEY_DELETE,         SHIFT }},

    {{ keyboard::KEY_TAB,           NONE,   NONE    }, { MatrixKey::KEY_CTRL,           NONE  }},
    {{ keyboard::KEY_Q,             NONE,   NONE    }, { MatrixKey::KEY_Q,              NONE  }},
    {{ keyboard::KEY_W,             NONE,   NONE    }, { MatrixKey::KEY_W,              NONE  }},
    {{ keyboard::KEY_E,             NONE,   NONE    }, { MatrixKey::KEY_E,              NONE  }},
    {{ keyboard::KEY_R,             NONE,   NONE    }, { MatrixKey::KEY_R,              NONE  }},
    {{ keyboard::KEY_T,             NONE,   NONE    }, { MatrixKey::KEY_T,              NONE  }},
    {{ keyboard::KEY_Y,             NONE,   NONE    }, { MatrixKey::KEY_Y,              NONE  }},
    {{ keyboard::KEY_U,             NONE,   NONE    }, { MatrixKey::KEY_U,              NONE  }},
    {{ keyboard::KEY_I,             NONE,   NONE    }, { MatrixKey::KEY_I,              NONE  }},
    {{ keyboard::KEY_O,             NONE,   NONE    }, { MatrixKey::KEY_O,              NONE  }},
    {{ keyboard::KEY_P,             NONE,   NONE    }, { MatrixKey::KEY_P,              NONE  }},

    {{ keyboard::KEY_TAB,           SHIFT,  NONE    }, { MatrixKey::KEY_CTRL,           SHIFT }},
    {{ keyboard::KEY_Q,             SHIFT,  NONE    }, { MatrixKey::KEY_Q,              SHIFT }},
    {{ keyboard::KEY_W,             SHIFT,  NONE    }, { MatrixKey::KEY_W,              SHIFT }},
    {{ keyboard::KEY_E,             SHIFT,  NONE    }, { MatrixKey::KEY_E,              SHIFT }},
    {{ keyboard::KEY_R,             SHIFT,  NONE    }, { MatrixKey::KEY_R,              SHIFT }},
    {{ keyboard::KEY_T,             SHIFT,  NONE    }, { MatrixKey::KEY_T,              SHIFT }},
    {{ keyboard::KEY_Y,             SHIFT,  NONE    }, { MatrixKey::KEY_Y,              SHIFT }},
    {{ keyboard::KEY_U,             SHIFT,  NONE    }, { MatrixKey::KEY_U,              SHIFT }},
    {{ keyboard::KEY_I,             SHIFT,  NONE    }, { MatrixKey::KEY_I,              SHIFT }},
    {{ keyboard::KEY_O,             SHIFT,  NONE    }, { MatrixKey::KEY_O,              SHIFT }},
    {{ keyboard::KEY_P,             SHIFT,  NONE    }, { MatrixKey::KEY_P,              SHIFT }},

    {{ keyboard::KEY_OPEN_BRACKET,  NONE,   NONE    }, { MatrixKey::KEY_COLON,          SHIFT }},
    {{ keyboard::KEY_CLOSE_BRACKET, NONE,   NONE    }, { MatrixKey::KEY_SEMICOLON,      SHIFT }},
    {{ keyboard::KEY_BACKSLASH,     NONE,   NONE    }, { MatrixKey::KEY_POUND,          NONE  }},
    {{ keyboard::KEY_BACKSLASH,     SHIFT,  NONE    }, { MatrixKey::KEY_POUND,          SHIFT }},

    {{ keyboard::KEY_A,             NONE,   NONE    }, { MatrixKey::KEY_A,              NONE  }},
    {{ keyboard::KEY_S,             NONE,   NONE    }, { MatrixKey::KEY_S,              NONE  }},
    {{ keyboard::KEY_D,             NONE,   NONE    }, { MatrixKey::KEY_D,              NONE  }},
    {{ keyboard::KEY_F,             NONE,   NONE    }, { MatrixKey::KEY_F,              NONE  }},
    {{ keyboard::KEY_G,             NONE,   NONE    }, { MatrixKey::KEY_G,              NONE  }},
    {{ keyboard::KEY_H,             NONE,   NONE    }, { MatrixKey::KEY_H,              NONE  }},
    {{ keyboard::KEY_J,             NONE,   NONE    }, { MatrixKey::KEY_J,              NONE  }},
    {{ keyboard::KEY_K,             NONE,   NONE    }, { MatrixKey::KEY_K,              NONE  }},
    {{ keyboard::KEY_L,             NONE,   NONE    }, { MatrixKey::KEY_L,              NONE  }},
    {{ keyboard::KEY_SEMICOLON,     NONE,   NONE    }, { MatrixKey::KEY_SEMICOLON,      NONE  }},
    {{ keyboard::KEY_APOSTROPHE,    NONE,   NONE    }, { MatrixKey::KEY_7,              SHIFT }},

    {{ keyboard::KEY_A,             SHIFT,  NONE    }, { MatrixKey::KEY_A,              SHIFT }},
    {{ keyboard::KEY_S,             SHIFT,  NONE    }, { MatrixKey::KEY_S,              SHIFT }},
    {{ keyboard::KEY_D,             SHIFT,  NONE    }, { MatrixKey::KEY_D,              SHIFT }},
    {{ keyboard::KEY_F,             SHIFT,  NONE    }, { MatrixKey::KEY_F,              SHIFT }},
    {{ keyboard::KEY_G,             SHIFT,  NONE    }, { MatrixKey::KEY_G,              SHIFT }},
    {{ keyboard::KEY_H,             SHIFT,  NONE    }, { MatrixKey::KEY_H,              SHIFT }},
    {{ keyboard::KEY_J,             SHIFT,  NONE    }, { MatrixKey::KEY_J,              SHIFT }},
    {{ keyboard::KEY_K,             SHIFT , NONE    }, { MatrixKey::KEY_K,              SHIFT }},
    {{ keyboard::KEY_L,             SHIFT,  NONE    }, { MatrixKey::KEY_L,              SHIFT }},
    {{ keyboard::KEY_SEMICOLON,     SHIFT , NONE    }, { MatrixKey::KEY_COLON,          NONE  }},
    {{ keyboard::KEY_APOSTROPHE,    SHIFT,  NONE    }, { MatrixKey::KEY_2,              SHIFT }},

    {{ keyboard::KEY_ENTER,         NONE,   NONE    }, { MatrixKey::KEY_RETURN,         NONE  }},
    {{ keyboard::KEY_ENTER,         SHIFT,  NONE    }, { MatrixKey::KEY_RETURN,         SHIFT }},

    {{ keyboard::KEY_LEFT_SHIFT,    NONE,   NONE    }, { MatrixKey::KEY_LEFT_SHIFT,     NONE  }},

    {{ keyboard::KEY_Z,             NONE,   NONE    }, { MatrixKey::KEY_Z,              NONE  }},
    {{ keyboard::KEY_X,             NONE,   NONE    }, { MatrixKey::KEY_X,              NONE  }},
    {{ keyboard::KEY_C,             NONE,   NONE    }, { MatrixKey::KEY_C,              NONE  }},
    {{ keyboard::KEY_V,             NONE,   NONE    }, { MatrixKey::KEY_V,              NONE  }},
    {{ keyboard::KEY_B,             NONE,   NONE    }, { MatrixKey::KEY_B,              NONE  }},
    {{ keyboard::KEY_N,             NONE,   NONE    }, { MatrixKey::KEY_N,              NONE  }},
    {{ keyboard::KEY_M,             NONE,   NONE    }, { MatrixKey::KEY_M,              NONE  }},
    {{ keyboard::KEY_COMMA,         NONE,   NONE    }, { MatrixKey::KEY_COMMA,          NONE  }},
    {{ keyboard::KEY_DOT,           NONE,   NONE    }, { MatrixKey::KEY_DOT,            NONE  }},
    {{ keyboard::KEY_SLASH,         NONE,   NONE    }, { MatrixKey::KEY_SLASH,          NONE  }},

    {{ keyboard::KEY_Z,             SHIFT,  NONE    }, { MatrixKey::KEY_Z,              SHIFT }},
    {{ keyboard::KEY_X,             SHIFT,  NONE    }, { MatrixKey::KEY_X,              SHIFT }},
    {{ keyboard::KEY_C,             SHIFT,  NONE    }, { MatrixKey::KEY_C,              SHIFT }},
    {{ keyboard::KEY_V,             SHIFT,  NONE    }, { MatrixKey::KEY_V,              SHIFT }},
    {{ keyboard::KEY_B,             SHIFT,  NONE    }, { MatrixKey::KEY_B,              SHIFT }},
    {{ keyboard::KEY_N,             SHIFT,  NONE    }, { MatrixKey::KEY_N,              SHIFT }},
    {{ keyboard::KEY_M,             SHIFT,  NONE    }, { MatrixKey::KEY_M,              SHIFT }},
    {{ keyboard::KEY_COMMA,         SHIFT,  NONE    }, { MatrixKey::KEY_COMMA,          SHIFT }},
    {{ keyboard::KEY_DOT,           SHIFT,  NONE    }, { MatrixKey::KEY_DOT,            SHIFT }},
    {{ keyboard::KEY_SLASH,         SHIFT,  NONE    }, { MatrixKey::KEY_SLASH,          SHIFT }},
    {{ keyboard::KEY_RIGHT_SHIFT,   NONE,   NONE    }, { MatrixKey::KEY_RIGHT_SHIFT,    NONE  }},

    {{ keyboard::KEY_LEFT_CTRL,     NONE,   NONE    }, { MatrixKey::KEY_CBM,            NONE  }},
    {{ keyboard::KEY_SPACE,         NONE,   NONE    }, { MatrixKey::KEY_SPACE,          NONE  }},

    {{ keyboard::KEY_CURSOR_LEFT,   NONE,   NONE    }, { MatrixKey::KEY_CURSOR_RIGHT,   SHIFT }},
    {{ keyboard::KEY_CURSOR_RIGHT,  NONE,   NONE    }, { MatrixKey::KEY_CURSOR_RIGHT,   NONE  }},
    {{ keyboard::KEY_CURSOR_UP,     NONE,   NONE    }, { MatrixKey::KEY_CURSOR_DOWN,    SHIFT }},
    {{ keyboard::KEY_CURSOR_DOWN,   NONE,   NONE    }, { MatrixKey::KEY_CURSOR_DOWN,    NONE  }},

    {{ keyboard::KEY_DELETE,        NONE,   NONE    }, { MatrixKey::KEY_DELETE,         NONE  }},
    {{ keyboard::KEY_DELETE,        SHIFT,  NONE    }, { MatrixKey::KEY_DELETE,         NONE  }},
    {{ keyboard::KEY_INSERT,        NONE,   NONE    }, { MatrixKey::KEY_DELETE,         SHIFT }},
    {{ keyboard::KEY_INSERT,        SHIFT,  NONE    }, { MatrixKey::KEY_DELETE,         SHIFT }},
    {{ keyboard::KEY_HOME,          NONE,   NONE    }, { MatrixKey::KEY_HOME,           NONE  }},
    {{ keyboard::KEY_HOME,          SHIFT,  NONE    }, { MatrixKey::KEY_HOME,           SHIFT }},
    {{ keyboard::KEY_PAGE_DOWN,     NONE,   NONE    }, { MatrixKey::KEY_UP_ARROW,       NONE  }},
    {{ keyboard::KEY_PAGE_DOWN,     SHIFT,  NONE    }, { MatrixKey::KEY_UP_ARROW,       SHIFT }},
};

C64Keyboard::MatrixKey C64Keyboard::to_c64(const std::string& name)
{
    auto it = name_to_c64.find(name);
    return (it == name_to_c64.end() ? MatrixKey::KEY_NONE : it->second);
}

C64Keyboard::C64Keyboard(const std::string& label, const std::function<void()>& restore_cb)
    : Keyboard{label},
      _restore_cb{restore_cb},
      _key_to_c64{default_key_to_c64}
{
    _matrix.fill(0);
}

C64Keyboard::~C64Keyboard()
{
}

void C64Keyboard::restore_key(const std::function<void()>& restore_cb)
{
    _restore_cb = restore_cb;
}

void C64Keyboard::reset()
{
    _matrix.fill(0);
}

void C64Keyboard::pressed(keyboard::Key key)
{
    std::lock_guard<std::mutex> lock{_matrix_mutex};

    if (key == keyboard::KEY_PAGE_UP) {
        /*
         * PAGE-UP as RESTORE key like VICE.
         */
        if (_restore_cb) {
            _restore_cb();
        }

    } else if (key == keyboard::KEY_LEFT_SHIFT) {
        _shift_pressed = _lshift = true;
        set_matrix(MatrixKey::KEY_LEFT_SHIFT, true);

    } else if (key == keyboard::KEY_RIGHT_SHIFT) {
        _shift_pressed = _rshift = true;
        set_matrix(MatrixKey::KEY_RIGHT_SHIFT, true);

    } else if (key == keyboard::KEY_ALT_GR) {
        _altgr_pressed = true;

    } else {
        std::tuple pc_key{key, _shift_pressed, _altgr_pressed};
        auto it = _key_to_c64.find(pc_key);
        if (it != _key_to_c64.end()) {
            auto [c64_key, c64_shift] = it->second;
            set_matrix(MatrixKey::KEY_RIGHT_SHIFT, false);
            set_matrix(MatrixKey::KEY_LEFT_SHIFT, c64_shift);
            set_matrix(c64_key, true);
            _prev_keys.push_back(std::move(pc_key));
        }
    }
}

void C64Keyboard::released(keyboard::Key key)
{
    std::lock_guard<std::mutex> lock{_matrix_mutex};

    if (key == keyboard::KEY_LEFT_SHIFT) {
        _lshift = _shift_pressed = false;
        set_matrix(MatrixKey::KEY_LEFT_SHIFT, false);

    } else if (key == keyboard::KEY_RIGHT_SHIFT) {
        _rshift = _shift_pressed = false;
        set_matrix(MatrixKey::KEY_RIGHT_SHIFT, false);

    } else if (key == keyboard::KEY_ALT_GR) {
        _altgr_pressed = false;

    } else {
        auto pc_key = std::find_if(_prev_keys.begin(), _prev_keys.end(),
            [&key](const std::tuple<keyboard::Key, bool, bool>& elem) -> bool {
                return (std::get<0>(elem) == key);
        });

        if (pc_key != _prev_keys.end()) {
            auto it = _key_to_c64.find(*pc_key);
            if (it != _key_to_c64.end()) {
                auto [c64_key, _] = it->second;
                set_matrix(c64_key, false);
                set_matrix(MatrixKey::KEY_LEFT_SHIFT, _lshift);
                set_matrix(MatrixKey::KEY_RIGHT_SHIFT, _rshift);
                _prev_keys.erase(pc_key);
            }
        }
    }
}

uint8_t C64Keyboard::read()
{
    std::lock_guard<std::mutex> lock{_matrix_mutex};

    uint8_t cols{0};

    for (size_t r = 0; r < std::size(_matrix); ++r) {
        if ((_scanrow & (1 << r)) == 0) {
            cols |= _matrix[r];
        }
    }

    return ~cols;
}

void C64Keyboard::write(uint8_t row)
{
    _scanrow = row;
}

void C64Keyboard::add_key_map(const std::string& key_name, bool key_shift, bool key_altgr, const std::string& impl_name,
    bool impl_shift)
{
    auto key = keyboard::to_key(key_name);
    if (key == keyboard::KEY_NONE) {
        throw InvalidArgument{"Invalid key name: \"" + key_name + "\""};
    }

    MatrixKey impl_key = to_c64(impl_name);
    if (impl_key == MatrixKey::KEY_NONE) {
        throw InvalidArgument{"Invalid C64 key name: \"" + impl_name + "\""};
    }

    std::tuple pc_key{key, key_shift, key_altgr};
    std::pair c64_key{impl_key, impl_shift};

    auto it = _key_to_c64.emplace(pc_key, c64_key);
    if (it.second == false) {
        /* Replace the existing definition */
        it.first->second = c64_key;

        log.warn("C64Keyboard: Key redefined: %s%s%s. Previous value has been replaced\n", key_name.c_str(),
            (key_shift ? " SHIFT" : ""), (key_altgr ? " ALTGR" : ""));
    }
}

void C64Keyboard::clear_key_map()
{
    _key_to_c64.clear();
}

void C64Keyboard::set_matrix(MatrixKey key, bool set)
{
    if (key != MatrixKey::KEY_NONE) {
        uint8_t row = static_cast<uint16_t>(key) >> 8;
        uint8_t col = static_cast<uint16_t>(key) & 0xFF;
        if (row < _matrix.size()) {
            if (set) {
                _matrix[row] |= col;
            } else {
                _matrix[row] &= ~col;
            }
        }
    }
}

}
}
}
