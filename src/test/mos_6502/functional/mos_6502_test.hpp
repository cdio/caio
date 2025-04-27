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

#include "clock.hpp"
#include "fs.hpp"
#include "ram.hpp"
#include "mos_6502.hpp"
#include "readline.hpp"

#include "mos_6502_test_aspace.hpp"

namespace caio {
namespace test {

constexpr static const unsigned CLOCK_FREQ = 985248;

/**
 * MOS-6502 tester.
 */
class Mos6502Test {
public:
    Mos6502Test(const fs::Path& fname);

    void run(bool autostart);

    sptr_t<Mos6502> cpu() {
        return _cpu;
    }

private:
    sptr_t<Clock>               _clk;
    sptr_t<RAM>                 _ram;
    sptr_t<Mos6502>             _cpu;
    Readline                    _io;
    sptr_t<Mos6502TestASpace>   _mmap;
};

}
}
