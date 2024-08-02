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
#include "aspace.hpp"

#include "logger.hpp"
#include "utils.hpp"

namespace caio {

uint8_t ASpace::read(addr_t addr, ReadMode mode)
{
    const auto [bank, offset] = decode(addr);
    if (bank < _rmaps.size()) {
        auto& dev = _rmaps[bank];
        auto addr = dev.second + offset;
        uint8_t data = dev.first->read(addr, mode);
        if (mode != ReadMode::Peek) {
            _address_bus = addr;
            _data_bus = data;
        }
        return data;
    }

    log.fatal("ASpace: Invalid read address: {:04x}\n", addr);
    /* NOTREACHED */
}

void ASpace::write(addr_t addr, uint8_t value)
{
    const auto [bank, offset] = decode(addr);
    if (bank < _wmaps.size()) {
        auto& dev = _wmaps[bank];
        _address_bus = dev.second + offset;
        _data_bus = value;
        dev.first->write(_address_bus, value);
    } else {
        log.fatal("ASpace: Invalid write address: {:04x}\n", addr);
        /* NOTREACHED */
    }
}

void ASpace::reset(const addrmap_t& rmaps, const addrmap_t& wmaps, addr_t amask)
{
    auto banks = wmaps.size();
    CAIO_ASSERT(banks != 0 && banks == rmaps.size());

    auto bsize = (static_cast<size_t>(amask) + 1) / banks;
    auto bmask = bsize - 1;
    CAIO_ASSERT((bsize & bmask) == 0);

    _amask = amask;
    _rmaps = rmaps;
    _wmaps = wmaps;
    _bsize = bsize;
    _bmask = bmask;
    for (_bshift = 0; bmask != 0; bmask >>= 1, ++_bshift);
    _data_bus = 0;
    _address_bus = 0;
}

inline std::pair<addr_t, addr_t> ASpace::decode(addr_t addr) const
{
    addr &= _amask;
    addr_t bank = addr >> _bshift;
    addr_t offset = addr & _bmask;
    return {bank, offset};
}

std::ostream& ASpace::dump(std::ostream& os) const
{
    os.setf(std::ios::left, std::ios::adjustfield);

    unsigned amax = _amask + 1;

    for (unsigned addr = 0; addr < amax; addr += _bsize) {
        auto [bank, _] = decode(addr);
        const auto& rdev = _rmaps[bank];
        const auto& wdev = _wmaps[bank];
        const addr_t astart = static_cast<addr_t>(addr);
        const addr_t aend = addr + _bsize - 1;
        const addr_t rstart = rdev.second;
        const addr_t wstart = wdev.second;
        const addr_t rend = rstart + _bsize - 1;
        const addr_t wend = wstart + _bsize - 1;

        os << utils::to_string(astart) << "-" << utils::to_string(aend) << "    ";

        os << utils::to_string(rstart) << "-" << utils::to_string(rend) << " ";
        os.width(25);
        os << rdev.first->label() << "  ";

        os << utils::to_string(wstart) << "-" << utils::to_string(wend) << " ";
        os.width(25);
        os << wdev.first->label() << "\n";
    }

    return os;
}

}
