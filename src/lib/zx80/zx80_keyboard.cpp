/*
 * Copyright (C) 2020-2022 Claudio Castiglia
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
#include "zx80_keyboard.hpp"

#include "logger.hpp"
#include "utils.hpp"


namespace caio {
namespace zx80 {

/*
 *XXX FIXME
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
using MatrixKey = ZX80Keyboard::MatrixKey;

constexpr static const bool SHIFT = true;
constexpr static const bool NONE = false;

std::map<std::string, MatrixKey> ZX80Keyboard::name_to_zx80{
    { "KEY_1",          MatrixKey::KEY_1        },
    { "KEY_2",          MatrixKey::KEY_2        },
    { "KEY_3",          MatrixKey::KEY_3        },
    { "KEY_4",          MatrixKey::KEY_4        },
    { "KEY_5",          MatrixKey::KEY_5        },
    { "KEY_6",          MatrixKey::KEY_6        },
    { "KEY_7",          MatrixKey::KEY_7        },
    { "KEY_8",          MatrixKey::KEY_8        },
    { "KEY_9",          MatrixKey::KEY_9        },
    { "KEY_0",          MatrixKey::KEY_0        },

    { "KEY_Q",          MatrixKey::KEY_Q        },
    { "KEY_W",          MatrixKey::KEY_W        },
    { "KEY_E",          MatrixKey::KEY_E        },
    { "KEY_R",          MatrixKey::KEY_R        },
    { "KEY_T",          MatrixKey::KEY_T        },
    { "KEY_Y",          MatrixKey::KEY_Y        },
    { "KEY_U",          MatrixKey::KEY_U        },
    { "KEY_I",          MatrixKey::KEY_I        },
    { "KEY_O",          MatrixKey::KEY_O        },
    { "KEY_P",          MatrixKey::KEY_P        },

    { "KEY_A",          MatrixKey::KEY_A        },
    { "KEY_S",          MatrixKey::KEY_S        },
    { "KEY_D",          MatrixKey::KEY_D        },
    { "KEY_F",          MatrixKey::KEY_F        },
    { "KEY_G",          MatrixKey::KEY_G        },
    { "KEY_H",          MatrixKey::KEY_H        },
    { "KEY_J",          MatrixKey::KEY_J        },
    { "KEY_K",          MatrixKey::KEY_K        },
    { "KEY_L",          MatrixKey::KEY_L        },
    { "KEY_NEWLINE",    MatrixKey::KEY_NEWLINE  },

    { "KEY_SHIFT",      MatrixKey::KEY_A        },
    { "KEY_Z",          MatrixKey::KEY_S        },
    { "KEY_X",          MatrixKey::KEY_D        },
    { "KEY_C",          MatrixKey::KEY_F        },
    { "KEY_V",          MatrixKey::KEY_G        },
    { "KEY_B",          MatrixKey::KEY_H        },
    { "KEY_N",          MatrixKey::KEY_J        },
    { "KEY_M",          MatrixKey::KEY_K        },
    { "KEY_DOT",        MatrixKey::KEY_L        },
    { "KEY_SPACE",      MatrixKey::KEY_NEWLINE  }
};

std::map<std::tuple<Keyboard::Key, bool, bool>, std::pair<MatrixKey, bool>> ZX80Keyboard::default_key_to_zx80{
    {{ Keyboard::KEY_1,             NONE,   NONE    }, { MatrixKey::KEY_1,              NONE  }},
    {{ Keyboard::KEY_2,             NONE,   NONE    }, { MatrixKey::KEY_2,              NONE  }},
    {{ Keyboard::KEY_3,             NONE,   NONE    }, { MatrixKey::KEY_3,              NONE  }},
    {{ Keyboard::KEY_4,             NONE,   NONE    }, { MatrixKey::KEY_4,              NONE  }},
    {{ Keyboard::KEY_5,             NONE,   NONE    }, { MatrixKey::KEY_5,              NONE  }},
    {{ Keyboard::KEY_6,             NONE,   NONE    }, { MatrixKey::KEY_6,              NONE  }},
    {{ Keyboard::KEY_7,             NONE,   NONE    }, { MatrixKey::KEY_7,              NONE  }},
    {{ Keyboard::KEY_8,             NONE,   NONE    }, { MatrixKey::KEY_8,              NONE  }},
    {{ Keyboard::KEY_9,             NONE,   NONE    }, { MatrixKey::KEY_9,              NONE  }},
    {{ Keyboard::KEY_0,             NONE,   NONE    }, { MatrixKey::KEY_0,              NONE  }},

    {{ Keyboard::KEY_1,             SHIFT,  NONE    }, { MatrixKey::KEY_1,              SHIFT }},
    {{ Keyboard::KEY_2,             SHIFT,  NONE    }, { MatrixKey::KEY_2,              SHIFT }},
    {{ Keyboard::KEY_3,             SHIFT,  NONE    }, { MatrixKey::KEY_3,              SHIFT }},
    {{ Keyboard::KEY_4,             SHIFT,  NONE    }, { MatrixKey::KEY_4,              SHIFT }},
    {{ Keyboard::KEY_CURSOR_LEFT,   NONE,   NONE    }, { MatrixKey::KEY_5,              SHIFT }},
    {{ Keyboard::KEY_CURSOR_UP,     NONE,   NONE    }, { MatrixKey::KEY_6,              SHIFT }},
    {{ Keyboard::KEY_CURSOR_DOWN,   NONE,   NONE    }, { MatrixKey::KEY_7,              SHIFT }},
    {{ Keyboard::KEY_CURSOR_RIGHT,  NONE,   NONE    }, { MatrixKey::KEY_8,              SHIFT }},
    {{ Keyboard::KEY_HOME,          NONE,   NONE    }, { MatrixKey::KEY_9,              SHIFT }},
    {{ Keyboard::KEY_BACKSPACE,     NONE,   NONE    }, { MatrixKey::KEY_0,              SHIFT }},

    {{ Keyboard::KEY_CURSOR_LEFT,   SHIFT,  NONE    }, { MatrixKey::KEY_5,              SHIFT }},
    {{ Keyboard::KEY_CURSOR_UP,     SHIFT,  NONE    }, { MatrixKey::KEY_6,              SHIFT }},
    {{ Keyboard::KEY_CURSOR_DOWN,   SHIFT,  NONE    }, { MatrixKey::KEY_7,              SHIFT }},
    {{ Keyboard::KEY_CURSOR_RIGHT,  SHIFT,  NONE    }, { MatrixKey::KEY_8,              SHIFT }},
    {{ Keyboard::KEY_HOME,          SHIFT,  NONE    }, { MatrixKey::KEY_9,              SHIFT }},
    {{ Keyboard::KEY_BACKSPACE,     SHIFT,  NONE    }, { MatrixKey::KEY_0,              SHIFT }},

    {{ Keyboard::KEY_APOSTROPHE,    SHIFT,  NONE    }, { MatrixKey::KEY_Y,              SHIFT }},
    {{ Keyboard::KEY_4,             SHIFT,  NONE    }, { MatrixKey::KEY_U,              SHIFT }},
    {{ Keyboard::KEY_9,             SHIFT,  NONE    }, { MatrixKey::KEY_I,              SHIFT }},
    {{ Keyboard::KEY_0,             SHIFT,  NONE    }, { MatrixKey::KEY_O,              SHIFT }},
    {{ Keyboard::KEY_8,             SHIFT,  NONE    }, { MatrixKey::KEY_P,              SHIFT }},

    {{ Keyboard::KEY_MINUS,         NONE,   NONE    }, { MatrixKey::KEY_J,              SHIFT }},
    {{ Keyboard::KEY_EQUAL,         SHIFT,  NONE    }, { MatrixKey::KEY_L,              SHIFT }},
    {{ Keyboard::KEY_SEMICOLON,     SHIFT,  NONE    }, { MatrixKey::KEY_Z,              SHIFT }},
    {{ Keyboard::KEY_SEMICOLON,     NONE,   NONE    }, { MatrixKey::KEY_X,              SHIFT }},
    {{ Keyboard::KEY_SLASH,         SHIFT,  NONE    }, { MatrixKey::KEY_C,              SHIFT }},
    {{ Keyboard::KEY_SLASH,         NONE,   NONE    }, { MatrixKey::KEY_V,              SHIFT }},
    {{ Keyboard::KEY_B,             SHIFT,  NONE    }, { MatrixKey::KEY_B,              SHIFT }},
    {{ Keyboard::KEY_COMMA,         SHIFT,  NONE    }, { MatrixKey::KEY_N,              SHIFT }},
    {{ Keyboard::KEY_DOT,           SHIFT,  NONE    }, { MatrixKey::KEY_M,              SHIFT }},
    {{ Keyboard::KEY_DOT,           NONE,   NONE    }, { MatrixKey::KEY_DOT,            NONE  }},
    {{ Keyboard::KEY_COMMA,         NONE,   NONE    }, { MatrixKey::KEY_DOT,            SHIFT }},
    {{ Keyboard::KEY_SPACE,         SHIFT,  NONE    }, { MatrixKey::KEY_SPACE,          SHIFT }},
};


ZX80Keyboard::MatrixKey ZX80Keyboard::to_zx80(const std::string &name)
{
    auto it = name_to_zx80.find(name);
    return (it == name_to_zx80.end() ? MatrixKey::KEY_NONE : it->second);
}

ZX80Keyboard::ZX80Keyboard(const std::string &label)
    : Keyboard{label},
      _key_to_zx80{default_key_to_zx80}
{
    _matrix.fill(0);
}

ZX80Keyboard::~ZX80Keyboard()
{
}

void ZX80Keyboard::reset()
{
    _matrix.fill(0);
}

void ZX80Keyboard::key_pressed(Keyboard::Key key)
{
    std::lock_guard<std::mutex> lock{_matrix_mutex};

    if (key == Key::KEY_LEFT_SHIFT || key == Key::KEY_RIGHT_SHIFT) {
        _shift_pressed = _shift = true;
        set_matrix(MatrixKey::KEY_SHIFT, true);

    } else {
        std::tuple pc_key{key, _shift_pressed, _altgr_pressed};
        auto it = _key_to_zx80.find(pc_key);
        if (it != _key_to_zx80.end()) {
            auto [zx80_key, zx80_shift] = it->second;
            set_matrix(MatrixKey::KEY_SHIFT, zx80_shift);
            set_matrix(zx80_key, true);
            _prev_keys.push_back(std::move(pc_key));
        }
    }
}

void ZX80Keyboard::key_released(Key key)
{
    std::lock_guard<std::mutex> lock{_matrix_mutex};

    if (key == Key::KEY_LEFT_SHIFT || key == Key::KEY_RIGHT_SHIFT) {
        _shift = _shift_pressed = false;
        set_matrix(MatrixKey::KEY_SHIFT, false);

    } else if (key == Key::KEY_ALT_GR) {
        _altgr_pressed = false;

    } else {
        auto pc_key = std::find_if(_prev_keys.begin(), _prev_keys.end(),
            [&key](const std::tuple<Key, bool, bool> &elem) -> bool {
                return (std::get<0>(elem) == key);
        });

        if (pc_key != _prev_keys.end()) {
            auto it = _key_to_zx80.find(*pc_key);
            if (it != _key_to_zx80.end()) {
                auto [zx80_key, _] = it->second;
                set_matrix(zx80_key, false);
                set_matrix(MatrixKey::KEY_SHIFT, _shift);
                _prev_keys.erase(pc_key);
            }
        }
    }
}

uint8_t ZX80Keyboard::read()
{
    std::lock_guard<std::mutex> lock{_matrix_mutex};

    uint8_t cols{0};

    for (size_t r = 0; r < _matrix.size(); ++r) {
        if ((_scanrow & (1 << r)) == 0) {
            cols |= _matrix[r];
        }
    }

    return ~cols;
}

void ZX80Keyboard::write(uint8_t row)
{
    _scanrow = row;
}

void ZX80Keyboard::add_key_map(const std::string &key_name, bool key_shift, bool key_altgr, const std::string &impl_name,
    bool impl_shift)
{
    Key key = Keyboard::to_key(key_name);
    if (key == Key::KEY_NONE) {
        throw InvalidArgument{"Invalid key name: \"" + key_name + "\""};
    }

    MatrixKey impl_key = to_zx80(impl_name);
    if (impl_key == MatrixKey::KEY_NONE) {
        throw InvalidArgument{"Invalid ZX80 key name: \"" + impl_name + "\""};
    }

    std::tuple pc_key{key, key_shift, key_altgr};
    std::pair zx80_key{impl_key, impl_shift};

    auto it = _key_to_zx80.emplace(pc_key, zx80_key);
    if (it.second == false) {
        /* Replace the existing definition */
        it.first->second = zx80_key;

        log.warn("ZX80Keyboard: key redefined: %s%s%s. Previous value has been replaced\n", key_name.c_str(),
            (key_shift ? " SHIFT" : ""), (key_altgr ? " ALTGR" : ""));
    }
}

void ZX80Keyboard::clear_key_map()
{
    _key_to_zx80.clear();
}

void ZX80Keyboard::set_matrix(MatrixKey key, bool set)
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
