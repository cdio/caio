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

#include <vector>

#include "device.hpp"
#include "types.hpp"
#include "utils.hpp"


namespace caio {

/**
 * ROM Device.
 */
class DeviceROM : public Device {
public:
    constexpr static const char* TYPE = "ROM";

    /**
     * Initialise this ROM Device.
     * @param label Label assigned to this Device;
     * @param data  Buffer with ROM data.
     */
    DeviceROM(const std::string& label, const std::vector<uint8_t>& data)
        : Device{TYPE, label},
          _data(data) {
    }

    /**
     * Initialise this ROM Device.
     * @param data Buffer with ROM data.
     */
    DeviceROM(const std::vector<uint8_t>& data)
        : DeviceROM{{}, data} {
    }

    /**
     * Initialise this ROM Device with data from a file.
     * @param fname Name of the file containing the ROM data;
     * @param label Label assigned to this Device;
     * @param size  If non-zero, the size that the ROM file must have.
     * @exception IOError if the specified file cannot be opened or its size is different from the expected one.
     */
    DeviceROM(const std::string& fname, const std::string& label, size_t size = 0);

    /**
     * Initialise this ROM Device with data from a file.
     * @param fname Name of the file containing the ROM data;
     * @param size  If non-zero, the size that the ROM file must have.
     * @exception IOError if the specified file cannot be opened or its size is different from the expected one.
     */
    DeviceROM(const std::string& fname, size_t size = 0)
        : DeviceROM{fname, {}, size} {
    }

    /**
     * Initialise this ROM Device with data from an input stream.
     * @param is   Input stream to read from;
     * @param size Size of the ROM to read (bytes).
     * @exception IOError if the input stream is empited before the specified amount of data bytes are read.
     */
    DeviceROM(std::istream& is, size_t size);

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
     * This method does nothing.
     */
    void write(addr_t addr, uint8_t data) override;

    /**
     * @see Device::dump()
     */
    std::ostream& dump(std::ostream& os, addr_t base = 0) const override {
        return utils::dump(os, _data, base);
    }

private:
    std::vector<uint8_t> _data{};

    friend class DeviceRAM;
};

}
