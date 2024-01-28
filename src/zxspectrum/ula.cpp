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
#include "ula.hpp"

#include "zxsp_params.hpp"


namespace caio {
namespace sinclair {
namespace zxspectrum {

ULA::ULA(const sptr_t<Z80>& cpu, const sptr_t<RAM>& ram, const sptr_t<ROM>& rom,
    const sptr_t<ZXSpectrumKeyboard>& kbd, const sptr_t<Joystick>& joy, const sptr_t<ZXSpectrumTape>& tape)
    : _audio{std::make_shared<ULAAudio>(CLOCK_FREQ, "audio")},
      _video{std::make_shared<ULAVideo>(cpu, ram, "video")},
      _aspace{std::make_shared<ULAASpace>(cpu, ram, rom, _audio, _video, kbd, joy, tape)}
{
}

void ULA::reset()
{
    _aspace->reset();
}

std::string ULA::to_string() const
{
    return _audio->to_string() + "\n  " + _video->to_string();
}

}
}
}
