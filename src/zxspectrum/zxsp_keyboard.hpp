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
#include <list>
#include <map>
#include <mutex>
#include <string>
#include <tuple>
#include <utility>

#include "keyboard.hpp"
#include "types.hpp"

namespace caio {
namespace sinclair {
namespace zxspectrum {

/**
 * ZX-Spectrum Keyboard.
 *
 * Keyboard layout:
 *
 *     +---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+
 *     | blue    | red     |magenta  | green   | cyan    | yellow  | white   |unbright | bright  | black   |
 *     | edit    | caps    |true vid | inv vid |  <-     |   v     |   ^     |  ->     |graphics | delete  |
 *     |  1 !    |  2 @    |  3 #    |  4 $    |  5 %    |  6 &    |  7 ^    |  8 (    |  9 )    |  0 -    |
 *     | def fn  | fn      | line    | open#   | close#  | move    | erase   | point   | cat     | format  |
 *     +---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+
 *          +---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+
 *          | sin     | cos     | tan     | int     | rnd     | str$    | chr$    | code    | peek    | tab     |
 *          | <= plot | <> draw | >= rem  | < run   | > rand  |and retrn| or if   |at input | ; poke  | " print |
 *          |   Q     |   W     |   E     |   R     |   T     |   Y     |   U     |   I     |   O     |   P     |
 *          | asn     | acs     | atn     | verify  | merge   |   [     |   ]     |  in     |  out    |  (c)    |
 *          +---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+
 *              +---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+
 *              | read    | restore | data    | sgn     | abs     | sqr     | val     | len     | usr     |         |
 *              |stop new |not save |step dim | to for  |then goto| ^       | ^ gosub | + list  | =  let  |  ENTER  |
 *              |    A    |    S    |    D    |    F    |    G    |    H    |    J    |    K    |    L    |         |
 *              |    ~    |    |    |    \    |    {    |    }    | cirlcle | val$    | screen$ | attr    |         |
 *              +---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+
 *     +---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+
 *     |         | ln      | exp     | lprint  | llist   | bin     | inkey$  | pi      |         | break   |
 *     | CAPS    | : copy  | £ clear | ? cont  | / cls   | * border| , next  | . pause | SYMBOL  | SPACE   |
 *     |  SHIFT  |    Z    |     X   |    C    |    V    |    B    |    N    |    M    |         |         |
 *     |         | beep    | ink     | paper   | flash   | bright  | over    | inverse |         |         |
 *     +---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+
 *
 * Keyboard matrix:
 *
 *     +----+-----------------------------------------------------------+
 *     |    |   D7     D6     D5     D4     D3     D2     D1     D0     |
 *     +----+-----------------------------------------------------------+
 *     | A0 |   -      -      -      V      C      X      Z      SHIFT  |
 *     | A1 |   -      -      -      G      F      D      S      A      |
 *     | A2 |   -      -      -      T      R      E      W      Q      |
 *     | A3 |   -      -      -      5      4      3      2      1      |
 *     | A4 |   -      -      -      6      7      8      9      0      |
 *     | A5 |   -      -      -      Y      U      I      O      P      |
 *     | A6 |   -      -      -      H      J      K      L      ENTER  |
 *     | A7 |   -      -      -      B      N      M      SYMBOL SPACE  |
 *     +----+-----------------------------------------------------------+
 *
 * Rows are actually connected to address lines A8-A15 but this implementation needs them to be shifted to A0-A7.
 *
 * A0-A7: Row to scan (0=Scan, 1=Do not scan)
 * D0-D4: Keyboard columns (0=Pressed, 1=Released)
 * D5-D7: Unused
 *
 * The ZXSpectrumKeyboard::write(uint8_t) method sets the row to scan.
 * The ZXSpectrumKeyboard::read() method returns the columns associated to the scanned row.
 */
class ZXSpectrumKeyboard : public keyboard::Keyboard {
public:
    constexpr static const uint8_t COLUMN_MASK  = 0x1F;
    constexpr static const unsigned MATRIX_ROWS = 8;

    enum class MatrixKey : uint16_t {
        KEY_1       = 0x0301,
        KEY_2       = 0x0302,
        KEY_3       = 0x0304,
        KEY_4       = 0x0308,
        KEY_5       = 0x0310,
        KEY_6       = 0x0410,
        KEY_7       = 0x0408,
        KEY_8       = 0x0404,
        KEY_9       = 0x0402,
        KEY_0       = 0x0401,
        KEY_Q       = 0x0201,
        KEY_W       = 0x0202,
        KEY_E       = 0x0204,
        KEY_R       = 0x0208,
        KEY_T       = 0x0210,
        KEY_Y       = 0x0510,
        KEY_U       = 0x0508,
        KEY_I       = 0x0504,
        KEY_O       = 0x0502,
        KEY_P       = 0x0501,

        KEY_A       = 0x0101,
        KEY_S       = 0x0102,
        KEY_D       = 0x0104,
        KEY_F       = 0x0108,
        KEY_G       = 0x0110,
        KEY_H       = 0x0610,
        KEY_J       = 0x0608,
        KEY_K       = 0x0604,
        KEY_L       = 0x0602,
        KEY_ENTER   = 0x0601,

        KEY_SHIFT   = 0x0001,
        KEY_Z       = 0x0002,
        KEY_X       = 0x0004,
        KEY_C       = 0x0008,
        KEY_V       = 0x0010,
        KEY_B       = 0x0710,
        KEY_N       = 0x0708,
        KEY_M       = 0x0704,
        KEY_SYMBOL  = 0x0702,
        KEY_SPACE   = 0x0701,

        KEY_NONE    = 0xFFFF
    };

    /**
     * Initialise this keyboard.
     * @param label Label assigned to this keyboard.
     */
    ZXSpectrumKeyboard(const std::string& label);

    virtual ~ZXSpectrumKeyboard() {
    }

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
     * @return The MatrixKey code (MatrixKey::KEY_NONE if the key name is invalid).
     * @see MatrixKey
     * @see name_to_spectrum
     */
    static MatrixKey to_spectrum(const std::string& name);

private:
    /**
     * Set/Clear a key in the keyboard matrix.
     * @param key The matrix key code;
     * @param set true to set, false to clear.
     * @see _matrix
     */
    void set_matrix(MatrixKey key, bool set = true);

    /**
     * (Negated) row to scan.
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
     * ZXSpectrum shift status.
     */
    bool _shift{};

    /**
     * Multiple keys can be pressed at once, this variable
     * tracks all of them in order to release them properly.
     */
    std::list<std::tuple<keyboard::Key, bool, bool>> _prev_keys{};

    /**
     * Conversion table from keyboard::Key to ZXSpectrum matrix code.
     */
    std::map<std::tuple<keyboard::Key, bool, bool>, std::pair<MatrixKey, bool>> _key_to_spectrum;

    /**
     * Default conversion table from keyboard::Key to ZXSpectrum matrix code.
     * The default conversion table translates from US-ANSI keyboard to ZXSpectrum keyboard.
     */
    static std::map<std::tuple<keyboard::Key, bool, bool>, std::pair<MatrixKey, bool>> default_key_to_spectrum;

    /**
     * Conversion table from a ZXSpectrum matrix name to a ZXSpectrum matrix code.
     */
    static std::map<std::string, MatrixKey> name_to_spectrum;
};

}
}
}
