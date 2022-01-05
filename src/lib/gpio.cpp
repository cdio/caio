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
#include "gpio.hpp"


namespace cemu {

Gpio::Gpio()
{
}

Gpio::~Gpio()
{
}

void Gpio::add_ior(const ior_t &ior, uint8_t mask)
{
    _iors.push_back({ior, mask});
}

void Gpio::add_iow(const iow_t &iow, uint8_t mask)
{
    _iows.push_back({iow, mask});
}

uint8_t Gpio::ior(uint8_t addr) const
{
    uint8_t value{255};

    for (const auto &[ior, mask] : _iors) {
        value &= (ior(addr) & mask) | ~mask;
    }

    return value;
}

void Gpio::iow(uint8_t addr, uint8_t value)
{
    for (auto &[iow, mask] : _iows) {
        iow(addr, value & mask);
    }
}

}
