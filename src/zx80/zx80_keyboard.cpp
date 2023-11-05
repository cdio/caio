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
#include "zx80_keyboard.hpp"

#include "logger.hpp"
#include "utils.hpp"


namespace caio {
namespace sinclair {
namespace zx80 {

/*
 * ZX80 Keyboard:
 * +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
 * | not   | and   | then  | to    | left  | down  | up    | right | home  | rubout|
 * |       |       |       |       |       |       |       |       |       |       |
 * |   1   |   2   |   3   |   4   |   5   |   6   |   7   |   8   |   9   |   0   |
 * +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
 *      +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
 *      | new   | load  | save  | run   | cont  | rem   | if    | input | print |       |
 *      |   ..  |   ..  |   ..  |   ..  |   ..  |   "   |   $   |   (   |   )   |   *   |
 *      |   Q   |   W   |   E   |   R   |   T   |   Y   |   U   |   I   |   O   |   P   |
 *      +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
 *          +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
 *          | list  | stop  | dim   | for   | goto  | poke  | rand  | let   |       | edit  |
 *          |   ..  |   ..  |   ..  |   ..  |   ..  |   ..  |   -   |   +   |   =   |       |
 *          |   A   |   S   |   D   |   F   |   G   |   H   |   J   |   K   |   L   | ENTER |
 *          +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
 * +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
 * |       |       | clear | cls   | gosub | ret   | next  |       |       | break |
 * |       |   :   |   ;   |   ?   |   /   |   CR  |   <   |   >   |   '   |   £   |
 * | SHIFT |   Z   |   X   |   C   |   V   |   B   |   N   |   M   |   .   | SPACE |
 * +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
 *
 * Keyboard Matrix:
 * +----+---------------------------------------------------+
 * |    | D7    D6    D5    D4    D3    D2    D1    D0      |
 * |    | 80    40    20    10    08    04    02    01      |
 * +----+---------------------------------------------------+
 * | A0 |                   V     C     X     Z     SHIFT   |
 * | A1 |                   G     F     D     S     A       |
 * | A2 |                   T     R     E     W     Q       |
 * | A3 |                   5     4     3     2     1       |
 * | A4 |                   6     7     8     9     0       |
 * | A5 |                   Y     U     I     O     P       |
 * | A6 |                   H     J     K     L     NEWLINE |
 * | A7 |                   B     N     M     .     SPACE   |
 * +----+---------------------------------------------------+
 *
 * A0-A7: Row to scan (0->Scan, 1->Do not scan)
 * D0-D4: Keyboard columns (0->Pressed, 1->Released)
 * D5-D7: 0
 *
 * The write(uint8_t) method sets the (negated) row to scan.
 * The read() method returns the (negated) columns associated to the specified row.
 */
using MatrixKey = ZX80Keyboard::MatrixKey;

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

    { "KEY_SHIFT",      MatrixKey::KEY_SHIFT    },
    { "KEY_Z",          MatrixKey::KEY_Z        },
    { "KEY_X",          MatrixKey::KEY_X        },
    { "KEY_C",          MatrixKey::KEY_C        },
    { "KEY_V",          MatrixKey::KEY_V        },
    { "KEY_B",          MatrixKey::KEY_B        },
    { "KEY_N",          MatrixKey::KEY_N        },
    { "KEY_M",          MatrixKey::KEY_M        },
    { "KEY_DOT",        MatrixKey::KEY_DOT      },
    { "KEY_SPACE",      MatrixKey::KEY_SPACE    },
};

std::map<std::tuple<Keyboard::Key, bool, bool>, std::pair<MatrixKey, bool>> ZX80Keyboard::default_key_to_zx80{
    {{ Keyboard::KEY_1,             NONE,   NONE    }, { MatrixKey::KEY_1,          NONE    }},
    {{ Keyboard::KEY_2,             NONE,   NONE    }, { MatrixKey::KEY_2,          NONE    }},
    {{ Keyboard::KEY_3,             NONE,   NONE    }, { MatrixKey::KEY_3,          NONE    }},
    {{ Keyboard::KEY_4,             NONE,   NONE    }, { MatrixKey::KEY_4,          NONE    }},
    {{ Keyboard::KEY_5,             NONE,   NONE    }, { MatrixKey::KEY_5,          NONE    }},
    {{ Keyboard::KEY_6,             NONE,   NONE    }, { MatrixKey::KEY_6,          NONE    }},
    {{ Keyboard::KEY_7,             NONE,   NONE    }, { MatrixKey::KEY_7,          NONE    }},
    {{ Keyboard::KEY_8,             NONE,   NONE    }, { MatrixKey::KEY_8,          NONE    }},
    {{ Keyboard::KEY_9,             NONE,   NONE    }, { MatrixKey::KEY_9,          NONE    }},
    {{ Keyboard::KEY_0,             NONE,   NONE    }, { MatrixKey::KEY_0,          NONE    }},

    {{ Keyboard::KEY_1,             SHIFT,  NONE    }, { MatrixKey::KEY_1,          SHIFT   }},
    {{ Keyboard::KEY_2,             SHIFT,  NONE    }, { MatrixKey::KEY_2,          SHIFT   }},
    {{ Keyboard::KEY_3,             SHIFT,  NONE    }, { MatrixKey::KEY_3,          SHIFT   }},
    {{ Keyboard::KEY_4,             SHIFT,  NONE    }, { MatrixKey::KEY_4,          SHIFT   }},
    {{ Keyboard::KEY_5,             SHIFT,  NONE    }, { MatrixKey::KEY_5,          SHIFT   }},
    {{ Keyboard::KEY_6,             SHIFT,  NONE    }, { MatrixKey::KEY_6,          SHIFT   }},
    {{ Keyboard::KEY_7,             SHIFT,  NONE    }, { MatrixKey::KEY_7,          SHIFT   }},
    {{ Keyboard::KEY_8,             SHIFT,  NONE    }, { MatrixKey::KEY_8,          SHIFT   }},
    {{ Keyboard::KEY_9,             SHIFT,  NONE    }, { MatrixKey::KEY_9,          SHIFT   }},
    {{ Keyboard::KEY_0,             SHIFT,  NONE    }, { MatrixKey::KEY_0,          SHIFT   }},

    {{ Keyboard::KEY_Q,             NONE,   NONE    }, { MatrixKey::KEY_Q,          NONE    }},
    {{ Keyboard::KEY_W,             NONE,   NONE    }, { MatrixKey::KEY_W,          NONE    }},
    {{ Keyboard::KEY_E,             NONE,   NONE    }, { MatrixKey::KEY_E,          NONE    }},
    {{ Keyboard::KEY_R,             NONE,   NONE    }, { MatrixKey::KEY_R,          NONE    }},
    {{ Keyboard::KEY_T,             NONE,   NONE    }, { MatrixKey::KEY_T,          NONE    }},
    {{ Keyboard::KEY_Y,             NONE,   NONE    }, { MatrixKey::KEY_Y,          NONE    }},
    {{ Keyboard::KEY_U,             NONE,   NONE    }, { MatrixKey::KEY_U,          NONE    }},
    {{ Keyboard::KEY_I,             NONE,   NONE    }, { MatrixKey::KEY_I,          NONE    }},
    {{ Keyboard::KEY_O,             NONE,   NONE    }, { MatrixKey::KEY_O,          NONE    }},
    {{ Keyboard::KEY_P,             NONE,   NONE    }, { MatrixKey::KEY_P,          NONE    }},

    {{ Keyboard::KEY_Q,             SHIFT,  NONE    }, { MatrixKey::KEY_Q,          SHIFT   }},
    {{ Keyboard::KEY_W,             SHIFT,  NONE    }, { MatrixKey::KEY_W,          SHIFT   }},
    {{ Keyboard::KEY_E,             SHIFT,  NONE    }, { MatrixKey::KEY_E,          SHIFT   }},
    {{ Keyboard::KEY_R,             SHIFT,  NONE    }, { MatrixKey::KEY_R,          SHIFT   }},
    {{ Keyboard::KEY_T,             SHIFT,  NONE    }, { MatrixKey::KEY_T,          SHIFT   }},
    {{ Keyboard::KEY_Y,             SHIFT,  NONE    }, { MatrixKey::KEY_Y,          SHIFT   }},
    {{ Keyboard::KEY_U,             SHIFT,  NONE    }, { MatrixKey::KEY_U,          SHIFT   }},
    {{ Keyboard::KEY_I,             SHIFT,  NONE    }, { MatrixKey::KEY_I,          SHIFT   }},
    {{ Keyboard::KEY_O,             SHIFT,  NONE    }, { MatrixKey::KEY_O,          SHIFT   }},
    {{ Keyboard::KEY_P,             SHIFT,  NONE    }, { MatrixKey::KEY_P,          SHIFT   }},

    {{ Keyboard::KEY_A,             NONE,   NONE    }, { MatrixKey::KEY_A,          NONE    }},
    {{ Keyboard::KEY_S,             NONE,   NONE    }, { MatrixKey::KEY_S,          NONE    }},
    {{ Keyboard::KEY_D,             NONE,   NONE    }, { MatrixKey::KEY_D,          NONE    }},
    {{ Keyboard::KEY_F,             NONE,   NONE    }, { MatrixKey::KEY_F,          NONE    }},
    {{ Keyboard::KEY_G,             NONE,   NONE    }, { MatrixKey::KEY_G,          NONE    }},
    {{ Keyboard::KEY_H,             NONE,   NONE    }, { MatrixKey::KEY_H,          NONE    }},
    {{ Keyboard::KEY_J,             NONE,   NONE    }, { MatrixKey::KEY_J,          NONE    }},
    {{ Keyboard::KEY_K,             NONE,   NONE    }, { MatrixKey::KEY_K,          NONE    }},
    {{ Keyboard::KEY_L,             NONE,   NONE    }, { MatrixKey::KEY_L,          NONE    }},
    {{ Keyboard::KEY_ENTER,         NONE,   NONE    }, { MatrixKey::KEY_NEWLINE,    NONE    }},

    {{ Keyboard::KEY_A,             SHIFT,  NONE    }, { MatrixKey::KEY_A,          SHIFT   }},
    {{ Keyboard::KEY_S,             SHIFT,  NONE    }, { MatrixKey::KEY_S,          SHIFT   }},
    {{ Keyboard::KEY_D,             SHIFT,  NONE    }, { MatrixKey::KEY_D,          SHIFT   }},
    {{ Keyboard::KEY_F,             SHIFT,  NONE    }, { MatrixKey::KEY_F,          SHIFT   }},
    {{ Keyboard::KEY_G,             SHIFT,  NONE    }, { MatrixKey::KEY_G,          SHIFT   }},
    {{ Keyboard::KEY_H,             SHIFT,  NONE    }, { MatrixKey::KEY_H,          SHIFT   }},
    {{ Keyboard::KEY_J,             SHIFT,  NONE    }, { MatrixKey::KEY_J,          SHIFT   }},
    {{ Keyboard::KEY_K,             SHIFT,  NONE    }, { MatrixKey::KEY_K,          SHIFT   }},
    {{ Keyboard::KEY_L,             SHIFT,  NONE    }, { MatrixKey::KEY_L,          SHIFT   }},
    {{ Keyboard::KEY_ENTER,         SHIFT,  NONE    }, { MatrixKey::KEY_NEWLINE,    SHIFT   }},

    {{ Keyboard::KEY_LEFT_SHIFT,    NONE,   NONE    }, { MatrixKey::KEY_SHIFT,      NONE    }},
    {{ Keyboard::KEY_Z,             NONE,   NONE    }, { MatrixKey::KEY_Z,          NONE    }},
    {{ Keyboard::KEY_X,             NONE,   NONE    }, { MatrixKey::KEY_X,          NONE    }},
    {{ Keyboard::KEY_C,             NONE,   NONE    }, { MatrixKey::KEY_C,          NONE    }},
    {{ Keyboard::KEY_V,             NONE,   NONE    }, { MatrixKey::KEY_V,          NONE    }},
    {{ Keyboard::KEY_B,             NONE,   NONE    }, { MatrixKey::KEY_B,          NONE    }},
    {{ Keyboard::KEY_N,             NONE,   NONE    }, { MatrixKey::KEY_N,          NONE    }},
    {{ Keyboard::KEY_M,             NONE,   NONE    }, { MatrixKey::KEY_M,          NONE    }},
    {{ Keyboard::KEY_DOT,           NONE,   NONE    }, { MatrixKey::KEY_DOT,        NONE    }},
    {{ Keyboard::KEY_SPACE,         NONE,   NONE    }, { MatrixKey::KEY_SPACE,      NONE    }},

    {{ Keyboard::KEY_RIGHT_SHIFT,   NONE,   NONE    }, { MatrixKey::KEY_SHIFT,      NONE    }},
    {{ Keyboard::KEY_Z,             SHIFT,  NONE    }, { MatrixKey::KEY_Z,          SHIFT   }},
    {{ Keyboard::KEY_X,             SHIFT,  NONE    }, { MatrixKey::KEY_X,          SHIFT   }},
    {{ Keyboard::KEY_C,             SHIFT,  NONE    }, { MatrixKey::KEY_C,          SHIFT   }},
    {{ Keyboard::KEY_V,             SHIFT,  NONE    }, { MatrixKey::KEY_V,          SHIFT   }},
    {{ Keyboard::KEY_B,             SHIFT,  NONE    }, { MatrixKey::KEY_B,          SHIFT   }},
    {{ Keyboard::KEY_N,             SHIFT,  NONE    }, { MatrixKey::KEY_N,          SHIFT   }},
    {{ Keyboard::KEY_M,             SHIFT,  NONE    }, { MatrixKey::KEY_M,          SHIFT   }},
    {{ Keyboard::KEY_DOT,           SHIFT,  NONE    }, { MatrixKey::KEY_DOT,        SHIFT   }},
    {{ Keyboard::KEY_SPACE,         SHIFT,  NONE    }, { MatrixKey::KEY_SPACE,      SHIFT   }},

    {{ Keyboard::KEY_BACKSPACE,     NONE,   NONE    }, { MatrixKey::KEY_0,          SHIFT   }},
    {{ Keyboard::KEY_CURSOR_LEFT,   NONE,   NONE    }, { MatrixKey::KEY_5,          SHIFT   }},
    {{ Keyboard::KEY_CURSOR_RIGHT,  NONE,   NONE    }, { MatrixKey::KEY_8,          SHIFT   }},
    {{ Keyboard::KEY_CURSOR_UP,     NONE,   NONE    }, { MatrixKey::KEY_7,          SHIFT   }},
    {{ Keyboard::KEY_CURSOR_DOWN,   NONE,   NONE    }, { MatrixKey::KEY_6,          SHIFT   }}
};

ZX80Keyboard::MatrixKey ZX80Keyboard::to_zx80(const std::string& name)
{
    auto it = name_to_zx80.find(name);
    return (it == name_to_zx80.end() ? MatrixKey::KEY_NONE : it->second);
}

ZX80Keyboard::ZX80Keyboard(const std::string& label)
    : Keyboard{label},
      _key_to_zx80{default_key_to_zx80}
{
    _matrix.fill(0);
}

void ZX80Keyboard::reset()
{
    std::lock_guard<std::mutex> lock{_matrix_mutex};
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
            [&key](const std::tuple<Key, bool, bool>& elem) -> bool {
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
    uint8_t col = (_scanrow >= std::size(_matrix) ? 0 : _matrix[_scanrow]);
    return (~col);
}

void ZX80Keyboard::write(uint8_t row)
{
    switch (static_cast<uint8_t>(~row)) {
    case 0x01:
        _scanrow = 0;
        break;
    case 0x02:
        _scanrow = 1;
        break;
    case 0x04:
        _scanrow = 2;
        break;
    case 0x08:
        _scanrow = 3;
        break;
    case 0x10:
        _scanrow = 4;
        break;
    case 0x20:
        _scanrow = 5;
        break;
    case 0x40:
        _scanrow = 6;
        break;
    case 0x80:
        _scanrow = 7;
        break;
    default:
        /* Don't accept attempts to read more than one row at a time */
        _scanrow = 255;
    }
}

void ZX80Keyboard::add_key_map(const std::string& key_name, bool key_shift, bool key_altgr,
    const std::string& impl_name, bool impl_shift)
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

        log.warn("ZX80Keyboard: Key redefined: %s%s%s. Previous value has been replaced\n", key_name.c_str(),
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
}
