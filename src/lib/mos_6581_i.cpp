/*
 * Copyright (C) 2020-2022 Claudio Castiglia
 *
 * This file is part of CAIO.
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
#include "mos_6581_i.hpp"

#include <algorithm>

#include "utils.hpp"


namespace caio {

std::ostream &Mos6581I::dump(std::ostream &os, addr_t base) const
{
    std::array<uint8_t, SIZE> regs{};
    std::fill(regs.begin(), regs.end(), 255);

    regs[ADC_1] = read(ADC_1);
    regs[ADC_2] = read(ADC_2);
    regs[VOICE_3_OSC] = read(VOICE_3_OSC);
    regs[VOICE_3_ENV] = read(VOICE_3_ENV);

    return utils::dump(os, regs, base);
}

}
