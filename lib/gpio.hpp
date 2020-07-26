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
#include <utility>
#include <vector>


namespace cemu {

/**
 * Generic GPIO.
 * A GPIO exposes an 8 bits address range where data can be written to or read from.
 * Read and write operations are done through user defined callbacks.
 */
class Gpio {
public:
    using ior_t = std::function<uint8_t(uint8_t)>;
    using iow_t = std::function<void(uint8_t, uint8_t)>;

    using ior_mask_t = std::pair<ior_t, uint8_t>;
    using iow_mask_t = std::pair<iow_t, uint8_t>;


    explicit Gpio() {
    }

    virtual ~Gpio() {
    }

    /**
     * Add an input callback.
     * @param ior  Input callback;
     * @param mask Bits to read.
     */
    void add_ior(const ior_t &ior, uint8_t mask) {
        _iors.push_back({ior, mask});
    }

    /**
     * Add an ouput callback.
     * @param iow  Output callback;
     * @param mask Bits to write.
     */
    void add_iow(const iow_t &iow, uint8_t mask) {
        _iows.push_back({iow, mask});
    }

    /**
     * Read from input pins.
     * @param addr Address to read from.
     * @return The input value.
     */
    uint8_t ior(uint8_t addr) const;

    /**
     * Write to output pins.
     * @param addr  Address to write;
     * @param value Value to write.
     */
    void iow(uint8_t addr, uint8_t value);

private:
    std::vector<ior_mask_t> _iors{};
    std::vector<iow_mask_t> _iows{};
};

}
