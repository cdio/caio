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
 * Pull-Up.
 * Emulate the effect of a pull-up resistor where several devices can drive
 * the line low (active).
 * A hardware pull-up pin defaults to 1 (not-active), other outputs
 * connected to this line can drive it low by setting a 0 output value.
 * This behaviour is emulated by incrementing a counter each time a connected
 * output is active and decrementing the counter when that output is not-active.
 * Using this mechanism a pull-up is active (0) when the counter is not 0
 * and not-active (1) when the counter is 0.
 */
class PullUp {
public:
    PullUp() {
    }

    virtual ~PullUp() {
    }

    /**
     * Activate/Deactivate this pull-up.
     * This method *must* be called twice by the connected outputs,
     * the first time to activate it and the second time to deactivate it.
     * @param value true to activate this pull-up (drive it low), false to de-activate it (drive it high).
     * @return this
     */
    PullUp &operator=(bool value) {
        /* Impemented this way to emulate edge triggered interrupts */
        if (value) {
            ++_count;
        } else if (_count) {
            --_count;
        }
        return *this;
    }

    /**
     * @return true if this pull-up is active (it is being driven
     * low by at least one connected output); false otherwise.
     */
    operator bool() const {
        return _count;
    }

    /**
     * Reset this pull-up (de-activate or drive it high).
     */
    void reset() {
        _count = 0;
    }

private:
    unsigned _count{};
};

}
