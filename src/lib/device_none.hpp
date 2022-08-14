/*
 * Copyright (C) 2020-2022 Claudio Castiglia
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

/**
 * The none device does nothing.
 */
class DeviceNone : public Device {
public:
    constexpr static const char *TYPE = "NONE";

    /**
     * Initialise None Device.
     */
    DeviceNone();

    ~DeviceNone();

    /**
     * @see Device::reset()
     */
    void reset() override;

    /**
     * @return 0.
     */
    size_t size() const override;

    /**
     * This method does nothing.
     * @return 0.
     */
    uint8_t read(addr_t) const override;

    /**
     * This method does nothing.
     */
    void write(addr_t, uint8_t) override;

    /**
     * This method does nothing.
     * @param os Output stream.
     * @return The output stream.
     */
    std::ostream &dump(std::ostream &os, addr_t = 0) const override;
};


extern devptr_t device_none;

}
