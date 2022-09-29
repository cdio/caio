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
#include "mos_6581_i.hpp"

#include <algorithm>

#include "utils.hpp"


namespace caio {

Mos6581I::Mos6581I(const std::string &label, unsigned clkf)
    : Device{TYPE, label},
      _clkf{clkf},
      _samples_cycles{Clock::cycles(SAMPLES_TIME, clkf)}
{
}

Mos6581I::~Mos6581I()
{
}

void Mos6581I::reset()
{
}

void Mos6581I::audio_buffer(const std::function<ui::AudioBuffer()> &abuf)
{
    _audio_buffer = abuf;
}

size_t Mos6581I::size() const
{
    return SIZE;
}

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
