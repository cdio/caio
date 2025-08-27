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

#include <cstdint>
#include <functional>
#include <tuple>
#include <type_traits>
#include <vector>

namespace caio {

/**
 * GPIO template.
 * A GPIO is a container of I/O Ports, each port has a fixed number of bits or pins.
 * R/W access to the ports are handled by input/output callbacks.
 * Each callback is associated to a port/mask pair which specifies the pins of the
 * port affected by the callback.
 * Each port can be associated to several input and output callbacks so the port
 * pins are implemented as pull-ups.
 */
template <typename ADDR, typename DATA>
requires std::is_unsigned_v<ADDR> && std::is_unsigned_v<DATA>
class Gpio_ {
public:
    using addr_type = ADDR;
    using data_type = DATA;

    using IorCb = std::function<DATA(ADDR)>;
    using IowCb = std::function<void(ADDR, DATA, bool)>;

    using IorMask = std::tuple<IorCb, DATA>;
    using IowMask = std::tuple<IowCb, DATA>;

    Gpio_() = default;

    virtual ~Gpio_() = default;

    /**
     * Add an input callback.
     * @param ior  Input callback;
     * @param mask Data bits used by the callback.
     */
    void add_ior(const IorCb& ior, DATA mask)
    {
        _iors.push_back({ior, mask});
    }

    /**
     * Add an ouput callback.
     * @param iow  Output callback;
     * @param mask Data bits used by the callback.
     */
    void add_iow(const IowCb& iow, DATA mask)
    {
        _iows.push_back({iow, mask});
    }

    /**
     * Read from an input port.
     * The input callbacks associated to the port are called
     * and the combined result (bitwise AND) is returned.
     * @param addr Port to read from.
     * @return The port value.
     */
    DATA ior(ADDR addr) const
    {
        DATA value{static_cast<DATA>(-1)};   /* pull-up */
        for (const auto& [ior, mask] : _iors) {
            value &= (ior(addr) & mask) | ~mask;
        }
        return value;
    }

    /**
     * Write to an output port.
     * The output callbacks associated to the port are called,
     * each callback will be receiving as argument the bitwise
     * AND operation between the specified value and its mask.
     * @param addr  Address to write;
     * @param value Value to write;
     * @param force Force write.
     */
    void iow(ADDR addr, DATA value, bool force = false)
    {
        for (auto& [iow, mask] : _iows) {
            iow(addr, value & mask, force);
        }
    }

private:
    std::vector<IorMask> _iors{};
    std::vector<IowMask> _iows{};
};

/**
 * Generic I/O ports.
 * 256 I/O ports with 8 pins each.
 */
using Gpio = Gpio_<uint8_t, uint8_t>;

}
