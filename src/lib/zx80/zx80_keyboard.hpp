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
namespace zx80 {

/**
 * ZX80 Keyboard.
 */
class ZX80Keyboard : public Keyboard {
public:
    enum class MatrixKey {
        KEY_1       = 0x0780,
        KEY_2       = 0x0780,
        KEY_3       = 0x0780,
        KEY_4       = 0x0780,
        KEY_5       = 0x0780,
        KEY_6       = 0x0780,
        KEY_7       = 0x0780,
        KEY_8       = 0x0780,
        KEY_9       = 0x0780,
        KEY_0       = 0x0780,
        KEY_Q       = 0x0740,
        KEY_W       = 0x0740,
        KEY_E       = 0x0740,
        KEY_R       = 0x0740,
        KEY_T       = 0x0740,
        KEY_Y       = 0x0740,
        KEY_U       = 0x0740,
        KEY_I       = 0x0740,
        KEY_O       = 0x0740,
        KEY_P       = 0x0740,

        KEY_A       = 0x0740,
        KEY_S       = 0x0740,
        KEY_D       = 0x0740,
        KEY_F       = 0x0740,
        KEY_G       = 0x0740,
        KEY_H       = 0x0740,
        KEY_J       = 0x0740,
        KEY_K       = 0x0740,
        KEY_L       = 0x0740,
        KEY_NEWLINE = 0x0740,

        KEY_SHIFT   = 0x0610,
        KEY_Z       = 0x0480,
        KEY_X       = 0x0480,
        KEY_C       = 0x0480,
        KEY_V       = 0x0480,
        KEY_B       = 0x0480,
        KEY_N       = 0x0480,
        KEY_M       = 0x0480,
        KEY_DOT     = 0x0510,
        KEY_SPACE   = 0x0510,

        KEY_NONE    = -1
    };

    /**
     * Initialise this keyboard.
     * @param label Label assigned to this keyboard.
     */
    ZX80Keyboard(const std::string &label);

    virtual ~ZX80Keyboard();

    /**
     * @see Keyboard::reset()
     */
    void reset() override;

    /**
     * @see Keyboard::key_pressed()
     */
    void key_pressed(Key key) override;

    /**
     * @see Keyboard::key_released()
     */
    void key_released(Key key) override;

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
    void add_key_map(const std::string &key_name, bool key_shift, bool key_altgr, const std::string &impl_name,
        bool impl_shift) override;

    /**
     * @see Keyboard::clear_key_map()
     */
    void clear_key_map() override;

    /**
     * Convert a string to a KeyMatrix.
     * @param name Name of the KeyMatrix key.
     * @return The KeyMatrix code (KeyMatrix::NONE if the key name is invalid).
     * @see name_to_zx80
     */
    static MatrixKey to_zx80(const std::string &name);

private:
    /**
     * Set/Clear a key on the keyboard matrix.
     * @param key The matrix key code;
     * @param set true to set, false to unset.
     * @see _matrix
     */
    void set_matrix(MatrixKey key, bool set = true);

    /**
     * The (negated) row being scanned.
     */
    uint8_t _scanrow{255};

    /**
     * The keyboard matrix.
     */
    std::array<uint8_t, 8> _matrix{};
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
     * Shift status.
     */
    bool _shift{};

    /**
     * Multiple keys can be pressed at once, this variable
     * tracks all of them in order to release them properly.
     */
    std::list<std::tuple<Key, bool, bool>> _prev_keys{};

    /**
     * Conversion table from Key to ZX80 matrix code.
     */
    std::map<std::tuple<Key, bool, bool>, std::pair<MatrixKey, bool>> _key_to_zx80;

    /**
     * Default conversion table from Key to ZX80 matrix code.
     * The default conversion table translates from US-ANSI keyboard to ZX80 keyboard.
     */
    static std::map<std::tuple<Key, bool, bool>, std::pair<MatrixKey, bool>> default_key_to_zx80;

    /**
     * Conversion table from a ZX80 matrix name to a ZX80 matrix code.
     */
    static std::map<std::string, MatrixKey> name_to_zx80;
};

}
}
