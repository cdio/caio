/*
 * Copyright (C) 2020 Claudio Castiglia
 *
 * This file is part of CEMU.
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


namespace cemu {

/**
 * RAM Device.
 */
class DeviceRAM : public Device {
public:
    constexpr static const char *TYPE = "RAM";

    /**
     * Initialise a RAM Device.
     * @param label Label assigned to this Device;
     * @param size  Size of this RAM.
     */
    DeviceRAM(const std::string &label, size_t size)
        : Device{TYPE, label},
          _data(size) {
    }

    /**
     * Initialise a RAM Device.
     * @param size Size of this RAM.
     */
    DeviceRAM(size_t size)
        : DeviceRAM{{}, size} {
    }

    /**
     * Initialise a RAM Device with predefined values.
     * @param label Label assigned to this Device;
     * @param data  Buffer with data values.
     */
    DeviceRAM(const std::string &label, const std::vector<uint8_t> &data)
        : Device{TYPE, label},
          _data(data) {
    }

    /**
     * Initialise a RAM Device with predefined values.
     * @param data Buffer with predefined values.
     */
    DeviceRAM(const std::vector<uint8_t> &data)
        : DeviceRAM{{}, data} {
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
    uint8_t read(addr_t addr) const override {
        if (addr < _data.size()) {
            return _data[addr];
        }

        throw InvalidReadAddress{*this, addr};
    }

    /**
     * @see Device::write()
     */
    void write(addr_t addr, uint8_t data) override {
        if (addr < _data.size()) {
            _data[addr] = data;
        } else {
            throw InvalidWriteAddress{*this, addr};
        }
    }

    /**
     * @see Device::dump()
     */
    std::ostream &dump(std::ostream &os, addr_t base = 0) const override {
        return utils::dump(os, _data, base);
    }

private:
    std::vector<uint8_t> _data{};
};

}
