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
#include <string>

#include "clock.hpp"
#include "zilog_z80.hpp"
#include "device_ram.hpp"
#include "device_rom.hpp"

#include "z80_test_aspace.hpp"


namespace caio {
namespace test {

constexpr static const unsigned CLOCK_FREQ = 3250000;

/**
 * Z80 tester.
 */
class Z80Test {
public:
    Z80Test(const std::string& fname);

    void run(bool autostart);

private:
    std::shared_ptr<Clock>         _clk{};
    std::shared_ptr<DeviceRAM>     _ram{};
    std::shared_ptr<DeviceROM>     _rom{};
    std::shared_ptr<Z80>           _cpu{};
    std::shared_ptr<Z80TestASpace> _mmap{};
};

}
}
