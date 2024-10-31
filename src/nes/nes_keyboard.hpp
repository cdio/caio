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
#pragma once

#include <array>
#include <functional>
#include <list>
#include <map>
#include <mutex>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

#include "types.hpp"
#include "keyboard.hpp"

namespace caio {
namespace nintendo {
namespace nes {

/**
 * Family BASIC Keyboard
 *
 * ## Keyboard Layout:
 *
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
 *   F1  F2  F3  F4  F5  F6  F7  F8
 *   1 2 3 4 5 6 7 8 9 0 - ^ ¥ STOP      CH INS DEL
 *   ESC Q W E R T Y U I O P @ [ RETURN       UP
 *   CTR A S D F G H J K L ; : ] KANA     LEFT RIGHT
 *   SHIFT Z X C V B N M , . / _ SHIFT       DOWN
 *        GRPH   SPACE
 *
 * The default implementation follows a positional mappings, with the exception of:
 *
 *   BACKSLASH \        -> STOP
 *   TAB                -> CTR
 *   ALTGR ;            -> CLOSE BRACKET
 *   ALTGR '            -> KANA
 *   ALTGR /            -> UNDERSCORE
 *   LEFT-CONTROL       -> GRPH
 *   GRAVE ACCENT `     -> YEN
 *
 * ## Keyboard Matrix:
 *
 *    +-----+---------+---------+---------+---------+---------+---------+---------+---------+
 *    |     |               COLUMN 0                |               COLUMN 1                |
 *    | ROW +---------+---------+---------+---------+---------+---------+---------+---------+
 *    |     |   D7    |    D6   |   D5    |   D4    |   D3    |   D2    |   D1    |   D0    |
 *    +-----+---------+---------+---------+---------+---------+---------+---------+---------+
 *    |  0  |    ]    |    [    | RETURN  |   F8    |  STOP   |   YEN   | RSHIFT  |  KANA   |
 *    |  1  |    ;    |    :    |   @     |   F7    |    ^    |    -    |    /    |    _    |
 *    |  2  |    K    |    L    |   O     |   F6    |    0    |    P    |    ,    |    .    |
 *    |  3  |    J    |    U    |   I     |   F5    |    8    |    9    |    N    |    M    |
 *    |  4  |    H    |    G    |   Y     |   F4    |    6    |    7    |    V    |    B    |
 *    |  5  |    D    |    R    |   T     |   F3    |    4    |    5    |    C    |    F    |
 *    |  6  |    A    |    S    |   W     |   F2    |    3    |    E    |    Z    |    X    |
 *    |  7  |   CTR   |    Q    |  ESC    |   F1    |    2    |    1    |  GRPH   | LSHIFT  |
 *    |  8  |  LEFT   |  RIGHT  |   UP    |CLR/HOME |   INS   |   DEL   |  SPACE  |  DOWN   |
 *    +-----+---------+---------+---------+---------+---------+---------+---------+---------+
 *
 * ### Scan procedure:
 *
 * - Write command:
 *
 *   D7 D6 D5 D4 D3 D2 D1 D0
 *    |  |  |  |  |  |  |  |
 *    x  x  x  x  x  |  |  +-> R: Reset keyboard to first row
 *                   |  +----> C: Column select 0 or 1 (see table above);
 *                   |            Row incremented if this bit goes from high to low (and not reset)
 *                   +-------> K: Enable matrix (if 0 all voltages go to 5V, reading back as logical 0)
 *
 * - Read the status of the currently selected row/column:
 *
 *   D7 D6 D5 D4 D3 D2 D1 D0
 *    |  |  |  |  |  |  |  |
 *    x  x  x  |  |  |  |  x
 *             +--+--+--+----> K: Status of currently selected row/column (0: pressed, 1: released).
 *
 * @see https://www.nesdev.org/wiki/Family_BASIC_Keyboard
 */
class NESKeyboard : public keyboard::Keyboard {
public:
    /*
     * The keyboard electronics uses a CD4017 which is a decade
     * counter, with the last counter output (Q9) disconnected.
     */
    constexpr static const unsigned MATRIX_ROWS = 10;

    enum class MatrixKey : uint16_t {
        KEY_CURSOR_LEFT     = 0x0880,
        KEY_CURSOR_RIGHT    = 0x0840,
        KEY_CURSOR_UP       = 0x0820,
        KEY_HOME            = 0x0810,
        KEY_INSERT          = 0x0808,
        KEY_DELETE          = 0x0804,
        KEY_SPACE           = 0x0802,
        KEY_CURSOR_DOWN     = 0x0801,

        KEY_CTR             = 0x0780,
        KEY_Q               = 0x0740,
        KEY_ESC             = 0x0720,
        KEY_F1              = 0x0710,
        KEY_2               = 0x0708,
        KEY_1               = 0x0704,
        KEY_GRPH            = 0x0702,
        KEY_LEFT_SHIFT      = 0x0701,

        KEY_A               = 0x0680,
        KEY_S               = 0x0640,
        KEY_W               = 0x0620,
        KEY_F2              = 0x0610,
        KEY_3               = 0x0608,
        KEY_E               = 0x0604,
        KEY_Z               = 0x0602,
        KEY_X               = 0x0601,

        KEY_D               = 0x0580,
        KEY_R               = 0x0540,
        KEY_T               = 0x0520,
        KEY_F3              = 0x0510,
        KEY_4               = 0x0508,
        KEY_5               = 0x0504,
        KEY_C               = 0x0502,
        KEY_F               = 0x0501,

        KEY_H               = 0x0480,
        KEY_G               = 0x0440,
        KEY_Y               = 0x0420,
        KEY_F4              = 0x0410,
        KEY_6               = 0x0408,
        KEY_7               = 0x0404,
        KEY_V               = 0x0402,
        KEY_B               = 0x0401,

        KEY_J               = 0x0380,
        KEY_U               = 0x0340,
        KEY_I               = 0x0320,
        KEY_F5              = 0x0310,
        KEY_8               = 0x0308,
        KEY_9               = 0x0304,
        KEY_N               = 0x0302,
        KEY_M               = 0x0301,

        KEY_K               = 0x0280,
        KEY_L               = 0x0240,
        KEY_O               = 0x0220,
        KEY_F6              = 0x0210,
        KEY_0               = 0x0208,
        KEY_P               = 0x0204,
        KEY_COMMA           = 0x0202,
        KEY_PERIOD          = 0x0201,

        KEY_SEMICOLON       = 0x0180,
        KEY_COLON           = 0x0140,
        KEY_AT              = 0x0120,
        KEY_F7              = 0x0110,
        KEY_CIRCACCENT      = 0x0108,   /* ^ */
        KEY_MINUS           = 0x0104,
        KEY_SLASH           = 0x0102,
        KEY_UNDERSCORE      = 0x0101,

        KEY_CLOSE_BRACKET   = 0x0080,
        KEY_OPEN_BRACKET    = 0x0040,
        KEY_RETURN          = 0x0020,
        KEY_F8              = 0x0010,
        KEY_STOP            = 0x0008,
        KEY_YEN             = 0x0004,
        KEY_RIGHT_SHIFT     = 0x0002,
        KEY_KANA            = 0x0001,

        KEY_NONE            = 0xFFFF
    };

    /**
     * Initialise this NES keyboard.
     * @param enabled Enable/disable keyboard.
     * @see NESKeyboard(std::string_view, bool)
     */
    NESKeyboard(bool enabled = true);

    /**
     * Initialise this NES keyboard.
     * @param label   Label assigned to this keyboard;
     * @param enabled Enable/disable keyboard.
     * @see Keyboard(std::string_view, bool)
     */
    NESKeyboard(std::string_view label, bool enabled = true);

    virtual ~NESKeyboard();

    /**
     * @see Keyboard::reset()
     */
    void reset() override;

    /**
     * @see Keyboard::pressed()
     */
    void pressed(keyboard::Key key) override;

    /**
     * @see Keyboard::released()
     */
    void released(keyboard::Key key) override;

    /**
     * @see Keyboard::read()
     */
    uint8_t read() override;

    /**
     * @see Keyboard::write()
     */
    void write(uint8_t row) override;

    /**
     * @see Keyboard::add_key_map()
     */
    void add_key_map(const std::string& key_name, bool key_shift, bool key_altgr, const std::string& impl_name,
        bool impl_shift) override;

    /**
     * @see Keyboard::clear_key_map()
     */
    void clear_key_map() override;

    /**
     * Convert a string to a MatrixKey.
     * @param name Name of the MatrixKey key.
     * @return The MatrixKey code (MatrixKey::NONE if the key name is invalid).
     * @see MatrixKey
     * @see name_to_matrix
     */
    static MatrixKey to_matrix(const std::string& name);

private:
    /**
     * Set/Clear a key on the keyboard matrix.
     * @param key The matrix key code;
     * @param set true to set, false to unset.
     * @see _matrix
     */
    void set_matrix(MatrixKey key, bool set = true);

    /**
     * Matrix status (enabled/disabled).
     */
    bool _matrix_en{};

    /**
     * The (negated) row being scanned.
     */
    uint8_t _row{};

    /**
     * Currently selected column.
     */
    bool _column{};

    /**
     * The keyboard matrix.
     */
    std::array<uint8_t, MATRIX_ROWS> _matrix{};
    mutable std::mutex _matrix_mutex{};

    /**
     * PC shift status.
     */
    bool _shift_pressed{};

    /**
     * PC alt-gr status.
     */
    bool _altgr_pressed{};

    /**
     * NES left shift status.
     */
    bool _lshift{};

    /**
     * NES right shift status.
     */
    bool _rshift{};

    /**
     * Multiple keys can be pressed at once, this variable
     * tracks all of them in order to release them properly.
     */
    std::list<std::tuple<keyboard::Key, bool, bool>> _prev_keys{};

    /**
     * Conversion table from keyboard::Key to NES matrix code.
     */
    std::map<std::tuple<keyboard::Key, bool, bool>, std::pair<MatrixKey, bool>> _key_to_matrix;

    /**
     * Default conversion table from keyboard::Key to NES matrix code.
     * The default conversion table translates from US-ANSI keyboards to NES keyboard.
     */
    static std::map<std::tuple<keyboard::Key, bool, bool>, std::pair<MatrixKey, bool>> default_key_to_matrix;

    /**
     * Conversion table from a NES matrix name to a NES matrix code.
     */
    static std::map<std::string, MatrixKey> name_to_matrix;
};

}
}
}
