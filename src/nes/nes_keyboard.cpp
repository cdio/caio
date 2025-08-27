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
#include "nes_keyboard.hpp"

#include "logger.hpp"
#include "utils.hpp"

namespace caio {
namespace nintendo {
namespace nes {

using MatrixKey = NESKeyboard::MatrixKey;

#define NAME_MATRIX(name)   { CAIO_STR(name),   MatrixKey::name }

std::map<std::string, MatrixKey> NESKeyboard::name_to_matrix{
    NAME_MATRIX(KEY_F1),
    NAME_MATRIX(KEY_F2),
    NAME_MATRIX(KEY_F3),
    NAME_MATRIX(KEY_F4),
    NAME_MATRIX(KEY_F5),
    NAME_MATRIX(KEY_F6),
    NAME_MATRIX(KEY_F7),
    NAME_MATRIX(KEY_F8),

    NAME_MATRIX(KEY_1),
    NAME_MATRIX(KEY_2),
    NAME_MATRIX(KEY_3),
    NAME_MATRIX(KEY_4),
    NAME_MATRIX(KEY_5),
    NAME_MATRIX(KEY_6),
    NAME_MATRIX(KEY_7),
    NAME_MATRIX(KEY_8),
    NAME_MATRIX(KEY_9),
    NAME_MATRIX(KEY_0),
    NAME_MATRIX(KEY_MINUS),
    NAME_MATRIX(KEY_CIRCACCENT),
    NAME_MATRIX(KEY_YEN),
    NAME_MATRIX(KEY_STOP),

    NAME_MATRIX(KEY_ESC),
    NAME_MATRIX(KEY_Q),
    NAME_MATRIX(KEY_W),
    NAME_MATRIX(KEY_E),
    NAME_MATRIX(KEY_R),
    NAME_MATRIX(KEY_T),
    NAME_MATRIX(KEY_Y),
    NAME_MATRIX(KEY_U),
    NAME_MATRIX(KEY_I),
    NAME_MATRIX(KEY_O),
    NAME_MATRIX(KEY_P),
    NAME_MATRIX(KEY_AT),
    NAME_MATRIX(KEY_OPEN_BRACKET),
    NAME_MATRIX(KEY_RETURN),

    NAME_MATRIX(KEY_CTR),
    NAME_MATRIX(KEY_A),
    NAME_MATRIX(KEY_S),
    NAME_MATRIX(KEY_D),
    NAME_MATRIX(KEY_F),
    NAME_MATRIX(KEY_G),
    NAME_MATRIX(KEY_H),
    NAME_MATRIX(KEY_J),
    NAME_MATRIX(KEY_K),
    NAME_MATRIX(KEY_L),
    NAME_MATRIX(KEY_SEMICOLON),
    NAME_MATRIX(KEY_COLON),
    NAME_MATRIX(KEY_CLOSE_BRACKET),
    NAME_MATRIX(KEY_KANA),

    NAME_MATRIX(KEY_LEFT_SHIFT),
    NAME_MATRIX(KEY_Z),
    NAME_MATRIX(KEY_X),
    NAME_MATRIX(KEY_C),
    NAME_MATRIX(KEY_V),
    NAME_MATRIX(KEY_B),
    NAME_MATRIX(KEY_N),
    NAME_MATRIX(KEY_M),
    NAME_MATRIX(KEY_COMMA),
    NAME_MATRIX(KEY_PERIOD),
    NAME_MATRIX(KEY_SLASH),
    NAME_MATRIX(KEY_UNDERSCORE),
    NAME_MATRIX(KEY_RIGHT_SHIFT),
    NAME_MATRIX(KEY_GRPH),
    NAME_MATRIX(KEY_SPACE),
    NAME_MATRIX(KEY_HOME),
    NAME_MATRIX(KEY_INSERT),
    NAME_MATRIX(KEY_DELETE),

    NAME_MATRIX(KEY_CURSOR_UP),
    NAME_MATRIX(KEY_CURSOR_DOWN),
    NAME_MATRIX(KEY_CURSOR_LEFT),
    NAME_MATRIX(KEY_CURSOR_RIGHT)
};

/*
 *    +--------+ +--------+ +--------+ +--------+ +--------+ +--------+ +--------+ +--------+
 *    |        | |        | |        | |        | |        | |        | |        | |        |
 *    |   F1   | |  F2    | |   F3   | |   F4   | |   F5   | |   F6   | |   F7   | |   F8   |
 *    |        | |        | |        | |        | |        | |        | |        | |        |
 *    +--------+ +--------+ +--------+ +--------+ +--------+ +--------+ +--------+ +--------+
 *      +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +------+
 *      |  !  | |  "  | |  #  | |  $  | |  %  | |  &  | |  '  | |  (  | |  )  | |     | |  =  | |     | |     | |      |
 *      | 1 ァ| | 2 ィ| | 3 ゥ| | 4 ェ| | 5 ォ| | 6   | | 7   | | 8   | | 9   | | 0   | | -   | | ^   | | ¥   | | STOP |
 *      |  ア | |  イ | |  ウ | |  エ | |  オ | |  ナ | |  ニ | |  ヌ | |  ネ | |  ノ | |  ラ | |  リ | |  ル | |      |      +------+ +-----+ +-----+
 *      +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +------+      | CLR  | | INS | | DEL |
 *   +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +----------+     | HOME | |     | |     |
 *   |     | |     | |     | |     | |     | |     | |     | |     | |     | |     | |     | |     | |     | |          |     +------+ +-----+ +-----+
 *   | ESC | | Q   | | W   | | E   | | R   | | T   | | Y パ| | U ピ| | I プ| | O ペ| | P ポ| | @   | | [ 「| |  RETURN  |
 *   |     | |  カ | |  キ | |  ク | |  ケ | |  コ | |  ハ | | ヒ  | |  フ | |  ヘ | |  ホ | |  レ | |  ロ | |          |            +--------+
 *   +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +----------+            |   UP   |
 *     +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+               +--------+
 *     |     | |     | |     | |     | |     | |     | |     | |     | |     | |     | |  +  | |  *  | |     | |     |         +--------+ +--------+
 *     | CTR | | A   | | S   | | D   | | F   | | G   | | H   | | J   | | K   | | L   | | ;   | | :   | | ] 」| | カナ|         |  LEFT  | | RIGHT  |
 *     |     | |  サ | |  シ | |  ス | |  セ | |  ソ | |  マ | |  ミ | |  ム | |  メ | |  モ   |  ー | |  。 | |     |         +--------+ +--------+
 *     +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+               +--------+
 *   +-------+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-------+                     |  DOWN  |
 *   |       | |     | |     | |     | |     | |     | |     | |     | |  <  | |  >  | |  ?  | |  ␣  | |       |                     +--------+
 *   | SHIFT | | Z   | | X   | | C   | | V   | | B   | | N   | | M   | | ,   | | .   | | /   | |     | | SHIFT |
 *   |       | |  タ | |  チ | |  ツ | |  テ | |  ト | |  ヤ | |  ユ | |  ヨ | |  ワ | |  ヲ | |  ン | |       |
 *   +-------+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-------+
 *                    +------+ +-------------------------------------------------------------+
 *                    |      | |                                                             |
 *                    | GRPH | |                         SPACE                               |
 *                    |      | |                                                             |
 *                    +------+ +-------------------------------------------------------------+
 *
 *     F1  F2  F3  F4  F5  F6  F7  F8
 *      1 2 3 4 5 6 7 8 9 0 - ^ ¥ STOP      CH INS DEL
 *     ESC Q W E R T Y U I O P @ [ RETURN       UP
 *     CTR A S D F G H J K L ; : ] KANA     LEFT RIGHT
 *     SHIFT Z X C V B N M , . / _ SHIFT       DOWN
 *            GRPH   SPACE
 *
 * Positional mappings with the exception of:
 *
 *   BACKSLASH \ -> STOP
 *   ALTGR ;     -> CLOSE BRACKET
 *   ALTGR '     -> KANA
 *   ALTGR /     -> UNDERSCORE
 *   COMMAND     -> GRPH
 */
std::map<std::tuple<keyboard::Key, bool, bool>, std::pair<MatrixKey, bool>> NESKeyboard::default_key_to_matrix{
    {{ keyboard::KEY_F1,            NONE,   NONE    }, { MatrixKey::KEY_F1,             NONE  }},
    {{ keyboard::KEY_F2,            NONE,   NONE    }, { MatrixKey::KEY_F2,             NONE  }},
    {{ keyboard::KEY_F3,            NONE,   NONE    }, { MatrixKey::KEY_F3,             NONE  }},
    {{ keyboard::KEY_F4,            NONE,   NONE    }, { MatrixKey::KEY_F4,             NONE  }},
    {{ keyboard::KEY_F5,            NONE,   NONE    }, { MatrixKey::KEY_F5,             NONE  }},
    {{ keyboard::KEY_F6,            NONE,   NONE    }, { MatrixKey::KEY_F6,             NONE  }},
    {{ keyboard::KEY_F7,            NONE,   NONE    }, { MatrixKey::KEY_F7,             NONE  }},
    {{ keyboard::KEY_F8,            NONE,   NONE    }, { MatrixKey::KEY_F8,             NONE  }},

    {{ keyboard::KEY_F1,            SHIFT,  NONE    }, { MatrixKey::KEY_F1,             SHIFT }},
    {{ keyboard::KEY_F2,            SHIFT,  NONE    }, { MatrixKey::KEY_F2,             SHIFT }},
    {{ keyboard::KEY_F3,            SHIFT,  NONE    }, { MatrixKey::KEY_F4,             SHIFT }},
    {{ keyboard::KEY_F4,            SHIFT,  NONE    }, { MatrixKey::KEY_F4,             SHIFT }},
    {{ keyboard::KEY_F5,            SHIFT,  NONE    }, { MatrixKey::KEY_F5,             SHIFT }},
    {{ keyboard::KEY_F6,            SHIFT,  NONE    }, { MatrixKey::KEY_F6,             SHIFT }},
    {{ keyboard::KEY_F7,            SHIFT,  NONE    }, { MatrixKey::KEY_F7,             SHIFT }},
    {{ keyboard::KEY_F8,            SHIFT,  NONE    }, { MatrixKey::KEY_F8,             SHIFT }},

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
    {{ keyboard::KEY_MINUS,         NONE,   NONE    }, { MatrixKey::KEY_MINUS,          NONE  }},
    {{ keyboard::KEY_EQUAL,         NONE,   NONE    }, { MatrixKey::KEY_CIRCACCENT,     NONE  }},
    {{ keyboard::KEY_GRAVE_ACCENT,  NONE,   NONE    }, { MatrixKey::KEY_YEN,            NONE  }},
    {{ keyboard::KEY_BACKSLASH,     NONE,   NONE    }, { MatrixKey::KEY_STOP,           NONE  }},

    {{ keyboard::KEY_1,             SHIFT,  NONE    }, { MatrixKey::KEY_1,              SHIFT }},
    {{ keyboard::KEY_2,             SHIFT,  NONE    }, { MatrixKey::KEY_2,              SHIFT }},
    {{ keyboard::KEY_3,             SHIFT,  NONE    }, { MatrixKey::KEY_3,              SHIFT }},
    {{ keyboard::KEY_4,             SHIFT,  NONE    }, { MatrixKey::KEY_4,              SHIFT }},
    {{ keyboard::KEY_5,             SHIFT,  NONE    }, { MatrixKey::KEY_5,              SHIFT }},
    {{ keyboard::KEY_6,             SHIFT,  NONE    }, { MatrixKey::KEY_6,              SHIFT }},
    {{ keyboard::KEY_7,             SHIFT,  NONE    }, { MatrixKey::KEY_7,              SHIFT }},
    {{ keyboard::KEY_8,             SHIFT,  NONE    }, { MatrixKey::KEY_8,              SHIFT }},
    {{ keyboard::KEY_9,             SHIFT,  NONE    }, { MatrixKey::KEY_9,              SHIFT }},
    {{ keyboard::KEY_0,             SHIFT,  NONE    }, { MatrixKey::KEY_0,              SHIFT }},
    {{ keyboard::KEY_MINUS,         SHIFT,  NONE    }, { MatrixKey::KEY_MINUS,          SHIFT }},
    {{ keyboard::KEY_EQUAL,         SHIFT,  NONE    }, { MatrixKey::KEY_CIRCACCENT,     SHIFT }},
    {{ keyboard::KEY_GRAVE_ACCENT,  SHIFT,  NONE    }, { MatrixKey::KEY_YEN,            SHIFT }},
    {{ keyboard::KEY_BACKSLASH,     SHIFT,  NONE    }, { MatrixKey::KEY_STOP,           SHIFT }},

    {{ keyboard::KEY_ESC,           NONE,   NONE    }, { MatrixKey::KEY_ESC,            NONE  }},
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
    {{ keyboard::KEY_OPEN_BRACKET,  NONE,   NONE    }, { MatrixKey::KEY_AT,             NONE  }},
    {{ keyboard::KEY_CLOSE_BRACKET, NONE,   NONE    }, { MatrixKey::KEY_OPEN_BRACKET,   NONE  }},
    {{ keyboard::KEY_ENTER,         NONE,   NONE    }, { MatrixKey::KEY_RETURN,         NONE  }},

    {{ keyboard::KEY_ESC,           SHIFT,  NONE    }, { MatrixKey::KEY_ESC,            SHIFT }},
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
    {{ keyboard::KEY_OPEN_BRACKET,  SHIFT,  NONE    }, { MatrixKey::KEY_AT,             SHIFT }},
    {{ keyboard::KEY_CLOSE_BRACKET, SHIFT,  NONE    }, { MatrixKey::KEY_OPEN_BRACKET,   SHIFT }},
    {{ keyboard::KEY_ENTER,         SHIFT,  NONE    }, { MatrixKey::KEY_RETURN,         SHIFT }},

    {{ keyboard::KEY_TAB,           NONE,   NONE    }, { MatrixKey::KEY_CTR,            NONE  }},
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
    {{ keyboard::KEY_APOSTROPHE,    NONE,   NONE    }, { MatrixKey::KEY_COLON,          NONE  }},
    {{ keyboard::KEY_SEMICOLON,     NONE,   ALTGR   }, { MatrixKey::KEY_CLOSE_BRACKET,  NONE  }},
    {{ keyboard::KEY_APOSTROPHE,    NONE,   ALTGR   }, { MatrixKey::KEY_KANA,           NONE  }},

    {{ keyboard::KEY_TAB,           SHIFT,  NONE    }, { MatrixKey::KEY_CTR,            SHIFT }},
    {{ keyboard::KEY_A,             SHIFT,  NONE    }, { MatrixKey::KEY_A,              SHIFT }},
    {{ keyboard::KEY_S,             SHIFT,  NONE    }, { MatrixKey::KEY_S,              SHIFT }},
    {{ keyboard::KEY_D,             SHIFT,  NONE    }, { MatrixKey::KEY_D,              SHIFT }},
    {{ keyboard::KEY_F,             SHIFT,  NONE    }, { MatrixKey::KEY_F,              SHIFT }},
    {{ keyboard::KEY_G,             SHIFT,  NONE    }, { MatrixKey::KEY_G,              SHIFT }},
    {{ keyboard::KEY_H,             SHIFT,  NONE    }, { MatrixKey::KEY_H,              SHIFT }},
    {{ keyboard::KEY_J,             SHIFT,  NONE    }, { MatrixKey::KEY_J,              SHIFT }},
    {{ keyboard::KEY_K,             SHIFT,  NONE    }, { MatrixKey::KEY_K,              SHIFT }},
    {{ keyboard::KEY_L,             SHIFT,  NONE    }, { MatrixKey::KEY_L,              SHIFT }},
    {{ keyboard::KEY_SEMICOLON,     SHIFT,  NONE    }, { MatrixKey::KEY_SEMICOLON,      SHIFT }},
    {{ keyboard::KEY_APOSTROPHE,    SHIFT,  NONE    }, { MatrixKey::KEY_COLON,          SHIFT }},
    {{ keyboard::KEY_SEMICOLON,     SHIFT,  ALTGR   }, { MatrixKey::KEY_CLOSE_BRACKET,  SHIFT }},
    {{ keyboard::KEY_APOSTROPHE,    SHIFT,  ALTGR   }, { MatrixKey::KEY_KANA,           SHIFT }},

    {{ keyboard::KEY_LEFT_SHIFT,    NONE,   NONE    }, { MatrixKey::KEY_LEFT_SHIFT,     NONE  }},
    {{ keyboard::KEY_Z,             NONE,   NONE    }, { MatrixKey::KEY_Z,              NONE  }},
    {{ keyboard::KEY_X,             NONE,   NONE    }, { MatrixKey::KEY_X,              NONE  }},
    {{ keyboard::KEY_C,             NONE,   NONE    }, { MatrixKey::KEY_C,              NONE  }},
    {{ keyboard::KEY_V,             NONE,   NONE    }, { MatrixKey::KEY_V,              NONE  }},
    {{ keyboard::KEY_B,             NONE,   NONE    }, { MatrixKey::KEY_B,              NONE  }},
    {{ keyboard::KEY_N,             NONE,   NONE    }, { MatrixKey::KEY_N,              NONE  }},
    {{ keyboard::KEY_M,             NONE,   NONE    }, { MatrixKey::KEY_M,              NONE  }},
    {{ keyboard::KEY_COMMA,         NONE,   NONE    }, { MatrixKey::KEY_COMMA,          NONE  }},
    {{ keyboard::KEY_DOT,           NONE,   NONE    }, { MatrixKey::KEY_PERIOD,         NONE  }},
    {{ keyboard::KEY_SLASH,         NONE,   NONE    }, { MatrixKey::KEY_SLASH,          NONE  }},
    {{ keyboard::KEY_SLASH,         NONE,   ALTGR   }, { MatrixKey::KEY_UNDERSCORE,     NONE  }},
    {{ keyboard::KEY_RIGHT_SHIFT,   NONE,   NONE    }, { MatrixKey::KEY_RIGHT_SHIFT,    NONE  }},

    {{ keyboard::KEY_Z,             SHIFT,  NONE    }, { MatrixKey::KEY_Z,              SHIFT }},
    {{ keyboard::KEY_X,             SHIFT,  NONE    }, { MatrixKey::KEY_X,              SHIFT }},
    {{ keyboard::KEY_C,             SHIFT,  NONE    }, { MatrixKey::KEY_C,              SHIFT }},
    {{ keyboard::KEY_V,             SHIFT,  NONE    }, { MatrixKey::KEY_V,              SHIFT }},
    {{ keyboard::KEY_B,             SHIFT,  NONE    }, { MatrixKey::KEY_B,              SHIFT }},
    {{ keyboard::KEY_N,             SHIFT,  NONE    }, { MatrixKey::KEY_N,              SHIFT }},
    {{ keyboard::KEY_M,             SHIFT,  NONE    }, { MatrixKey::KEY_M,              SHIFT }},
    {{ keyboard::KEY_COMMA,         SHIFT,  NONE    }, { MatrixKey::KEY_COMMA,          SHIFT }},
    {{ keyboard::KEY_DOT,           SHIFT,  NONE    }, { MatrixKey::KEY_PERIOD,         SHIFT }},
    {{ keyboard::KEY_SLASH,         SHIFT,  NONE    }, { MatrixKey::KEY_SLASH,          SHIFT }},
    {{ keyboard::KEY_SLASH,         SHIFT,  ALTGR   }, { MatrixKey::KEY_UNDERSCORE,     SHIFT }},

    {{ keyboard::KEY_LEFT_CTRL,     NONE,   NONE    }, { MatrixKey::KEY_GRPH,           NONE  }},
    {{ keyboard::KEY_LEFT_CTRL,     SHIFT,  NONE    }, { MatrixKey::KEY_GRPH,           SHIFT }},

    {{ keyboard::KEY_SPACE,         NONE,   NONE    }, { MatrixKey::KEY_SPACE,          NONE  }},
    {{ keyboard::KEY_SPACE,         SHIFT,  NONE    }, { MatrixKey::KEY_SPACE,          SHIFT }},

    {{ keyboard::KEY_HOME,          NONE,   NONE    }, { MatrixKey::KEY_HOME,           NONE  }},
    {{ keyboard::KEY_INSERT,        NONE,   NONE    }, { MatrixKey::KEY_INSERT,         NONE  }},
    {{ keyboard::KEY_DELETE,        NONE,   NONE    }, { MatrixKey::KEY_DELETE,         NONE  }},
    {{ keyboard::KEY_BACKSPACE,     NONE,   NONE    }, { MatrixKey::KEY_DELETE,         NONE  }},

    {{ keyboard::KEY_HOME,          SHIFT,  NONE    }, { MatrixKey::KEY_HOME,           SHIFT }},
    {{ keyboard::KEY_INSERT,        SHIFT,  NONE    }, { MatrixKey::KEY_INSERT,         SHIFT }},
    {{ keyboard::KEY_DELETE,        SHIFT,  NONE    }, { MatrixKey::KEY_DELETE,         SHIFT }},
    {{ keyboard::KEY_BACKSPACE,     SHIFT,  NONE    }, { MatrixKey::KEY_DELETE,         SHIFT }},

    {{ keyboard::KEY_CURSOR_UP,     NONE,   NONE    }, { MatrixKey::KEY_CURSOR_UP,      NONE  }},
    {{ keyboard::KEY_CURSOR_DOWN,   NONE,   NONE    }, { MatrixKey::KEY_CURSOR_DOWN,    NONE  }},
    {{ keyboard::KEY_CURSOR_LEFT,   NONE,   NONE    }, { MatrixKey::KEY_CURSOR_LEFT,    NONE  }},
    {{ keyboard::KEY_CURSOR_RIGHT,  NONE,   NONE    }, { MatrixKey::KEY_CURSOR_RIGHT,   NONE  }},

    {{ keyboard::KEY_CURSOR_UP,     SHIFT,  NONE    }, { MatrixKey::KEY_CURSOR_UP,      SHIFT }},
    {{ keyboard::KEY_CURSOR_DOWN,   SHIFT,  NONE    }, { MatrixKey::KEY_CURSOR_DOWN,    SHIFT }},
    {{ keyboard::KEY_CURSOR_LEFT,   SHIFT,  NONE    }, { MatrixKey::KEY_CURSOR_LEFT,    SHIFT }},
    {{ keyboard::KEY_CURSOR_RIGHT,  SHIFT,  NONE    }, { MatrixKey::KEY_CURSOR_RIGHT,   SHIFT }},
};

NESKeyboard::MatrixKey NESKeyboard::to_matrix(const std::string& name)
{
    auto it = name_to_matrix.find(name);
    return (it == name_to_matrix.end() ? MatrixKey::KEY_NONE : it->second);
}

NESKeyboard::NESKeyboard(bool enabled)
    : NESKeyboard{LABEL, enabled}
{
}

NESKeyboard::NESKeyboard(std::string_view label, bool enabled)
    : Keyboard{label, enabled},
      _key_to_matrix{default_key_to_matrix}
{
    _matrix.fill(0);
}

void NESKeyboard::reset()
{
    _matrix.fill(0);
}

void NESKeyboard::pressed(keyboard::Key key)
{
    std::lock_guard<std::mutex> lock{_matrix_mutex};

    if (key == keyboard::KEY_LEFT_SHIFT) {
        _shift_pressed = _lshift = true;
        set_matrix(MatrixKey::KEY_LEFT_SHIFT, true);

    } else if (key == keyboard::KEY_RIGHT_SHIFT) {
        _shift_pressed = _rshift = true;
        set_matrix(MatrixKey::KEY_RIGHT_SHIFT, true);

    } else if (key == keyboard::KEY_ALT_GR) {
        _altgr_pressed = true;

    } else {
        std::tuple pc_key{key, _shift_pressed, _altgr_pressed};
        auto it = _key_to_matrix.find(pc_key);
        if (it != _key_to_matrix.end()) {
            auto [nes_key, nes_shift] = it->second;
            set_matrix(MatrixKey::KEY_RIGHT_SHIFT, false);
            set_matrix(MatrixKey::KEY_LEFT_SHIFT, nes_shift);
            set_matrix(nes_key, true);
            _prev_keys.push_back(std::move(pc_key));
        }
    }
}

void NESKeyboard::released(keyboard::Key key)
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
            auto it = _key_to_matrix.find(*pc_key);
            if (it != _key_to_matrix.end()) {
                auto [nes_key, _] = it->second;
                set_matrix(nes_key, false);
                set_matrix(MatrixKey::KEY_LEFT_SHIFT, _lshift);
                set_matrix(MatrixKey::KEY_RIGHT_SHIFT, _rshift);
                _prev_keys.erase(pc_key);
            }
        }
    }
}

uint8_t NESKeyboard::read()
{
    std::lock_guard<std::mutex> lock{_matrix_mutex};

    if (_matrix_en) {
        const auto cols = _matrix[_row];
        return ~(_column ? (cols & (D3 | D2 | D1 | D0)) << 1 : (cols & (D7 | D6 | D5 | D4)) >> 3);
    }

    return 0;
}

void NESKeyboard::write(uint8_t opcode)
{
    constexpr static const uint8_t RST = D0;
    constexpr static const uint8_t COL = D1;
    constexpr static const uint8_t EN  = D2;

    _matrix_en = (opcode & EN);

    if (_matrix_en) {
        const bool col_select = (opcode & COL);
        const bool row_reset = (opcode & RST);
        if (row_reset) {
            _row = 0;
        } else if (!col_select && _column) {
            /*
             * Row incremented if column moved from 1 to 0 and not reset.
             */
            _row = (_row + 1) % MATRIX_ROWS;
        }

        _column = col_select;
    }
}

void NESKeyboard::add_key_map(const std::string& key_name, bool key_shift, bool key_altgr, const std::string& impl_name,
    bool impl_shift)
{
    auto key = keyboard::to_key(key_name);
    if (key == keyboard::KEY_NONE) {
        throw InvalidArgument{"Invalid key name: \"{}\"", key_name};
    }

    MatrixKey impl_key = to_matrix(impl_name);
    if (impl_key == MatrixKey::KEY_NONE) {
        throw InvalidArgument{"Invalid NES key name: \"{}\"", impl_name};
    }

    std::tuple pc_key{key, key_shift, key_altgr};
    std::pair nes_key{impl_key, impl_shift};

    auto it = _key_to_matrix.emplace(pc_key, nes_key);
    if (it.second == false) {
        /* Replace the existing definition */
        it.first->second = nes_key;

        log.warn("NESKeyboard: Redefined key: {}{}{}. Previous value has been replaced\n", key_name,
            (key_shift ? " SHIFT" : ""), (key_altgr ? " ALTGR" : ""));
    }
}

void NESKeyboard::clear_key_map()
{
    _key_to_matrix.clear();
}

void NESKeyboard::set_matrix(MatrixKey key, bool set)
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
