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

#include "mos_6502.hpp"


namespace cemu {

/**
 * MOS 6510 emulator.
 */
class Mos6510 : public Mos6502 {
public:
    constexpr static const char *TYPE     = "MOS6510";

    constexpr static const addr_t PORT_0  = 0x0000;
    constexpr static const addr_t PORT_1  = 0x0001;

    /**
     * Initialise this CPU connected to a set of other devices and peripherals.
     * @param type  CPU type (usually the model name);
     * @param label CPY label (usually an arbitrary instance name).
     * @see ASpace
     */
    Mos6510(const std::string &type = TYPE, const std::string &label = LABEL);

    /**
     * Initialise this CPU connected to a set of other devices and peripherals.
     * @param mmap  System mappings;
     * @param type  CPU type (usually the model name);
     * @param label CPY label (usually an arbitrary instance name).
     * @see ASpace
     */
    Mos6510(const std::shared_ptr<ASpace> &mmap, const std::string &type = TYPE, const std::string &label = LABEL);

    virtual ~Mos6510();

    /**
     * @see Mos6502::bpadd()
     */
    void bpadd(addr_t addr, const std::function<void(Mos6510 &, void *)> &cb, void *arg);
};

}