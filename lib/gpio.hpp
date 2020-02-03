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
#include <functional>


namespace cemu {

/**
 * Generic Gpio.
 * A Gpio exposes an 8 bits address range where data can be written to or read from.
 * Read and write operations are done through user defined callbacks.
 * This class is used as a glue to interconnect components exposing I/O pins.
 */
class Gpio {
public:
    using ior_t = std::function<uint8_t(uint8_t)>;
    using iow_t = std::function<void(uint8_t, uint8_t)>;

    Gpio(ior_t ior = {}, iow_t iow = {})
        : _ior{ior},
          _iow{iow} {
    }

    virtual ~Gpio() {
    }

    /**
     * Set the Gpio pins callbacks.
     * The ior callback must implement the proper device behaviour (pull-up, floating pin, etc).
     * @param ior Method to call when input I/O pins are read;
     * @param iow Method to call when output I/O pins are written.
     */
    void gpio(ior_t ior, iow_t iow) {
        _ior = ior;
        _iow = iow;
    }

    /**
     * Read from an input Gpio.
     * @param addr Address to read from.
     * @return The Gpio value.
     */
    uint8_t ior(uint8_t addr) const {
        return (_ior ? _ior(addr) : 0);
    }

    /**
     * Write to an output Gpio.
     * @param addr  Address to write;
     * @param value Value to write.
     */
    void iow(uint8_t addr, uint8_t value) {
        if (_iow) {
            _iow(addr, value);
        }
    }

private:
    ior_t _ior{};
    iow_t _iow{};
};

}
