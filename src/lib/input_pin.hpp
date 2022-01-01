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


namespace cemu {

/**
 * Input pin.
 * 1 bit GPIO pin.
 */
class InputPin {
public:
    InputPin(bool active = true)
        : _pin{active} {
    }

    ~InputPin() {
    }

    bool operator=(bool value) {
        _pin = value;
        return _pin;
    }

    void set(bool active = true) {
        _pin = active;
    }

    void unset() {
        _pin = false;
    }

    bool is_active() const {
        return _pin;
    }

    operator bool() const {
        return is_active();
    }

private:
    bool _pin;
};

}
