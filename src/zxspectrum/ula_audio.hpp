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

#include <functional>
#include <string_view>

#include "clock.hpp"
#include "signal.hpp"
#include "types.hpp"
#include "ui.hpp"
#include "utils.hpp"

namespace caio {
namespace sinclair {
namespace zxspectrum {

/**
 * ULA Audio interface.
 * The ULA Audio interface generates BEEPs (square signals).
 */
class ULAAudio : public Clockable, public Name {
public:
    constexpr static const char* TYPE               = "ULA-AUDIO";
    constexpr static const unsigned SAMPLING_RATE   = 44100;
    constexpr static const float  SAMPLES_TIME      = 0.020f;
    constexpr static const size_t SAMPLES           = static_cast<size_t>(utils::ceil(SAMPLING_RATE * SAMPLES_TIME));
    constexpr static const size_t CHANNELS          = 1;

    /**
     * Initalise this ULA Audio interface.
     * @param clkf  System clock frequency;
     * @param label Label assigned to this instance.
     */
    ULAAudio(unsigned clkf, std::string_view label);

    virtual ~ULAAudio() {
    }

    /**
     * Set the audio buffer provider.
     * @param abuf Audio buffer provider.
     * @see ui::AudioBuffer
     */
    void audio_buffer(const std::function<ui::AudioBuffer()>& abuf) {
        _audio_buffer = abuf;
    }

    /**
     * Beep.
     * @param vol Beep volume (between 0 and 1).
     */
    void beep(float vol) {
        _beep = ((vol > 1.0f) ? 1.0f : (vol < 0.0f ? 0.0f : vol));
    }

private:
    /**
     * Audio sample tick.
     * This method is called by the system clock at SAMPLING_RATE intervals.
     * It fills a buffer with audio samples and delivers it to the audio
     * handler at intervals of SAMPLES_TIME seconds.
     * @param clk Caller clock.
     * @return The number of clock cycles that must pass before calling this method again.
     * @see SAMPLING_RATE
     * @see SAMPLES_TIME
     * @see audio_buffer(const std::function<ui::AudioBuffer()>&)
     * @see Clockable::tick(const Clock&)
     */
    size_t tick(const Clock& clk) override;

    /**
     * Clear the samples buffer.
     */
    void clear();

    size_t                           _samples_cycles;
    int16_t                          _samples_buffer[SAMPLES];

    samples_i16                      _samples{ _samples_buffer, std::size(_samples_buffer) };
    size_t                           _pos{};
    float                            _beep{};
    std::function<ui::AudioBuffer()> _audio_buffer{};
};

}
}
}
