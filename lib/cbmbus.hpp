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

#include "gpio.hpp"
#include "name.hpp"


namespace cemu {

/**
 * CBM-BUS or IEC-BUS.
 * This class implements the Commodore IEC serial bus
 * which is a stripped down version of IEEE-488/IEC-425.
 *
 */
class CBMBus : public Name, public Gpio {
public:
    constexpr static const char *TYPE = "CBM-BUS";

    enum Pinout {
        SERIAL_SRQ,         /* Serial Service Request (IN) (not implemented) */
        SERIAL_ATN,         /* Attention (IN/OUT)   */
        SERIAL_CLK,         /* Clock (IN/OUT)       */
        SERIAL_DAT,         /* Serial data (IN/OUT) */
        SERIAL_RST          /* Not used             */
    };

    /**
     * Initalise this SID instance.
     * @param label Label for this instance.
     */
    CBMBus(const std::string &label = {})
        : Name{TYPE, label} {
    }

    /**
     * Destroy this instance.
     */
    virtual ~CBMBus() {
    }

    /**
     *
     */


    1. Implement some sort of IPC mechanism using named pipes,
       the name of the pipe is set by the user in the constructor on a reset method.
       The IPC must emulate the status of all the pins (aka it must implement the protocol).

    2. Then create another class that instantiates a thread with a device number assigned to it (during creation)
       and an optional name.
       This way a 1541, tape, etc can be emulated inside the process or as a different external process.
};

}
