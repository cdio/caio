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
#include "ula_audio.hpp"

namespace caio {
namespace sinclair {
namespace zxspectrum {

ULAAudio::ULAAudio(unsigned clkf, const std::string& label)
    : Name{TYPE, label},
      _samples_cycles{Clock::cycles(1.0f / SAMPLING_RATE, clkf)}
{
    clear();
}

inline void ULAAudio::clear()
{
    std::fill(_samples.begin(), _samples.end(), 0);
}

size_t ULAAudio::tick(const Clock& clk)
{
    if (_audio_buffer) {
        _samples[_pos] = _beep * 8192;
        ++_pos;
        if (_pos == SAMPLES) {
            auto v = _audio_buffer();
            if (v) {
                std::copy(_samples.begin(), _samples.end(), v.begin());
            }

            clear();
            _pos = 0;
        }
    }

    return _samples_cycles;
}

}
}
}
