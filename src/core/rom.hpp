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

namespace caio {

/**
 * ROM device.
 */
class ROM : public RAM {
public:
    constexpr static const char* TYPE = "ROM";

    /**
     * Initialise this ROM with data from a buffer.
     * @param first Input iterator first element;
     * @param last  Input iterator last element + 1;
     * @param label Label assigned to this ROM.
     */
    template<typename Iterator>
    ROM(Iterator first, Iterator last, std::string_view label)
        : RAM{first, last, label} {
        type(TYPE);
    }

    /**
     * Initialise this ROM with data from a file.
     * @param fname  Name of the file;
     * @param digest Signature (SHA-256 digest the file must have);
     * @param label  Label assigned to this ROM.
     * @exception IOError If the specified signature is not equal to the calculated one.
     * @see signature()
     * @see RAM::RAM(std::string_view size_t, std::string_view)
     */
    ROM(std::string_view fname, std::string_view digest, std::string_view label);

    /**
     * Initialise this ROM with data from a file.
     * @param fname  Name of the file;
     * @param size   Size the file must have (0 not to check size);
     * @param label  Label assigned to this ROM.
     * @exception IOError If the size of the file is not equal to the specified size.
     * @see RAM:RAM(std::string_view size_t, std::string_view)
     */
    ROM(std::string_view fname, size_t size, std::string_view label);

    /**
     * Initialise this ROM with data from an input stream.
     * @param is    Input stream to read from;
     * @param count Number of bytes to read (0 means fs::LOAD_MAXSIZ bytes);
     * @exception IOError If the input stream is emptied before the specified amount of bytes are read.
     * @see RAM::RAM(std::istream&, size_t)
     */
    ROM(std::istream& is, size_t count = 0);

    /**
     * Get an iterator to the first element of this ROM.
     * @return An iterator to the first element.
     */
    buffer_cit_t begin() const {
        return _data.cbegin();
    }

    /**
     * Get an iterator to the last element of this ROM + 1.
     * @return An iterator to the last element +1.
     */
    buffer_cit_t end() const {
        return _data.cend();
    }

    /**
     * This method does nothing.
     * @see Device::dev_write(addr_t, uint8_t)
     */
    void dev_write(addr_t addr, uint8_t data) override;

    /**
     * Calculate the signature of this ROM.
     * @return The SHA-256 digest of this ROM as a string.
     */
    std::string signature() const;
};

}
