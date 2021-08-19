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
 * IRQ input pin.
 * Anything with an IRQ input pin.
 * This class emulates the effect of pull-up resistors.
 */
class IRQPin {
public:
    IRQPin() {
    }

    virtual ~IRQPin() {
    }

    /**
     * Activate/Deactivate this IRQ pin.
     * This method must be called twice by external peripherals,
     * the first time to activate the pin and a second time to deactivate
     * it when a request was acknowledged.
     * @param active true to activate the pin; false otherwise.
     */
    void trigger(bool active) {
        if (active) {
            ++_pin;
        } else if (_pin != 0) {
            --_pin;
        }
    }

    /**
     * @return true if the IRQ pin is active; false otherwise.
     */
    bool is_active() const {
        return (_pin != 0);
    }

    /**
     * Reset the IRQ pin.
     * This can be used to simulate edge triggered interrupts.
     */
    void reset() {
        _pin = 0;
    }

private:
    /**
     * IRQ pin status.
     * Used a counter to simulate the effect of a pull-up resistor connected to the IRQ pin.
     * - Zero: No interrupts;
     * - Non-zero: At least one source of interrupts is keeping the pin active.
     */
    unsigned _pin{};
};

}
