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
#include "c64_bus_controller.hpp"


namespace caio {
namespace commodore {
namespace c64 {

C64BusController::C64BusController(const sptr_t<cbm_bus::Bus>& bus, const sptr_t<Mos6526>& cia2)
    : cbm_bus::Controller{bus},
      _cia2{cia2}
{
    _cia2->add_ior([this](uint8_t addr) -> uint8_t {
        return bus_read(addr);
    }, CBMBUS_READ_MASK | CBMBUS_WRITE_MASK);

    _cia2->add_iow([this](uint8_t addr, uint8_t value, bool) {
        bus_write(addr, value);
    }, CBMBUS_WRITE_MASK);
}

uint8_t C64BusController::bus_read(uint8_t addr)
{
    switch (addr) {
    case Mos6526::PRA: {
        /*
         * The read value is the value present in the bus
         * including the output pins (as they were written).
         */
        uint8_t value = _lastw | CBMBUS_READ_MASK;

        /*
         * Input CLK and DAT are received negated.
         */
        if (!clk()) {
            value &= ~CBMBUS_CLK_IN;
        }

        if (!dat()) {
            value &= ~CBMBUS_DAT_IN;
        }

        return value;
    }

    case Mos6526::PRB:
        //TODO User port
        break;

    default:;
    }

    return 255;
}

void C64BusController::bus_write(uint8_t addr, uint8_t value)
{
    switch (addr) {
    case Mos6526::PRA:
        /*
         * Written ATN, CLK and DAT values are not negated,
         * the values on these pins are inverted by hardware.
         */
        _lastw = value & CBMBUS_WRITE_MASK;
        atn(!(_lastw & CBMBUS_ATN_OUT));
        clk(!(_lastw & CBMBUS_CLK_OUT));
        dat(!(_lastw & CBMBUS_DAT_OUT));
        break;

    case Mos6526::PRB:
        //TODO User port
        break;

    default:;
    }
}

}
}
}
