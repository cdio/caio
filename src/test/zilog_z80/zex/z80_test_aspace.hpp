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

#include <iostream>
#include <memory>

#include "aspace.hpp"
#include "device.hpp"
#include "zilog_z80.hpp"


namespace caio {
namespace test {

/**
 * Z80 test device.
 */
class Z80TestASpace : public ASpace {
public:
    Z80TestASpace(const sptr_t<Z80>& cpu, const devptr_t& ram, const devptr_t& rom, std::ostream& out);

    virtual ~Z80TestASpace() {
    }

    uint8_t read(addr_t addr, ReadMode mode = ReadMode::Read) override;

    void write(addr_t addr, uint8_t value) override;

private:
    void bdos(uint8_t cmd, addr_t arg);

    std::shared_ptr<Z80> _cpu;
    devptr_t             _ram;
    devptr_t             _rom;
    std::ostream&        _out;
};

}
}
