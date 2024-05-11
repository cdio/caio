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

#include <span>
#include <string>
#include <tuple>
#include <vector>

#include "types.hpp"
#include "zilog_z80.hpp"


namespace caio {
namespace sinclair {
namespace zxspectrum {

/**
 * Snapshot base class
 * A snapshot is an image of a running system. It usually
 * contains the status of the cpu registers and the content
 * of the RAM as it-was when the snapshot was created.
 *
 * This class must be derived by a specific snapshot format.
 */
struct Snapshot {
public:
    using intflags_t = std::tuple<Z80::IMode, bool, bool>;
    using buffer_t = std::vector<uint8_t>;

    Snapshot() {
    }

    virtual ~Snapshot() {
    }

    /**
     * Get the CPU registers.
     * @return The CPU registers.
     * @see Z80::Registers
     */
    Z80::Registers regs() const {
        return _regs;
    }

    /**
     * Get the CPU interrupt mode.
     * @return The CPU interrupt mode and internal flip flop values.
     * @see Z80::IMode
     */
    intflags_t interrupt_flags() const {
        return _intflags;
    }

    /**
     * Get the uncompressed RAM.
     * @return The uncompressed RAM data.
     */
    std::span<const uint8_t> ram() const {
        return _data;
    }

    /**
     * Retrieve the border colour.
     * @return The border colour.
     */
    uint8_t border_colour() const {
        return _border_colour;
    }

protected:
    void throw_ioerror(const std::string& caller, const std::string& reason = {}) const;

    std::string     _fname{};
    buffer_t        _data{};
    Z80::Registers  _regs{};
    intflags_t      _intflags{};
    uint8_t         _border_colour{};
};

}
}
}
