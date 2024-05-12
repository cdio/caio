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

#include <cstdint>
#include <iostream>
#include <span>
#include <string>
#include <vector>

#include "types.hpp"

namespace caio {
namespace commodore {
namespace c64 {

/**
 * PRG file.
 */
class PrgFile : public std::vector<uint8_t> {
public:
    struct PrgHeader {
        uint16_t addr;      /* Loading address (little endian on file, host order on memory) */
        /* data follows */
    } __attribute__((packed));

    /**
     * Load a PRG file.
     * @param fname If defined, name of the PRG file to load.
     * @exception IOError
     * @see load(const std::string &)
     */
    PrgFile(const std::string& fname = {}) {
        PrgFile::load(fname);
    }

    virtual ~PrgFile() {
    }

    /**
     * Get the start address of this PRG file.
     * @return The start address.
     */
    addr_t address() const {
        return _hdr.addr;
    }

    /**
     * Set a new start address for this PRG file.
     * @param addr New start address.
     */
    void address(addr_t addr) {
        _hdr.addr = addr;
    }

    /**
     * Load a PRG file.
     * Any previous content of this PRG is replaced with the new data.
     * @param fname Name of the PRG file to load.
     * @exception IOError
     */
    virtual void load(const std::string& fname);

    /**
     * Load a PRG file from an already open stream.
     * Any previous content of this PRG is replaced with the new data.
     * @param is Input stream.
     * @return is.
     * @exception IOError
     */
    virtual std::istream& load(std::istream& is);

    /**
     * Save a PRG file.
     * @param fname Name of the PRG file to save;
     * @param addr  If non-zero, use this value as the PRG's start address ignoring address().
     * @exception IOError
     */
    virtual void save(const std::string& fname, addr_t addr = 0);

    /**
     * Save a PRG file.
     * @param os   Output stream;
     * @param addr If non-zero, use this value as the PRG's start address ignoring address().
     * @return os.
     * @exception IOError
     */
    virtual std::ostream& save(std::ostream& os, addr_t addr = 0);

    /**
     * Save a PRG file.
     * @param fname Name of the PRG file to save.
     * @param addr  Start address of the PRG file;
     * @param data  Data.
     * @exception IOError
     */
    static void save(const std::string& fname, addr_t addr, const std::span<uint8_t>& data);

    /**
     * Save a PRG file.
     * @param os   Output stream;
     * @param addr Start address of the PRG file;
     * @param data Data.
     * @return os.
     * @exception IOError
     */
    static std::ostream& save(std::ostream& os, addr_t addr, const std::span<uint8_t>& data);

private:
    PrgHeader _hdr{};
};

}
}
}
