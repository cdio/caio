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

#include <memory>

#include "cbm_bus.hpp"
#include "mos_6526.hpp"

namespace caio {
namespace commodore {
namespace c64 {

class C64BusController : public cbm_bus::Controller {
public:
    constexpr static const uint8_t CBMBUS_ATN_OUT    = Mos6526::P3;
    constexpr static const uint8_t CBMBUS_CLK_OUT    = Mos6526::P4;
    constexpr static const uint8_t CBMBUS_DAT_OUT    = Mos6526::P5;
    constexpr static const uint8_t CBMBUS_CLK_IN     = Mos6526::P6;
    constexpr static const uint8_t CBMBUS_DAT_IN     = Mos6526::P7;
    constexpr static const uint8_t CBMBUS_READ_MASK  = CBMBUS_CLK_IN | CBMBUS_DAT_IN;
    constexpr static const uint8_t CBMBUS_WRITE_MASK = CBMBUS_ATN_OUT | CBMBUS_CLK_OUT | CBMBUS_DAT_OUT;

    C64BusController(const sptr_t<cbm_bus::Bus>& bus, const sptr_t<Mos6526>& cia2);

    virtual ~C64BusController() {
    }

private:
    /**
     * Called when the CIA2-Port A CBMBUS_WRITE_MASK or CBMBUS_READ_MASK pins are accessed.
     */
    uint8_t bus_read(uint8_t addr);

    /**
     * Called when the CIA2-Port A CBMBUS_WRITE_MASK pins are accessed.
     */
    void bus_write(uint8_t addr, uint8_t value);

    sptr_t<Mos6526> _cia2{};
    uint8_t         _lastw{};   /* Last written value */
};

}
}
}
