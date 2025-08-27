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

#include "fs.hpp"
#include "types.hpp"

namespace caio {
namespace sinclair {
namespace zx80 {

/**
 * .o cassette file image.
 */
class OFile : public std::vector<uint8_t> {
public:
    constexpr static const addr_t SIZE_OFFSET = 10;
    constexpr static const addr_t LOAD_ADDR   = 0x4000;
    constexpr static const addr_t MAX_SIZE    = 16384;

    OFile() = default;

    /**
     * Create a cassette file from a data buffer.
     * @param buf Data buffer.
     */
    OFile(std::vector<uint8_t>&& buf)
        : std::vector<uint8_t>{std::move(buf)}
    {
    }

    virtual ~OFile() = default;

    /**
     * Get the load address of this .o file.
     * @return The load address.
     */
    virtual addr_t load_address() const
    {
        return LOAD_ADDR;
    }

    /**
     * Get the position containing the size of this file (little-endian).
     * @return The offset to the size of this .o file.
     */
    virtual addr_t size_offset() const
    {
        return SIZE_OFFSET;
    }

    /**
     * Load a Cassette Image file.
     * The content of this instance is replaced with the new data.
     * @param fname Name of the file to load.
     * @exception IOError
     */
    void load(std::string_view fname);

    /**
     * Save this cassette image to file.
     * @param fname Name of the file to save.
     * @exception IOError
     */
    void save(std::string_view fname);
};

/**
 * .p cassette file image.
 */
class PFile : public OFile {
public:
    constexpr static const addr_t SIZE_OFFSET = 11;
    constexpr static const addr_t LOAD_ADDR   = 0x4009;

    PFile() = default;

    /**
     * Create a cassette file from a data buffer.
     * @param buf Data buffer.
     */
    PFile(std::vector<uint8_t>&& buf)
        : OFile{std::move(buf)}
    {
    }

    virtual ~PFile() = default;

    /**
     * Get the load address of this .p file.
     * @return The load address.
     */
    addr_t load_address() const override
    {
        return PFile::LOAD_ADDR;
    }

    /**
     * Get the position containing the size of this file (little-endian).
     * @return The offset to the size of this .p file.
     */
    addr_t size_offset() const override
    {
        return PFile::SIZE_OFFSET;
    }
};

}
}
}
