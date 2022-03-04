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
 * Latch register template.
 * The type T determines the size of the register.
 */
template <typename T, std::enable_if_t<std::is_unsigned<T>::value, bool> = true>
class Latch_ {
public:
    /**
     * Initialise this latch register.
     * @param value Initial value (default is 0).
     */
    Latch_(T value = {})
        : _reg{value} {
    }

    virtual ~Latch_() {
    }

    /**
     * Set this latch register value.
     * @param value Value to set.
     * @return this.
     */
    Latch_ &operator=(T value) {
        _reg = value;
        return *this;
    }

    /**
     * @return true if at least one of the bits of this latch register is active; false otherwise.
     */
    operator bool() const {
        return (_reg != 0);
    }

    /**
     * Bitwise AND operator.
     * @param value Value to AND to this latch register.
     * @return A new latch register with the result of the operation.
     */
    Latch_ operator&(T value) {
        return {_reg & value};
    }

    /**
     * Bitwise OR operator.
     * @param value Value to OR to this latch register.
     * @return A new latch register with the result of the operation.
     */
    Latch_ operator|(T value) {
        return {_reg | value};
    }

    /**
     * Bitwise NEG operator.
     * @return A new latch register with the result of the operation.
     */
    Latch_ operator~() const {
        return {~_reg};
    }

    /**
     * Bitwise AND assignment.
     * @parma value Value to AND to this latch register.
     * @return this
     */
    Latch_ &operator&=(T value) {
        _reg &= value;
        return *this;
    }

    /**
     * Bitwise OR assignment.
     * @param value Value to OR to this latch register.
     * @return this
     */
    Latch_ &operator|=(T value) {
        _reg |= value;
        return *this;
    }

private:
    T _reg;
};

/**
 * One bit latch.
 */
using Latch = Latch_<bool>;

/**
 * 8 bits latch register.
 */
using Latch8 = Latch_<uint8_t>;

}
