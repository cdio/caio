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

#include "types.hpp"
#include "keyboard.hpp"

#include <array>
#include <functional>
#include <list>
#include <map>
#include <mutex>
#include <tuple>
#include <utility>

namespace caio {
namespace commodore {
namespace c64 {

/**
 * C64 Keyboard.
 *
 * ### Keyboard Matrix:
 *
 *     +------------------------------------------------------------------------------+-------+
 *     |                             CIA 1 Port B ($DC01)                             | Joy 2 |
 *     +-------------+----------------------------------------------------------------+-------+
 *     |             | PB7     PB6     PB5     PB4     PB3     PB2     PB1     PB0    |       |
 *     +-------------+----------------------------------------------------------------+-------+
 *     | CIA1    PA7 | STOP    Q       C=      SPACE   2       CTRL    <-      1      |       |
 *     | Port A  PA6 | /       ^       =       RSHIFT  HOME    ;       *       £      |       |
 *     | ($DC00) PA5 | ,       @       :       .       -       L       P       +      |       |
 *     |         PA4 | N       O       K       M       0       J       I       9      | Fire  |
 *     |         PA3 | V       U       H       B       8       G       Y       7      | Right |
 *     |         PA2 | X       T       F       C       6       D       R       5      | Left  |
 *     |         PA1 | LSHIFT  E       S       Z       4       A       W       3      | Down  |
 *     |         PA0 | CRSR DN F5      F3      F1      F7      CRSR RT RETURN  DELETE | Up    |
 *     +-------------+----------------------------------------------------------------+-------+
 *     | Joy 1       |                         Fire    Right   Left    Down    Up     |       |
 *     +-------------+----------------------------------------------------------------+-------+
 *
 * @see <https://www.c64-wiki.com/wiki/Keyboard>
 */
class C64Keyboard : public keyboard::Keyboard {
public:
    constexpr static const unsigned MATRIX_ROWS = 8;

    enum class MatrixKey : uint16_t {
        KEY_RUNSTOP      = 0x0780,
        KEY_Q            = 0x0740,
        KEY_CBM          = 0x0720,
        KEY_SPACE        = 0x0710,
        KEY_2            = 0x0708,
        KEY_CTRL         = 0x0704,
        KEY_LEFT_ARROW   = 0x0702,
        KEY_1            = 0x0701,

        KEY_SLASH        = 0x0680,
        KEY_UP_ARROW     = 0x0640,
        KEY_EQUAL        = 0x0620,
        KEY_RIGHT_SHIFT  = 0x0610,
        KEY_HOME         = 0x0608,
        KEY_SEMICOLON    = 0x0604,
        KEY_ASTERISK     = 0x0602,
        KEY_POUND        = 0x0601,

        KEY_COMMA        = 0x0580,
        KEY_AT           = 0x0540,
        KEY_COLON        = 0x0520,
        KEY_DOT          = 0x0510,
        KEY_MINUS        = 0x0508,
        KEY_L            = 0x0504,
        KEY_P            = 0x0502,
        KEY_PLUS         = 0x0501,

        KEY_N            = 0x0480,
        KEY_O            = 0x0440,
        KEY_K            = 0x0420,
        KEY_M            = 0x0410,
        KEY_0            = 0x0408,
        KEY_J            = 0x0404,
        KEY_I            = 0x0402,
        KEY_9            = 0x0401,

        KEY_V            = 0x0380,
        KEY_U            = 0x0340,
        KEY_H            = 0x0320,
        KEY_B            = 0x0310,
        KEY_8            = 0x0308,
        KEY_G            = 0x0304,
        KEY_Y            = 0x0302,
        KEY_7            = 0x0301,

        KEY_X            = 0x0280,
        KEY_T            = 0x0240,
        KEY_F            = 0x0220,
        KEY_C            = 0x0210,
        KEY_6            = 0x0208,
        KEY_D            = 0x0204,
        KEY_R            = 0x0202,
        KEY_5            = 0x0201,

        KEY_LEFT_SHIFT   = 0x0180,
        KEY_E            = 0x0140,
        KEY_S            = 0x0120,
        KEY_Z            = 0x0110,
        KEY_4            = 0x0108,
        KEY_A            = 0x0104,
        KEY_W            = 0x0102,
        KEY_3            = 0x0101,

        KEY_CURSOR_DOWN  = 0x0080,
        KEY_F5           = 0x0040,
        KEY_F3           = 0x0020,
        KEY_F1           = 0x0010,
        KEY_F7           = 0x0008,
        KEY_CURSOR_RIGHT = 0x0004,
        KEY_RETURN       = 0x0002,
        KEY_DELETE       = 0x0001,

        KEY_NONE         = 0xFFFF
    };

    /**
     * Initialise this C64 keyboard.
     * @param enabled    Enable/disable keyboard;
     * @param restore_cb Callback to call when the RESTORE key is pressed.
     * @see Keyboard::enable(bool)
     */
    C64Keyboard(bool enabled = true, const std::function<void()>& restore_cb = {});

    /**
     * Initialise this C64 keyboard.
     * @param label      Label assigned to this keyboard;
     * @param enabled    Enable/disable keyboard;
     * @param restore_cb Callback to call when the RESTORE key is pressed.
     * @see Keyboard::enable(bool)
     */
    C64Keyboard(std::string_view label, bool enabled = true, const std::function<void()>& restore_cb = {});

    virtual ~C64Keyboard() = default;

    /**
     * Set the RESTORE key callback.
     * @param restore_cb Callback to call when the RESTORE key is pressed.
     */
    void restore_key(const std::function<void()>& restore_cb);

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
     * @see name_to_c64
     */
    static MatrixKey to_c64(const std::string& name);

private:
    /**
     * Set/Clear a key on the keyboard matrix.
     * @param key The matrix key code;
     * @param set true to set, false to unset.
     * @see _matrix
     */
    void set_matrix(MatrixKey key, bool set = true);

    /**
     * RESTORE key callback.
     */
    std::function<void()> _restore_cb{};

    /**
     * The (negated) row being scanned.
     */
    uint8_t _scanrow{255};

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
     * C64 left shift status.
     */
    bool _lshift{};

    /**
     * C64 right shift status.
     */
    bool _rshift{};

    /**
     * Multiple keys can be pressed at once, this variable
     * tracks all of them in order to release them properly.
     */
    std::list<std::tuple<keyboard::Key, bool, bool>> _prev_keys{};

    /**
     * Conversion table from keyboard::Key to C64 matrix code.
     */
    std::map<std::tuple<keyboard::Key, bool, bool>, std::pair<MatrixKey, bool>> _key_to_c64;

    /**
     * Default conversion table from keyboard::Key to C64 matrix code.
     * The default conversion table translates from US-ANSI keyboards to C64 keyboard.
     */
    static std::map<std::tuple<keyboard::Key, bool, bool>, std::pair<MatrixKey, bool>> default_key_to_c64;

    /**
     * Conversion table from a C64 matrix name to a C64 matrix code.
     */
    static std::map<std::string, MatrixKey> name_to_c64;
};

}
}
}
