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

#include "mos_6581_i.hpp"


namespace cemu {

/**
 * MOS6581 reSID.
 * reSID library wrapper.
 * @see Mos6581I
 */
class Mos6581Resid : public Mos6581I {
public:
    Mos6581Resid(const std::string &label, unsigned clkf);

    virtual ~Mos6581Resid() {
    }

    /**
     * @see Mos6581I::read()
     */
    uint8_t read(addr_t addr) const override;

    /**
     * @see Mos6581I::write()
     */
    void write(addr_t addr, uint8_t value) override;

private:
    /**
     * Generate and play an audio signal.
     * This method must be called by the system clock.
     * @param clk Caller clock.
     * @return The number of clock cycles that must pass before this method could be called again.
     * @see Clockable::tick(const Clock &)
     */
    size_t tick(const Clock &clk) override;

    std::shared_ptr<class SID> _resid{};
};

}
