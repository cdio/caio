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

#include "device.hpp"
#include "fs.hpp"
#include "serializer.hpp"
#include "types.hpp"
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

    RAM();

    /**
     * Initialise this RAM and fill it with zeros.
     * @param label Label;
     * @param size  Size.
     */
    RAM(std::string_view label, size_t size);

    /**
     * Initialise this RAM with a repeating pattern.
     * @param label   Label;
     * @param size    Size;
     * @param pattern Pattern;
     * @param random  If true contaminate this RAM with some random values.
     * @see NO_RANDOM_VALUES
     * @see PUT_RANDOM_VALUES
     */
    template <typename T>
    requires std::is_integral_v<T>
    RAM(std::string_view label, size_t size, T pattern, bool random)
        : Device{TYPE, label},
          _data(size)
    {
        utils::fill({_data.data(), _data.size()}, pattern, random);
    }

    /**
     * Initialise this RAM with data from a buffer.
     * @param label Label;
     * @param first Input iterator first element;
     * @param last  Input iterator last element + 1.
     */
    template <typename IT>
    requires std::input_iterator<IT>
    RAM(std::string_view label, IT first, IT last)
        : Device{TYPE, label},
          _data{first, last}
    {
    }

    /**
     * Initialise this RAM with data from a file.
     * @param label Label;
     * @param fname File name;
     * @param count Bytes to read (0 means fs::LOAD_MAXSIZ bytes).
     * @exception IOError If count exceeds the size of the file.
     * @see fs::LOAD_MAXSIZ
     * @see fs::load()
     */
    RAM(std::string_view label, const fs::Path& fname, size_t count);

    /**
     * Initialise this RAM with data from an input stream.
     * @param label Label;
     * @param is    Input stream to read from;
     * @param count Bytes to read (0 means fs::LOAD_MAXSIZ bytes).
     * @exception IOError If the input stream is empited before the specified amount of bytes is read.
     */
    RAM(std::string_view label, std::istream& is, size_t count = 0);

    /**
     * Initialise this RAM moving another RAM.
     * @param other The RAM to move;
     */
    RAM(RAM&& other);

    /**
     * Move operator.
     * @param other The RAM to move.
     * @return This RAM.
     */
    RAM& operator=(RAM&& other);

    virtual ~RAM() = default;

    /**
     * Get an iterator to the first element of this RAM.
     * @return An iterator to the first element.
     */
    Buffer_it begin()
    {
        return _data.begin();
    }

    /**
     * Get an iterator to the last element of this RAM +1.
     * @return An iterator to the last element +1.
     */
    Buffer_it end()
    {
        return _data.end();
    }

    /**
     * @see Device::reset()
     */
    void reset() override
    {
    }

    /**
     * @see Device::size()
     */
    size_t size() const override
    {
        return _data.size();
    }

    /**
     * @see Device::dev_read(size_t, ReadMode)
     */
    uint8_t dev_read(size_t addr, ReadMode mode = ReadMode::Read) override;

    /**
     * @see Device::dev_write(size_t, uint8_t)
     */
    void dev_write(size_t addr, uint8_t data) override;

    /**
     * @see Device::dump(std::ostream&, size_t) const
     */
    std::ostream& dump(std::ostream& os, size_t base = 0) const override;

    /**
     * Direct access to the RAM data.
     * @return A reference to the internal data buffer.
     */
    const Buffer& buffer() const
    {
        return _data;
    }

protected:
    Buffer _data{};

    friend Serializer& operator&(Serializer&, RAM&);
};

}
