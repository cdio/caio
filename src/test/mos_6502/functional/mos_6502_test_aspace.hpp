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

#include <memory>

#include "aspace.hpp"
#include "device.hpp"
#include "mos_6502.hpp"
#include "readline.hpp"

namespace caio {
namespace test {

/**
 * MOS-6502 test device address space.
 */
class Mos6502TestASpace : public ASpace {
public:
    constexpr static const size_t BLOCKS = 1;
    using bank_t = std::array<devmap_t, BLOCKS>;

    Mos6502TestASpace(const sptr_t<Mos6502>& cpu, const devptr_t& ram, Readline& io);

    virtual ~Mos6502TestASpace() {
    }

    uint8_t read(addr_t addr, ReadMode mode = ReadMode::Read) override;

    void write(addr_t addr, uint8_t value) override;

private:
    sptr_t<Mos6502> _cpu;
    devptr_t        _ram;
    Readline&       _io;
    bank_t          _mmap;
};

}
}
