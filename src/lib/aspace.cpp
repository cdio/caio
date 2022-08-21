/*
 * Copyright (C) 2020-2022 Claudio Castiglia
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

#include "utils.hpp"


namespace caio {

ASpace::ASpace()
{
}

ASpace::ASpace(const addrmap_t &rmaps, const addrmap_t &wmaps, addr_t amask)
{
    reset(rmaps, wmaps, amask);
}

uint8_t ASpace::read(addr_t addr) const
{
    try {
        const auto [bank, offset] = decode(addr);
        auto &dev = _rmaps.at(bank);
        return dev.first->read(dev.second + offset);
    } catch (std::out_of_range &) {
        throw InvalidReadAddress{"ASpace", addr};
    }
}

void ASpace::write(addr_t addr, uint8_t value)
{
    try {
        const auto [bank, offset] = decode(addr);
        auto &dev = _wmaps.at(bank);
        dev.first->write(dev.second + offset, value);
    } catch (std::out_of_range &) {
        throw InvalidWriteAddress{"ASpace", addr};
    }
}

void ASpace::reset()
{
}

void ASpace::reset(const addrmap_t &rmaps, const addrmap_t &wmaps, addr_t amask)
{
    auto banks = wmaps.size();
    if (!banks) {
        throw InvalidArgument{"ASpace::reset(): 0 banks specified"};
    }

    auto bsize = (static_cast<size_t>(amask) + 1) / banks;
    auto bmask = bsize - 1;
    if ((bsize & bmask) != 0) {
        throw InvalidArgument{"ASpace::reset(): Bank size is not a power of 2: " + std::to_string(bsize)};
    }

    _amask = amask;
    _rmaps = rmaps;
    _wmaps = wmaps;
    _bsize = bsize;
    _bmask = bmask;
    for (_bshift = 0; bmask != 0; bmask >>= 1, ++_bshift);
}

inline std::pair<addr_t, addr_t> ASpace::decode(addr_t addr) const
{
    addr &= _amask;
    addr_t bank = addr >> _bshift;
    addr_t offset = addr & _bmask;
    return {bank, offset};
}

std::ostream &ASpace::dump(std::ostream &os) const
{
    os.setf(std::ios::left, std::ios::adjustfield);

    unsigned amax = _amask + 1;

    for (unsigned addr = 0; addr < amax; addr += _bsize) {
        auto [bank, _] = decode(addr);
        const auto &rdev = _rmaps[bank];
        const auto &wdev = _wmaps[bank];
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
