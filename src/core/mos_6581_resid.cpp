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
#include "mos_6581_resid.hpp"

#include "3rdparty/resid/sid.h"

#include "ui.hpp"
#include "utils.hpp"

namespace caio {
namespace mos_6581 {

class Resid : public ::SID {
public:
    using ::SID::SID;
};

Mos6581Resid::Mos6581Resid(std::string_view label, unsigned clkf)
    : Mos6581_{label, clkf},
      _resid{std::make_shared<Resid>()}
{
    _resid->set_chip_model(::chip_model::MOS6581);
    _resid->set_sampling_parameters(clkf, ::sampling_method::SAMPLE_FAST, SAMPLING_RATE);
    _resid->reset();
}

uint8_t Mos6581Resid::dev_read(addr_t addr, ReadMode)
{
    return _resid->read(addr);
}

void Mos6581Resid::dev_write(addr_t addr, uint8_t value)
{
    _resid->write(addr, value);
}

size_t Mos6581Resid::tick(const Clock& clk)
{
    if (_audio_buffer) {
        auto v = _audio_buffer();
        if (v) {
            auto dt = static_cast<::cycle_count>(_samples_cycles);
            _resid->clock(dt, &v[0], v.size(), 1);
            v.dispatch();
        }
    }

    return _samples_cycles;
}

std::string Mos6581Resid::version()
{
    return std::format("reSID-{}", resid_version_string);
}

}
}
