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

#include "mos_6581_i.hpp"


namespace caio {
namespace mos_6581 {

/**
 * MOS6581 reSID.
 * reSID library wrapper.
 * @see Mos6581_
 */
class Mos6581Resid : public Mos6581_ {
public:
    Mos6581Resid(const std::string& label, unsigned clkf);

    virtual ~Mos6581Resid() {
    }

    /**
     * @see Device::read()
     */
    uint8_t read(addr_t addr, ReadMode mode = ReadMode::Read) override;

    /**
     * @see Device::write()
     */
    void write(addr_t addr, uint8_t value) override;

    /**
     * @return The reSID library version.
     */
    static const std::string version();

private:
    /**
     * Generate and play an audio signal.
     * This method must be called by the system clock.
     * @param clk Caller clock.
     * @return The number of clock cycles that must pass before this method could be called again.
     * @see Clockable::tick(const Clock &)
     */
    size_t tick(const Clock& clk) override;

    sptr_t<class Resid> _resid{};
};

}

using Mos6581Resid = mos_6581::Mos6581Resid;

}
