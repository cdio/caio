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
#include "mos_6581.hpp"

#include <algorithm>

#include "logger.hpp"
#include "ui.hpp"


namespace caio {

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


Mos6581::Oscillator::Oscillator(unsigned clkf, Oscillator &syncos)
    : _clkf{static_cast<float>(clkf)},
      _syncos{syncos}
{
    reset();
}

Mos6581::Oscillator::~Oscillator()
{
}

void Mos6581::Oscillator::reset()
{
    _type   = WAVE_NONE;
    _ring   = false;;
    _test   = false;
    _sync   = false;
    _ufreq  = 0.0f;
    _freq   = 0.0f;
    _T      = 0.0f;
    _uwidth = 0;
    _width  = 0.0f;
    _ndelay = 0;
    _nvalue = 0.0f;
    _A      = 0.0f;
    _t      = 0.0f;
    //_rreg   = RANDOM_IV;
    rand_reset();
}

inline void Mos6581::Oscillator::freq_hi(uint8_t fh)
{
    _ufreq = (_ufreq & 0x00FF) | (static_cast<uint16_t>(fh) << 8);
    setfreq();
}

inline void Mos6581::Oscillator::freq_lo(uint8_t fl)
{
    _ufreq = (_ufreq & 0xFF00) | fl;
    setfreq();
}

inline float Mos6581::Oscillator::freq() const {
    return _freq;
}

inline void Mos6581::Oscillator::width_hi(uint8_t wh)
{
    _uwidth = (_uwidth & 0x00FF) | (static_cast<uint16_t>(wh & 15) << 8);
    setwidth();
}

inline void Mos6581::Oscillator::width_lo(uint8_t wl)
{
    _uwidth = (_uwidth & 0x0F00) | wl;
    setwidth();
}

inline float Mos6581::Oscillator::width() const
{
    return _width;
}

inline void Mos6581::Oscillator::type(unsigned wt)
{
    _type = static_cast<WaveType>(wt);
}

inline void Mos6581::Oscillator::ring(bool rb)
{
    _ring = rb;
}

inline void Mos6581::Oscillator::test(bool tb)
{
    _test = tb;
}

inline void Mos6581::Oscillator::sync(bool sb)
{
    _sync = sb;
}

inline bool Mos6581::Oscillator::is_test() const
{
    return _test;
}

inline float Mos6581::Oscillator::amplitude() const
{
    return _A;
}

inline float Mos6581::Oscillator::time() const
{
    return _t;
}

inline void Mos6581::Oscillator::setfreq()
{
    _freq = static_cast<float>(_ufreq) * _clkf / 16777216.0f;
    _T = 1.0f / _freq;
}

inline void Mos6581::Oscillator::setwidth()
{
    _width = ((_uwidth == 0) ? 1.0f : static_cast<float>(_uwidth) / 4095.0f);
}

inline void Mos6581::Oscillator::rand_reset()
{
    _rreg = RANDOM_IV;
}

uint8_t Mos6581::Oscillator::rand()
{
    /*
     * See http://www.sidmusic.org/sid/sidtech5.html.
     */
    uint8_t value =
        ((_rreg & (1 << 22)) ? 0x80 : 0x00) |
        ((_rreg & (1 << 20)) ? 0x40 : 0x00) |
        ((_rreg & (1 << 16)) ? 0x20 : 0x00) |
        ((_rreg & (1 << 13)) ? 0x10 : 0x00) |
        ((_rreg & (1 << 11)) ? 0x08 : 0x00) |
        ((_rreg & (1 <<  7)) ? 0x04 : 0x00) |
        ((_rreg & (1 <<  4)) ? 0x02 : 0x00) |
        ((_rreg & (1 <<  2)) ? 0x01 : 0x00);

    bool bit22 = _rreg & (1 << 22);
    bool bit17 = _rreg & (1 << 17);

    _rreg = (_rreg << 1) | (bit22 ^ bit17);

    return value;
}

float Mos6581::Oscillator::noise()
{
    /*
     * See http://www.sidmusic.org/sid/sidtech5.html.
     */
    if (_ndelay <= 0) {
        _ndelay += NOISE_DELAY / (_clkf / Mos6581::SAMPLING_RATE);
        _nvalue = (rand() - 128.0f) / 128.0f;
    }

    _ndelay -= _ufreq;

    return _nvalue;
}

float Mos6581::Oscillator::tick()
{
    if (_test) {
        _A = (_type == WAVE_PULSE ? 1.0f : 0.0f);
        return _A;
    }

    if (_sync) {
        _t = _syncos.time();
        if (_t >= _T) {
            _t = std::abs(std::remainder(_t, _T));
        }
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
            _A *= noise();
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

Mos6581::Envelope::Envelope(unsigned clkf)
    : _tadj{1000000.0f / static_cast<float>(clkf)}
{
    reset();
}

Mos6581::Envelope::~Envelope()
{
}

void Mos6581::Envelope::reset()
{
    _attack_time  = attack_times[0];
    _decay_time   = decay_times[0];
    _sustain      = 0.0f;
    _release_time = decay_times[0];
    _release_A    = 0.0f;
    _t            = 0.0f;
    _A            = 0.0f;
    _gate         = false;
    _cycle        = CYCLE_NONE;
}

inline void Mos6581::Envelope::attack(uint8_t value)
{
    _attack_time = attack_times[value] * _tadj;
}

inline void Mos6581::Envelope::decay(uint8_t value)
{
    _decay_time = decay_times[value] * _tadj;
}

inline void Mos6581::Envelope::sustain(uint8_t value)
{
    _sustain = static_cast<float>(value) / 15.0f;
}

inline void Mos6581::Envelope::release(uint8_t value)
{
    _release_time = decay_times[value] * _tadj;
}

inline float Mos6581::Envelope::amplitude() const
{
    return _A;
}

void Mos6581::Envelope::gate(bool gb)
{
    if (gb && !_gate) {
        _t = _A * _attack_time;
        _cycle = CYCLE_ATTACK;
    }

    _gate = gb;
}

float Mos6581::Envelope::tick()
{
    if (_gate) {
        /*
         * GATE is ON: Attack-Decay-Sustain cycle.
         */
        switch (_cycle) {
        case CYCLE_ATTACK:
            if (_t < _attack_time) {
                _A += _t / _attack_time;
                if (_A < 1.0f) {
                    break;
                }
            }

            _t = 0.0f;
            _A = 1.0f;
            _cycle = CYCLE_DECAY;
            /* PASSTHROUGH */

        case CYCLE_DECAY:
            if (_t < _decay_time) {
                _A = signal::exp(_sustain, 1.0f - _sustain, _t, _decay_time / 4.0f);
                if (_A > _sustain) {
                    break;
                }
            }

            _t = 0.0f;
            _A = _sustain;
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
        case CYCLE_ATTACK:
        case CYCLE_DECAY:
        case CYCLE_SUSTAIN:
            _t = 0.0f;
            _release_A = _A;
            _cycle = CYCLE_RELEASE;
            /* PASSTHROUGH */

        case CYCLE_RELEASE:
            if (_t < _release_time) {
                _A = signal::exp(0.0f, _release_A, _t, _release_time / 4.0f);
                break;
            }

            _t = 0.0f;
            _A = 0.0f;
            _cycle = CYCLE_NONE;
            /* PASSTHROUGH */

        case CYCLE_NONE:
            break;
        }
    }

    if (_cycle != CYCLE_NONE) {
        _t += DT;
    }

    return _A;
}

Mos6581::Voice::Voice(unsigned clkf, Voice &svoice)
    : _osc{clkf, svoice._osc},
      _env{clkf}
{
}

Mos6581::Voice::~Voice()
{
}

inline void Mos6581::Voice::reset()
{
    _osc.reset();
    _env.reset();
}

inline void Mos6581::Voice::freq_hi(uint8_t fh)
{
    _osc.freq_hi(fh);
}

inline void Mos6581::Voice::freq_lo(uint8_t fl)
{
    _osc.freq_lo(fl);
}

inline void Mos6581::Voice::width_hi(uint8_t pwh)
{
    _osc.width_hi(pwh);
}

inline void Mos6581::Voice::width_lo(uint8_t pwl)
{
    _osc.width_lo(pwl);
}

inline void Mos6581::Voice::attack(uint8_t value)
{
    _env.attack(value);
}

inline void Mos6581::Voice::decay(uint8_t value)
{
    _env.decay(value);
}

inline void Mos6581::Voice::sustain(uint8_t value)
{
    _env.sustain(value);
}

inline void Mos6581::Voice::release(uint8_t value)
{
    _env.release(value);
}

void Mos6581::Voice::control(uint8_t value)
{
    _osc.type(value >> 4);
    _osc.test(value & 8);
    _osc.ring(value & 4);
    _osc.sync(value & 2);
    _env.gate(value & 1);
}

inline float Mos6581::Voice::tick()
{
    return _osc.tick() * _env.tick() * 0.50f;
}

inline Mos6581::Oscillator &Mos6581::Voice::osc()
{
    return _osc;
}

inline const Mos6581::Oscillator &Mos6581::Voice::osc() const
{
    return _osc;
}

inline const Mos6581::Envelope &Mos6581::Voice::env() const
{
    return _env;
}

Mos6581::Filter::Filter()
{
    reset();
}

Mos6581::Filter::~Filter()
{
}

void Mos6581::Filter::reset()
{
    _ufc      = 0;
    _pufc     = 0.0f;
    _res      = 0;
    _pres     = 0;
    _lopass   = false;
    _hipass   = false;
    _bandpass = false;

    _klo_data.fill(0.0f);
    _khi_data.fill(0.0f);
    _kba_data.fill(0.0f);
}

inline void Mos6581::Filter::freq_hi(uint8_t hi)
{
    _ufc = (_ufc & 7) | (static_cast<uint16_t>(hi) << 3);
}

inline void Mos6581::Filter::freq_lo(uint8_t lo)
{
    _ufc = (_ufc & 0xFFF8) | (lo & 7);
}

inline void Mos6581::Filter::resonance(uint8_t rs)
{
    _res = rs & 15;
}

inline void Mos6581::Filter::lopass(bool active)
{
    _lopass = active;
}

inline void Mos6581::Filter::hipass(bool active)
{
    _hipass = active;
}

inline void Mos6581::Filter::bandpass(bool active)
{
    _bandpass = active;
}

inline bool Mos6581::Filter::lopass() const
{
    return _lopass;
}

inline bool Mos6581::Filter::hipass() const
{
    return _hipass;
}

inline bool Mos6581::Filter::bandpass() const
{
    return _bandpass;
}

inline bool Mos6581::Filter::is_enabled() const
{
    return (_lopass || _hipass || _bandpass);
}

inline bool Mos6581::Filter::is_disabled() const
{
    return (!is_enabled());
}

inline float Mos6581::Filter::frequency() const
{
    /*
     * MOS-6581 does not follow the specs, MOS-8580 does, kind of.
     *
     *     fc = FC_MIN + (FC_MAX - FC_MIN) * static_cast<float>(_ufc) / 2048.0f;
     *
     * I don't have a physical C64 here so I am approximating the
     * frequencies (found by the reSID v0.16 author through reverse
     * engineering) using a Sigmoid function (see doc/sid.md).
     */
    uint16_t m = 1024;
    float s0{};
    float sm{};
    float b0{};
    float b1{};

    if (_ufc < 1024) {
        s0 = 215.0f;
        sm = 17000.0f;
        b0 = -0.65f;
        b1 = 0.0072f;
    } else {
        s0 = 1024.0f;
        sm = 18200.0f;
        b0 = -1.30f;
        b1 = 0.0055f;
    }

    float fc = s0 + (sm - s0) / (1 + std::exp(-b0 - b1 * (_ufc - m)));
    return fc;
}

inline float Mos6581::Filter::Q() const
{
    /*
     * Q between 0.707 and 1.7.
     * FIXME: These values do not seem to be correct.
     */
    float Q = 0.707f + (1.7f - 0.707f) * (static_cast<float>(_res) / 15.0f);
    return Q;
}

void Mos6581::Filter::generate()
{
    if (_pufc != _ufc || _pres != _res) {
        _pufc = _ufc;
        _pres = _res;

        float fc = frequency();
        float Q = Filter::Q();

        _klo = samples_fp{_klo_data};
        _klo = signal::lopass_40(_klo, fc, Q, SAMPLING_RATE);

        _khi = samples_fp{_khi_data};
        _khi = signal::hipass_40(_khi, fc, Q, SAMPLING_RATE);

        _kba = samples_fp{_kba_data};
        _kba = signal::bapass_20(_kba, fc, SAMPLING_RATE);
    }
}

samples_fp Mos6581::Filter::apply(samples_fp &v)
{
    generate();

    if (_lopass) {
        v = signal::conv(v, v, _klo, signal::ConvShape::Central);
    }

    if (_hipass) {
        v = signal::conv(v, v, _khi, signal::ConvShape::Central);
    }

    if (_bandpass) {
        v = signal::conv(v, v, _kba, signal::ConvShape::Central);
    }

    return v;
}

Mos6581::Mos6581(const std::string &label, unsigned clkf)
    : Mos6581I{label, clkf},
      _voice_1{clkf, _voice_3},
      _voice_2{clkf, _voice_1},
      _voice_3{clkf, _voice_2}
{
    _samples_cycles = Clock::cycles(DT, clkf);
}

void Mos6581::reset()
{
    _voice_1.reset();
    _voice_2.reset();
    _voice_3.reset();

    _filter.reset();

    _v1 = {};
    _v2 = {};
    _v3 = {};
    _v4 = {};

    _voice_1_filtered = false;
    _voice_2_filtered = false;
    _voice_3_filtered = false;
    _voice_3_off      = false;

    _volume       = 0.0f;
    _prev_volume  = 0.0f;
    _sample_index = 0;
    _prev_index   = 0;
    _last_value   = 0;
}

uint8_t Mos6581::read(addr_t addr) const
{
    switch (addr) {
    case ADC_1:
    case ADC_2:
        /* TODO */
        break;

    case VOICE_3_OSC:
        return (signal::to_i16(_voice_3.osc().amplitude()) >> 8);

    case VOICE_3_ENV:
        return (signal::to_i16(_voice_3.env().amplitude()) >> 8);

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
        if (_voice_3.osc().is_test()) {
            _voice_3.osc().rand_reset();
        }
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
        _volume = static_cast<float>(value & 15) / 15.0f;
        _filter.lopass(value & 0x10);
        _filter.bandpass(value & 0x20);
        _filter.hipass(value & 0x40);
        _voice_3_off = value & 0x80;

        /* Volume bug or "fourth voice" */
        if (_prev_volume != _volume) {
            _prev_volume = _volume;
            float value = _volume * 2.0f - 1.0f;
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
    if (_audio_buffer) {
        _v1[_sample_index] = _voice_1.tick();
        _v2[_sample_index] = _voice_2.tick();
        _v3[_sample_index] = _voice_3.tick();

        /*
         * When a voice is filtered but the filter is disabled the sampled value is set to 0.
         * This behaviour together with the oscillator test mode allows the generation of PWM signals.
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
    }

    return _samples_cycles;
}

void Mos6581::play()
{
    auto v = _audio_buffer();
    if (v) {
        samples_fp v1{_v1};
        samples_fp v2{_v2};
        samples_fp v3{_v3};

        if (_filter.is_enabled()) {
            /* FIXME: optimise */
            if (is_v1_filtered()) {
                v1 = _filter.apply(v1);
            }

            if (is_v2_filtered()) {
                v2 = _filter.apply(v2);
            }

            if (is_v3_filtered()) {
                v3 = _filter.apply(v3);
            }
        }

        auto it1 = v1.begin();
        auto it2 = v2.begin();
        auto it3 = v3.begin();

        for (size_t i = 0; i < v.size(); ++i) {
            float value1 = *it1++;
            float value2 = *it2++;
            float value3 = (is_v3_active() ? *it3++ : 0.0f);

            float value = value1 + value2 + value3 + _v4[i];


            //XXX FIXME
#if 0
    each signal level can reach Qmax, so the maximum level is not 1.
    To have a final result of 1 the signal must be divided by Qmax

    The final signal (if all voices are enabled) could reach 3*Qmax, that is
    the factor the final value must divided by.
#endif

            value = std::max(std::min(value, 1.0f), -1.0f);

            /*
             * Audio output varies from 0.8 to 0.3 (typical values)
             * depending on the active voices.
             * (See MOS6581, Electrical characteristics, Audio Voltage Output).
             * Here we just limit the volume using a fixed factor VOUT_MAX.
             */
            v[i] = signal::to_i16(value * _volume * VOUT_MAX);
        }

        _v4.fill(0.0f);
        v.dispatch();
    }
}

inline bool Mos6581::is_v1_filtered() const
{
    return _voice_1_filtered;
}

inline bool Mos6581::is_v2_filtered() const
{
    return _voice_2_filtered;
}

inline bool Mos6581::is_v3_filtered() const
{
    return _voice_3_filtered;
}

inline bool Mos6581::is_v3_active() const
{
    return (_voice_3_filtered || !_voice_3_off);
}

}
