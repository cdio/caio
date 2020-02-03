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

#include "device_ram.hpp"


namespace cemu {

/**
 * Nibble RAM.
 * The lower nibble of a byte can be set; the higher nibble of a byte is fixed to 1111b.
 */
class NibbleRAM : public DeviceRAM {
public:
    constexpr static const char *TYPE = "4-BIT RAM";

    NibbleRAM(const std::string &label, size_t size)
        : DeviceRAM{label, size} {
        type(TYPE);
    }

    virtual ~NibbleRAM() {
    }

    /**
     * @see DeviceRAM::write()
     */
    void write(addr_t addr, uint8_t value) override {
        DeviceRAM::write(addr, 0xf0 | value);
    }
};

}
