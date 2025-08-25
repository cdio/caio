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

#include "ram.hpp"
#include "rom.hpp"
#include "serializer.hpp"

namespace caio {

class RAMBank : public Device {
public:
    constexpr static const char* TYPE = "RAM-bank";

    RAMBank()
        : Device{TYPE, ""}
    {
    }

    RAMBank(RAM& ram, size_t bsize, size_t bnk = 0)
        : Device{TYPE, ram.label()},
          _ram{&ram},
          _bsize{bsize},
          _banks{ram.size() / bsize}
    {
        bank(bnk);
    }

    RAMBank(ROM& rom, size_t bsize, size_t bnk = 0)
        : RAMBank{*static_cast<RAM*>(&rom), bsize, bnk}
    {
    }

    RAMBank& operator=(const RAMBank& other)
    {
        label(other.label());
        _ram = other._ram;
        _bsize = other._bsize;
        _banks = other._banks;
        _bank = other._bank;
        _boffset = other._boffset;
        return *this;
    }

    void reset() override
    {
    }

    size_t size() const override
    {
        return _bsize;
    }

    std::ostream& dump(std::ostream& os, size_t base = 0) const override;

    void bank(size_t bnk)
    {
        _bank = bnk % _banks;
        _boffset = _bank * _bsize;
    }

    size_t bank() const
    {
        return _bank;
    }

    size_t banks() const
    {
        return _banks;
    }

protected:
    uint8_t dev_read(size_t addr, RAM::ReadMode mode = RAM::ReadMode::Read) override
    {
        return _ram->dev_read(addr + _boffset, mode);
    }

    void dev_write(size_t addr, uint8_t data) override
    {
        _ram->dev_write(addr + _boffset, data);
    }

    RAM*    _ram{};
    size_t  _bsize{};
    size_t  _banks{};
    size_t  _bank{};
    size_t  _boffset{};

    friend Serializer& operator&(Serializer&, RAMBank&);
};

using ROMBank = RAMBank;

}
