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

#include "types.hpp"
#include "zilog_z80.hpp"

#include <span>
#include <tuple>

namespace caio {
namespace sinclair {
namespace zxspectrum {

/**
 * Snapshot base class.
 * A snapshot is an image of a running system. It usually
 * contains the status of the cpu registers and the content
 * of the RAM as it-was when the snapshot was created.
 *
 * This class must be derived by a specific snapshot format.
 */
struct Snapshot {
public:
    using IntFlags = std::tuple<Z80::IMode, bool, bool>;

    Snapshot() = default;

    virtual ~Snapshot() = default;

    /**
     * Get the CPU registers.
     * @return The CPU registers.
     * @see Z80::Registers
     */
    Z80::Registers regs() const
    {
        return _regs;
    }

    /**
     * Get the CPU interrupt mode.
     * @return The CPU interrupt mode and internal flip flop values.
     * @see Z80::IMode
     */
    IntFlags interrupt_flags() const
    {
        return _intflags;
    }

    /**
     * Get the uncompressed RAM.
     * @return The uncompressed RAM data.
     */
    std::span<const uint8_t> ram() const
    {
        return _data;
    }

    /**
     * Retrieve the border colour.
     * @return The border colour.
     */
    uint8_t border_colour() const
    {
        return _border_colour;
    }

protected:
    void throw_ioerror(std::string_view caller, std::string_view reason = {}) const;

    fs::Path        _fname{};
    Buffer          _data{};
    Z80::Registers  _regs{};
    IntFlags        _intflags{};
    uint8_t         _border_colour{};
};

}
}
}
