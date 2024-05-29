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

namespace caio {

extern devptr_t device_none;

/**
 * The device none does nothing.
 * A read operation returns a user defined default value.
 */
class DeviceNone : public Device {
public:
    constexpr static const char* TYPE = "NONE";

    /**
     * Initialise this device.
     * @param dvalue Value returned by a read operation (default is 0).
     * @see read(addr_t, ReadMode)
     */
    DeviceNone(uint8_t dvalue = 0)
        : Device{TYPE, TYPE},
          _dvalue{dvalue} {
    }

    virtual ~DeviceNone() {
    }

    /**
     * @see Device::reset()
     */
    void reset() override {
    }

    /**
     * Return 0.
     * @return 0.
     */
    size_t size() const override {
        return 0;
    }

    /**
     * This method does nothing.
     * @return The default value.
     * @see Device::dev_read(addr_t, ReadMode)
     */
    uint8_t dev_read(addr_t, ReadMode) override {
        return _dvalue;
    }

    /**
     * This method does nothing.
     * @see Device::dev_write(addr_t, uint8_t)
     */
    void dev_write(addr_t, uint8_t) override {
    }

    /**
     * This method does nothing.
     * @param os Output stream.
     * @return The output stream.
     */
    std::ostream& dump(std::ostream& os, addr_t = 0) const override {
        return os;
    }

private:
    uint8_t _dvalue;
};

}
