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

#include <atomic>
#include <array>
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include "clock.hpp"
#include "device.hpp"
#include "ui.hpp"
#include "utils.hpp"

namespace caio {
namespace mos_6581 {

constexpr static const unsigned SAMPLING_RATE = 44100;
constexpr static const float DT               = 1.0f / SAMPLING_RATE;

constexpr static const float  SAMPLES_TIME    = 0.020f;
constexpr static const size_t SAMPLES         = static_cast<size_t>(caio::ceil(SAMPLING_RATE * SAMPLES_TIME));

constexpr static const size_t CHANNELS        = 1;

/**
 * MOS6581 Sound Interface Device (SID) base class.
 * This class must be derived by the actual implementation.
 */
class Mos6581_ : public Device, public Clockable {
public:
    constexpr static const char* TYPE = "MOS6581";

    enum Registers {
        VOICE_1_FREQ_LO         = 0x00,
        VOICE_1_FREQ_HI         = 0x01,
        VOICE_1_PULSE_WIDTH_LO  = 0x02,
        VOICE_1_PULSE_WIDTH_HI  = 0x03,
        VOICE_1_CONTROL         = 0x04,
        VOICE_1_ATTACK_DECAY    = 0x05,
        VOICE_1_SUSTAIN_RELEASE = 0x06,

        VOICE_2_FREQ_LO         = 0x07,
        VOICE_2_FREQ_HI         = 0x08,
        VOICE_2_PULSE_WIDTH_LO  = 0x09,
        VOICE_2_PULSE_WIDTH_HI  = 0x0A,
        VOICE_2_CONTROL         = 0x0B,
        VOICE_2_ATTACK_DECAY    = 0x0C,
        VOICE_2_SUSTAIN_RELEASE = 0x0D,

        VOICE_3_FREQ_LO         = 0x0E,
        VOICE_3_FREQ_HI         = 0x0F,
        VOICE_3_PULSE_WIDTH_LO  = 0x10,
        VOICE_3_PULSE_WIDTH_HI  = 0x11,
        VOICE_3_CONTROL         = 0x12,
        VOICE_3_ATTACK_DECAY    = 0x13,
        VOICE_3_SUSTAIN_RELEASE = 0x14,

        FILTER_CUTOFF_LO        = 0x15,
        FILTER_CUTOFF_HI        = 0x16,

        FILTER_VOICE_CONTROL    = 0x17,
        FILTER_MODE             = 0x18,

        ADC_1                   = 0x19,
        ADC_2                   = 0x1A,

        VOICE_3_OSC             = 0x1B,
        VOICE_3_ENV             = 0x1C,

        UNUSED_1D               = 0x1D,
        UNUSED_1E               = 0x1E,
        UNUSED_1F               = 0x1F,

        REGMAX
    };

    constexpr static const addr_t SIZE = Registers::REGMAX;

    virtual ~Mos6581_() {
    }

    /**
     * Set the audio buffer provider.
     * @param abuf Audio buffer provider.
     */
    void audio_buffer(const std::function<ui::AudioBuffer()>& abuf) {
        _audio_buffer = abuf;
    }

    /**
     * @see Device::reset()
     */
    void reset() override {
    }

    /**
     * @see Device::size()
     */
    size_t size() const override {
        return SIZE;
    }

    /**
     * @see Device::dump()
     */
    std::ostream& dump(std::ostream& os, addr_t base = 0) const override;

protected:
    /**
     * Initalise this SID instance.
     * @param label Label assigned to this instance;
     * @param clkf  System clock frequency.
     */
    Mos6581_(const std::string& label, unsigned clkf)
        : Device{TYPE, label},
          _clkf{clkf},
          _samples_cycles{Clock::cycles(SAMPLES_TIME, clkf)} {
    }

    std::function<ui::AudioBuffer()> _audio_buffer{};
    unsigned                         _clkf{};
    size_t                           _samples_cycles{};  /* SAMPLES_TIME as clock cycles */
};

}

using Mos6581_ = mos_6581::Mos6581_;

}
