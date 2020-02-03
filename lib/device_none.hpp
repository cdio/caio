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

#include "device.hpp"


namespace cemu {

/**
 * The none Device does nothing.
 */
class DeviceNone : public Device {
public:
    constexpr static const char *TYPE = "NONE";

    /**
     * Initialise None Device.
     * @param label Label assigned to this Device;
     */
    DeviceNone(const std::string &label = {})
        : Device{TYPE, label} {
    }

    /**
     * @return 0.
     */
    size_t size() const override {
        return 0;
    }

    /**
     * This method does nothing.
     * @return 0.
     */
    uint8_t read(addr_t) const override {
        return 0;
    }

    /**
     * This method does nothing.
     */
    void write(addr_t, uint8_t) override {
    }

    /**
     * This method does nothing.
     * @param os Output stream.
     * @return The output stream.
     */
    std::ostream &dump(std::ostream &os, addr_t = 0) const override {
        return os;
    }
};


extern devptr_t none;

}
