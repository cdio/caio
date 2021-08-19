/*
 * Copyright (C) 2020 Claudio Castiglia
 *
 * This file is part of CEMU.
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

#include <cstdint>
#include <map>
#include <string>

#include "name.hpp"


namespace cemu {

/**
 * Emulated keyboard.
 * This class must be derived by the specific emulated keyboard.
 */
class Keyboard : public Name {
public:
    constexpr static const char *TYPE = "KBD";

    /**
     * CEMU Key codes.
     * (US ANSI layout + something else).
     */
    enum Key {
        KEY_ESC             = 0x0000,
        KEY_F1              = 0x0001,
        KEY_F2              = 0x0002,
        KEY_F3              = 0x0003,
        KEY_F4              = 0x0004,
        KEY_F5              = 0x0005,
        KEY_F6              = 0x0006,
        KEY_F7              = 0x0007,
        KEY_F8              = 0x0008,
        KEY_F9              = 0x0009,
        KEY_F10             = 0x000A,
        KEY_F11             = 0x000B,
        KEY_F12             = 0x000C,
        KEY_PAUSE           = 0x000D,

        KEY_GRAVE_ACCENT    = 0x000F,
        KEY_1               = 0x0010,
        KEY_2               = 0x0011,
        KEY_3               = 0x0012,
        KEY_4               = 0x0013,
        KEY_5               = 0x0014,
        KEY_6               = 0x0015,
        KEY_7               = 0x0016,
        KEY_8               = 0x0017,
        KEY_9               = 0x0018,
        KEY_0               = 0x0019,
        KEY_MINUS           = 0x001A,
        KEY_EQUAL           = 0x001B,
        KEY_BACKSPACE       = 0x001C,

        KEY_TAB             = 0x001D,
        KEY_Q               = 0x001E,
        KEY_W               = 0x001F,
        KEY_E               = 0x0020,
        KEY_R               = 0x0021,
        KEY_T               = 0x0022,
        KEY_Y               = 0x0023,
        KEY_U               = 0x0024,
        KEY_I               = 0x0025,
        KEY_O               = 0x0026,
        KEY_P               = 0x0027,
        KEY_OPEN_BRACKET    = 0x0028,
        KEY_CLOSE_BRACKET   = 0x0029,
        KEY_BACKSLASH       = 0x002A,

        KEY_A               = 0x002B,
        KEY_S               = 0x002C,
        KEY_D               = 0x002D,
        KEY_F               = 0x002E,
        KEY_G               = 0x002F,
        KEY_H               = 0x0030,
        KEY_J               = 0x0031,
        KEY_K               = 0x0032,
        KEY_L               = 0x0033,
        KEY_SEMICOLON       = 0x0034,
        KEY_APOSTROPHE      = 0x0035,
        KEY_ENTER           = 0x0036,

        KEY_Z               = 0x0037,
        KEY_X               = 0x0038,
        KEY_C               = 0x0039,
        KEY_V               = 0x003A,
        KEY_B               = 0x003B,
        KEY_N               = 0x003C,
        KEY_M               = 0x003D,
        KEY_COMMA           = 0x003E,
        KEY_DOT             = 0x003F,
        KEY_SLASH           = 0x0040,

        KEY_SPACE           = 0x0041,

        KEY_INSERT          = 0x0042,
        KEY_DELETE          = 0x0043,
        KEY_HOME            = 0x0044,
        KEY_END             = 0x0045,
        KEY_PAGE_UP         = 0x0046,
        KEY_PAGE_DOWN       = 0x0047,

        KEY_CURSOR_UP       = 0x0048,
        KEY_CURSOR_DOWN     = 0x0049,
        KEY_CURSOR_LEFT     = 0x004A,
        KEY_CURSOR_RIGHT    = 0x004B,

        KEY_LT              = 0x004D,   /* Missing in US-ANSI */

        KEY_LEFT_SHIFT      = 0x8000,
        KEY_RIGHT_SHIFT     = 0x4000,
        KEY_LEFT_CTRL       = 0x2000,
        KEY_RIGHT_CTRL      = 0x1000,
        KEY_LEFT_ALT        = 0x0800,
        KEY_RIGHT_ALT       = 0x0400,
        KEY_FN              = 0x0200,

        /* Special codes */
        KEY_CTRL_C          = 0x0100,
        KEY_ALT_GR          = KEY_RIGHT_ALT,
        KEY_ALT_J           = KEY_LEFT_ALT | KEY_J,
        KEY_ALT_M           = KEY_LEFT_ALT | KEY_M,
        KEY_NONE            = -1
    };


    Keyboard() {
    }

    virtual ~Keyboard() {
    }

    /**
     * Load a key mapping table from a file.
     * The previous key mappings is cleared.
     * @param fname Name of the file containing the key mappings.
     * @exception IOError
     * @see add_key_map()
     * @see clear_key_map()
     */
    void load(const std::string &fname);

    /**
     * Reset the keyboard.
     */
    virtual void reset() = 0;

    /**
     * Key pressed event.
     * This method must be called by the ui each time
     * a key press event is received from the user.
     * @param key Key code.
     */
    virtual void key_pressed(Key key) = 0;

    /**
     * Key released event.
     * This method must be called by the ui each time
     * a key release event es received from the user.
     * @param key Key code.
     */
    virtual void key_released(Key key) = 0;

    /**
     * Read the column associated to the current scan-row.
     * @return The (negated) column values for the current row.
     */
    virtual uint8_t read() = 0;

    /**
     * Set the scan-row.
     * @param row Negated row to scan.
     */
    virtual void write(uint8_t row) = 0;

    /**
     * Add a new translation code.
     * @param key_name   Name of the Key code;
     * @param key_shift  true if Key code must be shifted, false otherwise;
     * @param key_altgr  true if Key code is an ALT-GR combination; false otherwise:
     * @param impl_name  Name of the specific keyboard implementation code;
     * @param impl_shift true the specific keyboard implementation code must be shifted, false otherwise.
     * @exception InvalidArgument if the key_name or the impl_name is not valid.
     * @see clear_key_map()
     */
    virtual void add_key_map(const std::string &key_name, bool key_shift, bool key_altgr, const std::string &impl_name,
        bool impl_shift) = 0;

    /**
     * Clear the key mappings.
     * @see add_key_map()
     */
    virtual void clear_key_map() = 0;

    /**
     * Convert a Key name to a Key code.
     * @param name Key name.
     * @return The Key code (Key::KEY_NONE if the key name is invalid).
     * @see name_to_key
     */
    static Key to_key(const std::string &name);

protected:
    Keyboard(const std::string &label = {})
        : Name{TYPE, label} {
    }

private:
    static std::map<std::string, Key> name_to_key;
};

}
