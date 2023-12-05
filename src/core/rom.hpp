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
    ROM(Iterator first, Iterator last, const std::string& label = {})
        : RAM{first, last, label} {
        type(TYPE);
    }

    /**
     * Initialise this ROM with data from a file.
     * @param fname Name of the file;
     * @param count Number of bytes to read (0 means fs::LOAD_MAXSIZ bytes);
     * @param label Label assigned to this RAM.
     * @exception IOError If the size of the file is not equal to the specified count.
     * @see fs::LOAD_MAXSIZ
     * @see fs::load()
     */
    ROM(const std::string& fname, size_t count = 0, const std::string& label = {});

    /**
     * Initialise this ROM with data from an input stream.
     * @param is    Input stream to read from;
     * @param count Number of bytes to read (0 means fs::LOAD_MAXSIZ bytes);
     * @exception IOError If the input stream is emptied before the specified amount of bytes are read.
     */
    ROM(std::istream& is, size_t count = 0);

    /**
     * @return An iterator to the first element of this RAM.
     */
    std::vector<uint8_t>::const_iterator begin() const {
        return _data.cbegin();
    }

    /**
     * @return An iterator to the last element of this RAM +1.
     */
    std::vector<uint8_t>::const_iterator end() const {
        return _data.cend();
    }

    /**
     * This method does nothing.
     */
    void write(addr_t addr, uint8_t data) override;
};

}
