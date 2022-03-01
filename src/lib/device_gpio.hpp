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

#include <utility>
#include <vector>

#include "device.hpp"
#include "gpio.hpp"
#include "utils.hpp"


namespace cemu {

/**
 * GPIO Device.
 * Device with BANKS*256 addresses and 8 bits data size.
 */
class DeviceGpio : public Device {
public:
    constexpr static const char *TYPE = "I/O";
    constexpr static addr_t BSIZE     = 256;

    using ior_t = Gpio::ior_t;
    using iow_t = Gpio::iow_t;

    /**
     * Initialise a RAM Device.
     * @param label Label assigned to this Device;
     * @param banks Banks.
     */
    DeviceGpio(const std::string &label, addr_t banks);

    virtual ~DeviceGpio();

    /**
     * @see Device::size()
     */
    size_t size() const override;

    /**
     * @see Device::read()
     */
    uint8_t read(addr_t addr) const override;

    /**
     * @see Device::write()
     */
    void write(addr_t addr, uint8_t value) override;

    /**
     * @see Device::dump()
     */
    std::ostream &dump(std::ostream &os, addr_t base = 0) const override;

    /**
     * Add an input callback.
     * @param ior  Input callback;
     * @param bank Bank associated to the callback;
     * @param mask Data bits used by the callback.
     */
    void add_ior(const ior_t &ior, addr_t bank, uint8_t mask);

    /**
     * Add an ouput callback.
     * @param iow  Output callback;
     * @param bank Bank associated to the callback;
     * @param mask Data bits used by the callback.
     */
    void add_iow(const iow_t &iow, addr_t bank, uint8_t mask);

private:
    std::pair<addr_t, uint8_t> decode(addr_t addr) const;

    unsigned          _size{};
    addr_t            _mask{};
    std::vector<Gpio> _ports{};
};

}
