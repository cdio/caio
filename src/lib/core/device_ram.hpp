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
#include <vector>

#include "device.hpp"
#include "device_rom.hpp"


namespace caio {

/**
 * RAM Device.
 */
class DeviceRAM : public Device {
public:
    constexpr static const char* TYPE = "RAM";

    using init_cb_t = std::function<void(gsl::span<uint8_t>&)>;

    /**
     * Initialise a RAM Device.
     * @param label  Label assigned to this Device;
     * @param size   Size of this RAM;
     * @param initcb Initialisation callback (if this parameter is not set
     * the memory is initialised with zeros).
     */
    DeviceRAM(const std::string& label, size_t size, const init_cb_t& initcb = {});

    /**
     * Initialise a RAM Device.
     * @param size   Size of this RAM;
     * @param initcb Initialisation callback (if this parameter is not set
     * the memory is initialised with zeros).
     */
    DeviceRAM(size_t size, const init_cb_t& initcb = {})
        : DeviceRAM{{}, size, initcb} {
    }

    /**
     * Initialise a RAM Device with predefined values.
     * @param label Label assigned to this Device;
     * @param data  Buffer with data values.
     */
    DeviceRAM(const std::string& label, const std::vector<uint8_t>& data)
        : Device{TYPE, label},
          _data(data) {
    }

    /**
     * Initialise a RAM Device with predefined values.
     * @param data Buffer with predefined values.
     */
    DeviceRAM(const std::vector<uint8_t>& data)
        : DeviceRAM{{}, data} {
    }

    /**
     * Convert a ROM into a RAM device
     * @param rom ROM device to convert.
     */
    DeviceRAM(class DeviceROM&& rom)
        : Device{TYPE, rom.label()},
          _data{std::move(rom._data)} {
    }

    /**
     * Convert a ROM into a RAM device
     * @param rom ROM device to convert.
     */
    DeviceRAM(const class DeviceROM& rom)
        : Device{TYPE, rom.label()},
          _data{rom._data} {
    }

    virtual ~DeviceRAM() {
    }

    /**
     * Merge a ROM into a RAM device
     * Copy the content of a ROM device inside this RAM.
     * @param rom    ROM device to merge;
     * @param offset Destination starting offset.
     * @note It is expected the ROM size plus the specified offset is equal or larger than the RAM size.
     */
    void copy(const class DeviceROM& rom, size_t offset = 0);

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
    std::ostream& dump(std::ostream& os, addr_t base = 0) const override {
        return utils::dump(os, _data, base);
    }

private:
    std::vector<uint8_t> _data{};
    init_cb_t            _initcb{};
};

}
