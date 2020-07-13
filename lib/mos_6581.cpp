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
#include "mos_6581.hpp"

#include "logger.hpp"
#include "ui.hpp"


namespace cemu {

/*
 * Attack time table (seconds).
 */
const std::array<float, 16> Mos6581::Envelope::attack_times = {
    0.002f, 0.008f, 0.016f, 0.024f, 0.038f, 0.056f, 0.068f, 0.080f,
    0.100f, 0.250f, 0.500f, 0.800f, 1.000f, 3.000f, 5.000f, 8.000f
};


/*
 * Release/Decay time table (seconds).
 */
const std::array<float, 16> Mos6581::Envelope::decay_times = {
    0.006f, 0.024f, 0.048f, 0.072f, 0.114f, 0.168f,  0.204f,  0.240f,
    0.300f, 0.750f, 1.500f, 2.400f, 3.000f, 9.000f, 15.000f, 24.000f
};


float Mos6581::Oscillator::tick()
{
    if (_test) {
        _A = (_type == WAVE_PULSE ? 1.0f : 0.0f);
        return _A;
    }

    if (_sync) {
        _t = _syncos.time();
    }

    if (_type != WAVE_NONE) {
        _A = 1.0f;

        if (_type & WAVE_TRIANGLE) {
            _A *= signal::triangle(_t, _T) * ((_ring) ? _syncos.amplitude() : 1.0f);
        }

        if (_type & WAVE_SAWTOOTH) {
            _A *= signal::sawtooth(_t, _T);
        }

        if (_type & WAVE_PULSE) {
            _A *= signal::square(_t, _T * _width);
        }

        if (_type & WAVE_NOISE) {
            _A *= signal::rand();    // FIXME Frequency limited noise.
        }
    } else {
        _A = 0.0f;
    }

    _t += DT;

    if (_t >= _T) {
        _t = 0.0f;
    }

    return _A;
}


void Mos6581::Envelope::gate(bool gb)
{
    _gate = gb;

    /*
     * The Mos6581::tick() method is called each SAMPLES_TIME seconds.
     * When state changes are faster than that time (f.ex. sustain level 0)
     * some programs change the gate so fast that this device does not get noticed.
     * Placing the following initialisation here (instead of Envelope::tick()) this
     * envelope generator will always be in the proper state.
     */
    if (_gate) {
        _attack_slope = 1.0f / _attack_time;
        if (_attack_time + _decay_time < SAMPLES_TIME) {
            _A = 1.0f;
        }
        _cycle = CYCLE_ATTACK;
    } else {
        _release_A = _A;
        _cycle = CYCLE_RELEASE;
    }

    _t = 0.0f;
}

float Mos6581::Envelope::tick()
{
    if (_gate) {
        /*
         * GATE is ON: Attack-Decay-Sustain cycle.
         */
        switch (_cycle) {
        case CYCLE_ATTACK:
            if (_A < 1.0f) {
                _A += _attack_slope * _t;
                if (_A > 1.0f) {
                    _A = 1.0f;
                }
                break;
            }

            _t = 0.0f;
            _cycle = CYCLE_DECAY;
            /* PASSTHROUGH */

        case CYCLE_DECAY:
            if (_t < _decay_time) {
                _A = signal::exp(_sustain, 1.0f - _sustain, _t, _decay_time / 4.0f);
                break;
            }

            _t = 0.0f;
            _cycle = CYCLE_SUSTAIN;
            /* PASSTHROUGH */

        case CYCLE_SUSTAIN:
            break;

        default:;
        }
    } else {
        /*
         * GATE is OFF: Release cycle.
         */
        switch (_cycle) {
        case CYCLE_RELEASE:
            if (_t < _release_time) {
                _A = signal::exp(0.0f, _release_A, _t, _release_time / 4.0f);
                break;
            }

            _t = 0.0f;
            _A = 0.0f;
            _cycle = CYCLE_NONE;
            break;

        case CYCLE_NONE:
        default:;
        }
    }

    if (_cycle != CYCLE_NONE) {
        _t += DT;
    }

    return _A;
}


void Mos6581::Voice::control(uint8_t value)
{
    _osc.type(value >> 4);
    _osc.test(value & 8);
    _osc.ring(value & 4);
    _osc.sync(value & 2);
    _env.gate(value & 1);
}


void Mos6581::Filter::generate()
{
    if (!_generated) {
        float fc = FC_MIN + BW * static_cast<float>(_ufc) / 2048.0f;
        float rs = static_cast<float>(_resonance) / 15.0f;

        /*
         * These are almost ideal. Should we implement Chebyshev? Also to speed-up?
         */
        _klo = gsl::span{_klo_data.data(), _klo_data.size()};
        _klo = signal::lopass(_klo, fc, SAMPLING_RATE, rs);

        _khi = gsl::span{_khi_data.data(), _khi_data.size()};
        _khi = signal::hipass(_khi, fc, SAMPLING_RATE, rs);

        _kbd = gsl::span{_kbd_data.data(), _kbd_data.size()};
        _kbd = signal::bapass(_kbd, fc, fc, SAMPLING_RATE, rs);

        _generated = true;
    }
}

void Mos6581::Filter::apply(samples_fp &v)
{
    generate();

    if (_lopass) {
        signal::conv_kernel(v, _klo);
    }

    if (_hipass) {
        signal::conv_kernel(v, _khi);
    }

    if (_bandpass) {
        signal::conv_kernel(v, _kbd);
    }
}


Mos6581::Mos6581(const std::string &label, unsigned clkf)
    : Mos6581I{label, clkf},
      _voice_1{clkf, _voice_3},
      _voice_2{clkf, _voice_1},
      _voice_3{clkf, _voice_2},
      _v1(SAMPLES, 0.0f),
      _v2(SAMPLES, 0.0f),
      _v3(SAMPLES, 0.0f),
      _v4(SAMPLES, 0.0f)
{
    _samples_cycles = Clock::cycles(DT, clkf);
}

uint8_t Mos6581::read(addr_t addr) const
{
    switch (addr) {
    case ADC_1:
    case ADC_2:
        /* TODO */
        break;

    case VOICE_3_OSC:
        return (signal::to_i16(_voice_3._osc.amplitude()) >> 8);

    case VOICE_3_ENV:
        return (signal::to_i16(_voice_3._env.amplitude()) >> 8);

    default:;
    }

    return _last_value;
}

void Mos6581::write(addr_t addr, uint8_t value)
{
    switch (addr) {
    case VOICE_1_FREQ_LO:
        _voice_1.freq_lo(value);
        break;

    case VOICE_1_FREQ_HI:
        _voice_1.freq_hi(value);
        break;

    case VOICE_1_PULSE_WIDTH_LO:
        _voice_1.width_lo(value);
        break;

    case VOICE_1_PULSE_WIDTH_HI:
        _voice_1.width_hi(value);
        break;

    case VOICE_1_CONTROL:
        _voice_1.control(value);
        break;

    case VOICE_1_ATTACK_DECAY:
        _voice_1.attack(value >> 4);
        _voice_1.decay(value & 15);
        break;

    case VOICE_1_SUSTAIN_RELEASE:
        _voice_1.sustain(value >> 4);
        _voice_1.release(value & 15);
        break;

    case VOICE_2_FREQ_LO:
        _voice_2.freq_lo(value);
        break;

    case VOICE_2_FREQ_HI:
        _voice_2.freq_hi(value);
        break;

    case VOICE_2_PULSE_WIDTH_LO:
        _voice_2.width_lo(value);
        break;

    case VOICE_2_PULSE_WIDTH_HI:
        _voice_2.width_hi(value);
        break;

    case VOICE_2_CONTROL:
        _voice_2.control(value);
        break;

    case VOICE_2_ATTACK_DECAY:
        _voice_2.attack(value >> 4);
        _voice_2.decay(value & 15);
        break;

    case VOICE_2_SUSTAIN_RELEASE:
        _voice_2.sustain(value >> 4);
        _voice_2.release(value & 15);
        break;

    case VOICE_3_FREQ_LO:
        _voice_3.freq_lo(value);
        break;

    case VOICE_3_FREQ_HI:
        _voice_3.freq_hi(value);
        break;

    case VOICE_3_PULSE_WIDTH_LO:
        _voice_3.width_lo(value);
        break;

    case VOICE_3_PULSE_WIDTH_HI:
        _voice_3.width_hi(value);
        break;

    case VOICE_3_CONTROL:
        _voice_3.control(value);
        break;

    case VOICE_3_ATTACK_DECAY:
        _voice_3.attack(value >> 4);
        _voice_3.decay(value & 15);
        break;

    case VOICE_3_SUSTAIN_RELEASE:
        _voice_3.sustain(value >> 4);
        _voice_3.release(value & 15);
        break;

    case FILTER_CUTOFF_LO:
        _filter.freq_lo(value);
        break;

    case FILTER_CUTOFF_HI:
        _filter.freq_hi(value);
        break;

    case FILTER_VOICE_CONTROL:
        _filter.resonance(value >> 4);
        _voice_1_filtered = (value & 1);
        _voice_2_filtered = (value & 2);
        _voice_3_filtered = (value & 4);
        /* TODO: value & 8: External audio input. */
        break;

    case FILTER_MODE:
        _volume = 0.5f * static_cast<float>(value & 15) / 15.0f;
        _filter.lopass(value & 0x10);
        _filter.bandpass(value & 0x20);
        _filter.hipass(value & 0x40);
        _voice_3_off = value & 0x80;

        /* Volume bug or "fourth voice" */
        if (_prev_volume != _volume) {
            _prev_volume = _volume;
            float value = _volume * 4.0f - 1.0f;
            if (_prev_index < _sample_index) {
                std::fill_n(_v4.begin() + _prev_index, _sample_index - _prev_index, value);
            } else {
                std::fill(_v4.begin() + _prev_index, _v4.end(), value);
                std::fill_n(_v4.begin(), _sample_index, value);
            }
        }
        _prev_index = _sample_index;
        break;

    case ADC_1:
    case ADC_2:
    case VOICE_3_OSC:
    case VOICE_3_ENV:
    case UNUSED_1D:
    case UNUSED_1E:
    case UNUSED_1F:
    default:;
    }

    _last_value = value;
}

size_t Mos6581::tick(const Clock &clk)
{
    _v1[_sample_index] = _voice_1.tick();
    _v2[_sample_index] = _voice_2.tick();
    _v3[_sample_index] = _voice_3.tick();

    /*
     * When a voice is filtered but the filter is disabled the sampled value is set to 0.
     * This allows PWM using filter activation/deactivation.
     */
    if (_filter.is_disabled()) {
        if (is_v1_filtered()) {
            _v1[_sample_index] = 0.0f;
        }

        if (is_v2_filtered()) {
            _v2[_sample_index] = 0.0f;
        }

        if (is_v3_filtered()) {
            _v3[_sample_index] = 0.0f;
        }
    }

    ++_sample_index;
    if (_sample_index == SAMPLES) {
        _sample_index = 0;
        play();
    }

    return _samples_cycles;
}

void Mos6581::play()
{
    auto v = _ui->audio_buffer();
    if (v) {
        if (_filter.is_enabled()) {
            /* FIXME: optimise */
            if (is_v1_filtered()) {
                _filter.apply(_v1);
            }

            if (is_v2_filtered()) {
                _filter.apply(_v2);
            }

            if (is_v3_filtered()) {
                _filter.apply(_v3);
            }
        }

        for (size_t i = 0; i < v.size(); ++i) {
            float value = _v1[i] + _v2[i] + (is_v3_active() ? _v3[i] : 0.0f) + _v4[i];
            if (value > 1.0f) {
                value = 1.0f;
            } else if (value < -1.0f) {
                value = -1.0f;
            }

            v[i] = signal::to_i16(value * _volume);
        }

        std::fill(_v4.begin(), _v4.end(), 0.0f);
        v.dispatch();
    }
}

}
