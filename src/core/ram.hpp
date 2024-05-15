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

#include <functional>
#include <iostream>
#include <span>
#include <vector>

#include "device.hpp"
#include "utils.hpp"

namespace caio {

/**
 * RAM device.
 */
class RAM : public Device {
public:
    constexpr static const char* TYPE = "RAM";
    constexpr static const bool PUT_RANDOM_VALUES = true;
    constexpr static const bool NO_RANDOM_VALUES = false;

    /**
     * Initialise this RAM with zeros.
     * @param size  Size of this RAM;
     * @param label Label assigned to this RAM.
     */
    RAM(size_t size, std::string_view label);

    /**
     * Initialise this RAM with a repeating pattern.
     * @param size    Size of this RAM;
     * @param pattern Pattern;
     * @param random  If true contaminate this RAM with some random values;
     * @param label   Label assigned to this RAM.
     * @see NO_RANDOM_VALUES
     * @see PUT_RANDOM_VALUES
     * @see fill()
     */
    template<typename T>
    RAM(size_t size, T pattern, bool random, std::string_view label)
        : Device{TYPE, label},
          _data(size) {
        std::span<uint8_t> dst{_data.data(), _data.size()};
        fill(dst, pattern, random);
    }

    /**
     * Initialise this RAM with data from a buffer.
     * @param first Input iterator first element;
     * @param last  Input iterator last element + 1;
     * @param label Label assigned to this RAM.
     */
    template<typename Iterator>
    RAM(Iterator first, Iterator last, std::string_view label)
        : Device{TYPE, label},
          _data{first, last} {
    }

    /**
     * Initialise this RAM with data from a file.
     * @param fname Name of the file;
     * @param count Number of bytes to read (0 means fs::LOAD_MAXSIZ bytes);
     * @param label Label assigned to this RAM.
     * @exception IOError If count exceeds the size of the file.
     * @see fs::LOAD_MAXSIZ
     * @see fs::load()
     */
    RAM(std::string_view fname, size_t count, std::string_view label);

    /**
     * Initialise this RAM with data from an input stream.
     * @param is    Input stream to read from;
     * @param count Number of bytes to read (0 means fs::LOAD_MAXSIZ bytes);
     * @exception IOError If the input stream is empited before the specified amount of bytes are read.
     */
    RAM(std::istream& is, size_t count = 0);

    /**
     * Initialise this RAM moving another RAM.
     * @param other The RAM to move;
     */
    RAM(RAM&& other);

    virtual ~RAM();

    /**
     * Get an iterator to the first element of this RAM.
     * @return An iterator to the first element.
     */
    buffer_it_t begin() {
        return _data.begin();
    }

    /**
     * Get an iterator to the last element of this RAM +1.
     * @return An iterator to the last element +1.
     */
    buffer_it_t end() {
        return _data.end();
    }

    /**
     * @see Device::reset()
     */
    void reset() override {
    }

    /**
     * @see Device::size()
     */
    size_t size() const override {
        return _data.size();
    }

    /**
     * @see Device::read()
     */
    uint8_t read(addr_t addr, ReadMode mode = ReadMode::Read) override;

    /**
     * @see Device::write()
     */
    void write(addr_t addr, uint8_t data) override;

    /**
     * @see Device::dump()
     */
    std::ostream& dump(std::ostream& os, addr_t base = 0) const override;

protected:
    buffer_t _data{};
};

}
