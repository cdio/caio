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
#include "mos_6581.hpp"

#include <algorithm>
#include <cmath>

#include "logger.hpp"
#include "ui.hpp"

namespace caio {
namespace mos {
namespace mos_6581 {

/*
 * Attack time table (seconds).
 */
const std::array<fp_t, 16> Envelope::attack_times = {
    0.002, 0.008, 0.016, 0.024, 0.038, 0.056, 0.068, 0.080,
    0.100, 0.250, 0.500, 0.800, 1.000, 3.000, 5.000, 8.000
};

/*
 * Release/Decay time table (seconds).
 */
const std::array<fp_t, 16> Envelope::decay_times = {
    0.006, 0.024, 0.048, 0.072, 0.114, 0.168,  0.204,  0.240,
    0.300, 0.750, 1.500, 2.400, 3.000, 9.000, 15.000, 24.000
};

Oscillator::Oscillator(unsigned clkf, Oscillator& syncos)
    : _clkf{static_cast<fp_t>(clkf)},
      _syncos{syncos}
{
    reset();
}

void Oscillator::reset()
{
    _type   = WAVE_NONE;
    _ring   = false;;
    _test   = false;
    _sync   = false;
    _ufreq  = 0.0;
    _freq   = 0.0;
    _T      = 0.0;
    _uwidth = 0;
    _width  = 0.0;
    _ndelay = 0;
    _nvalue = 0.0;
    _A      = 0.0;
    _t      = 0.0;
    //_rreg   = RANDOM_IV;
    rand_reset();
}

inline void Oscillator::freq_hi(uint8_t fh)
{
    _ufreq = (_ufreq & 0x00FF) | (static_cast<uint16_t>(fh) << 8);
    setfreq();
}

inline void Oscillator::freq_lo(uint8_t fl)
{
    _ufreq = (_ufreq & 0xFF00) | fl;
    setfreq();
}

inline fp_t Oscillator::freq() const
{
    return _freq;
}

inline void Oscillator::width_hi(uint8_t wh)
{
    _uwidth = (_uwidth & 0x00FF) | (static_cast<uint16_t>(wh & 15) << 8);
    setwidth();
}

inline void Oscillator::width_lo(uint8_t wl)
{
    _uwidth = (_uwidth & 0x0F00) | wl;
    setwidth();
}

inline fp_t Oscillator::width() const
{
    return _width;
}

inline void Oscillator::type(unsigned wt)
{
    _type = static_cast<WaveType>(wt);
}

inline void Oscillator::ring(bool rb)
{
    _ring = rb;
}

inline void Oscillator::test(bool tb)
{
    _test = tb;
}

inline void Oscillator::sync(bool sb)
{
    _sync = sb;
}

inline bool Oscillator::is_test() const
{
    return _test;
}

inline fp_t Oscillator::amplitude() const
{
    return _A;
}

inline fp_t Oscillator::time() const
{
    return _t;
}

inline void Oscillator::setfreq()
{
    _freq = static_cast<fp_t>(_ufreq) * _clkf / 16777216.0;
    _T = 1.0 / _freq;
}

inline void Oscillator::setwidth()
{
    _width = ((_uwidth == 0) ? 1.0 : static_cast<fp_t>(_uwidth) / 4096.0);
}

inline void Oscillator::rand_reset()
{
    _rreg = RANDOM_IV;
}

uint8_t Oscillator::rand()
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

fp_t Oscillator::noise()
{
    /*
     * See http://www.sidmusic.org/sid/sidtech5.html.
     */
    if (_ndelay <= 0) {
        _ndelay += NOISE_DELAY / (_clkf / SAMPLING_RATE);
        _nvalue = (rand() - 128.0) / 128.0;
    }

    _ndelay -= _ufreq;

    return _nvalue;
}

fp_t Oscillator::tick()
{
    constexpr fp_t ATT = 0.5;

    if (_test) {
        _A = (_type == WAVE_PULSE ? ATT : 0.0);
        return _A;
    }

    if (_sync) {
        _t = _syncos.time();
        if (_t >= _T) {
            _t = std::fabs(std::remainder(_t, _T));
        }
    }

    if (_type != WAVE_NONE) {
        int16_t a = 0xFFFF;

        if (_type & WAVE_TRIANGLE) {
            a &= utils::to_i16(signal::triangle(_t, _T) * ((_ring) ? _syncos.amplitude() : 1.0));
        }

        if (_type & WAVE_SAWTOOTH) {
            a &= utils::to_i16(signal::sawtooth(_t, _T));
        }

        if (_type & WAVE_PULSE) {
            a &= utils::to_i16(signal::pulse(_t, _T * _width));
        }

        if (_type & WAVE_NOISE) {
            a &= utils::to_i16(noise());
        }

        _A = a / 32768.0;
    } else {
        _A = 0.0;
    }

    _t += DT;

    if (_t >= _T) {
        _t = 0.0;
    }

    _A *= ATT;

    return _A;
}

Envelope::Envelope(unsigned clkf)
    : _tadj{static_cast<fp_t>(1'000'000.0) / static_cast<fp_t>(clkf)}
{
    reset();
}

void Envelope::reset()
{
    _attack_time  = attack_times[0];
    _decay_time   = decay_times[0];
    _sustain      = 0.0;
    _release_time = decay_times[0];
    _release_A    = 0.0;
    _t            = 0.0;
    _A            = 0.0;
    _gate         = false;
    _cycle        = CYCLE_NONE;
}

inline void Envelope::attack(uint8_t value)
{
    _attack_time = attack_times[value] * _tadj;
}

inline void Envelope::decay(uint8_t value)
{
    _decay_time = decay_times[value] * _tadj;
}

inline void Envelope::sustain(uint8_t value)
{
    _sustain = static_cast<fp_t>(value) / 16.0;
}

inline void Envelope::release(uint8_t value)
{
    _release_time = decay_times[value] * _tadj;
}

inline fp_t Envelope::amplitude() const
{
    return _A;
}

void Envelope::gate(bool gb)
{
    if (gb && !_gate) {
        _t = _A * _attack_time;
        _cycle = CYCLE_ATTACK;
    }

    _gate = gb;
}

fp_t Envelope::tick()
{
    if (_gate) {
        /*
         * GATE is ON: Attack-Decay-Sustain cycle.
         */
        switch (_cycle) {
        case CYCLE_ATTACK:
            if (_t < _attack_time) {
                _A += _t / _attack_time;
                if (_A < 1.0) {
                    break;
                }
            }

            _t = 0.0;
            _A = 1.0;
            _cycle = CYCLE_DECAY;
            /* PASSTHROUGH */

        case CYCLE_DECAY:
            if (_t < _decay_time) {
                _A = 1.0 - _t * (1.0 - _sustain) / _decay_time;
                if (_A > _sustain) {
                    break;
                }
            }

            _t = 0.0;
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
            _t = 0.0;
            _release_A = _A;
            _cycle = CYCLE_RELEASE;
            /* PASSTHROUGH */

        case CYCLE_RELEASE:
            if (_t < _release_time) {
                _A = signal::exp(0.0, _release_A, _t, _release_time / 4.0);
                break;
            }

            _t = 0.0;
            _A = 0.0;
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

Voice::Voice(unsigned clkf, Voice& svoice)
    : _osc{clkf, svoice._osc},
      _env{clkf}
{
}

inline void Voice::reset()
{
    _osc.reset();
    _env.reset();
}

inline void Voice::freq_hi(uint8_t fh)
{
    _osc.freq_hi(fh);
}

inline void Voice::freq_lo(uint8_t fl)
{
    _osc.freq_lo(fl);
}

inline void Voice::width_hi(uint8_t pwh)
{
    _osc.width_hi(pwh);
}

inline void Voice::width_lo(uint8_t pwl)
{
    _osc.width_lo(pwl);
}

inline void Voice::attack(uint8_t value)
{
    _env.attack(value);
}

inline void Voice::decay(uint8_t value)
{
    _env.decay(value);
}

inline void Voice::sustain(uint8_t value)
{
    _env.sustain(value);
}

inline void Voice::release(uint8_t value)
{
    _env.release(value);
}

void Voice::control(uint8_t value)
{
    _osc.type(value >> 4);
    _osc.test(value & 8);
    _osc.ring(value & 4);
    _osc.sync(value & 2);
    _env.gate(value & 1);
}

inline fp_t Voice::tick()
{
    return _osc.tick() * _env.tick();
}

inline Oscillator& Voice::osc()
{
    return _osc;
}

inline const Oscillator& Voice::osc() const
{
    return _osc;
}

inline const Envelope& Voice::env() const
{
    return _env;
}

Filter::Filter()
{
    reset();
}

void Filter::reset()
{
    _ufc    = 0;
    _pufc   = 0.0;
    _res    = 0;
    _pres   = 0;
    _type   = FILTER_NONE;
}

inline void Filter::freq_hi(uint8_t hi)
{
    _ufc = (_ufc & 7) | (static_cast<uint16_t>(hi) << 3);
    generate();
}

inline void Filter::freq_lo(uint8_t lo)
{
    _ufc = (_ufc & 0xFFF8) | (lo & 7);
    generate();
}

inline void Filter::resonance(uint8_t rs)
{
    _res = rs & 15;
    generate();
}

inline void Filter::type(uint8_t type)
{
    _type = static_cast<FilterType>(type & FILTER_MASK);
    generate();
}

inline bool Filter::is_enabled() const
{
    return (_type != FILTER_NONE);
}

inline bool Filter::is_disabled() const
{
    return (!is_enabled());
}

inline fp_t Filter::frequency() const
{
    /*
     * MOS-6581 does not follow the specs, MOS-8580 does:
     *   fc = FC_MIN + (FC_MAX - FC_MIN) * _ufc / 2048;
     */
    static constexpr fp_t m = 1024.0;

    fp_t s0{};
    fp_t sm{};
    fp_t b0{};
    fp_t b1{};

    /* FIXME:
     * this function approximates the lookup table
     * from resid-0.16 but that one seems to be incorrect.
     * Extract the proper frequency values from a real 6581.
     */
    if (_ufc < 1024) {
        s0 = 215.0;
        sm = 17000.0;
        b0 = -0.65;
        b1 = 0.0072;
    } else {
        s0 = 1024.0;
        sm = 18200.0;
        b0 = -1.30;
        b1 = 0.0055;
    }

    fp_t fc = s0 + (sm - s0) / (1.0 + std::exp(-b0 - b1 * (_ufc - m)));

//XXX    fc = 30.0 + (12000.0 - 30.0) * _ufc / 2048.0;
    return fc;
}

inline fp_t Filter::Q() const
{
    /*
     * Qmax limited to 2.5.
     * Q = [ 0.707; 2.5 ]
     * 1/Q = 1/Q0 + res / Qr = sqrt(2) + res / Qr
     */
    const fp_t Q = 1.0 / (1.4143 - _res / 14.79);
    return Q;
}

inline fp_t Filter::operator()(fp_t v)
{
    return _flt(v) * _gain_comp;
}

void Filter::generate()
{
    if (_pufc != _ufc || _pres != _res || _ptype != _type) {
        _pufc = _ufc;
        _pres = _res;
        _ptype = _type;

        const fp_t fc = frequency();
        const fp_t Q = Filter::Q();

        _gain_comp = 1.0;

//        log.debug("type: {:x}, fc={}, Q={}\n", +_type, fc, Q);

        switch (_type) {
        case FILTER_LO:
            _flt = signal::iir_lopass40(fc, Q, SAMPLING_RATE);
            break;
        case FILTER_HI:
            _flt = signal::iir_hipass40(fc, Q, SAMPLING_RATE);
            break;
        case FILTER_BA:
            _gain_comp *= 2.0;
            _flt = signal::iir_bapass20(fc, Q, SAMPLING_RATE);
            break;
        case FILTER_LOHI:
            _gain_comp *= 2.0;
            _flt = signal::iir_hipass40(fc, Q, SAMPLING_RATE) +
                   signal::iir_lopass40(fc, Q, SAMPLING_RATE);
            break;
        case FILTER_LOBA:
            _flt = signal::iir_lopass40(fc, Q, SAMPLING_RATE);
            break;
        case FILTER_HIBA:
            _flt = signal::iir_hipass40(fc, Q, SAMPLING_RATE);
            break;
        case FILTER_LHBA:
            _gain_comp *= 3.0;
            _flt = signal::iir_hipass40(fc, Q, SAMPLING_RATE) +
                   signal::iir_lopass40(fc, Q, SAMPLING_RATE) +
                   signal::iir_bapass20(fc, Q, SAMPLING_RATE);
            break;
        default:;
        }

//        log.debug("b={}; a={};\n\n", signal::to_string(_flt.num), signal::to_string(_flt.den));
    }
}

Mos6581::Mos6581(std::string_view label, unsigned clkf)
    : Device{TYPE, label},
      _samples_cycles{Clock::cycles(DT, clkf)},
      _voice_1{clkf, _voice_3},
      _voice_2{clkf, _voice_1},
      _voice_3{clkf, _voice_2}
{
}

void Mos6581::reset()
{
    _voice_1.reset();
    _voice_2.reset();
    _voice_3.reset();

    _filter.reset();

    _v.fill(0.0);
    _v4.fill(0.0);

    _voice_1_filtered = false;
    _voice_2_filtered = false;
    _voice_3_filtered = false;
    _voice_3_off      = false;

    _volume       = 0.0;
    _prev_volume  = 0.0;
    _sample_index = 0;
    _prev_index   = 0;
    _last_value   = 0;
}

uint8_t Mos6581::dev_read(size_t addr, ReadMode)
{
    switch (addr) {
    case ADC_1:
    case ADC_2:
        /* TODO */
        break;

    case VOICE_3_OSC:
        return (utils::to_i16(_voice_3.osc().amplitude()) >> 8);

    case VOICE_3_ENV:
        return (utils::to_i16(_voice_3.env().amplitude()) >> 8);

    default:;
    }

    return _last_value;
}

void Mos6581::dev_write(size_t addr, uint8_t value)
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
        /* value & 8: External audio input not supported */
        break;

    case FILTER_MODE:
        _volume = 0.8 * static_cast<fp_t>(value & 15) / 16.0;
        _voice_3_off = value & 0x80;
        _filter.type(value);

        /* Volume bug or "fourth voice" */
        if (_prev_volume != _volume) {
            _prev_volume = _volume;
            fp_t value = _volume * 2.0 - 1.0;
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

std::ostream& Mos6581::dump(std::ostream& os, size_t base) const
{
    std::array<uint8_t, REGMAX> regs{};
    std::fill(regs.begin(), regs.end(), 255);

    regs[ADC_1] = peek(ADC_1);
    regs[ADC_2] = peek(ADC_2);
    regs[VOICE_3_OSC] = peek(VOICE_3_OSC);
    regs[VOICE_3_ENV] = peek(VOICE_3_ENV);

    return utils::dump(os, regs, base);
}

size_t Mos6581::tick(const Clock& clk)
{
    if (_audio_buffer) {
        fp_t att = 0.6 - 0.3 * is_v3_active();
        fp_t s1 = _voice_1.tick() * att;
        fp_t s2 = _voice_2.tick() * att;
        fp_t s3 = _voice_3.tick() * att;

        fp_t fs = 0.0;

        if (_filter.is_disabled()) {
            s1 *= !is_v1_filtered();
            s2 *= !is_v2_filtered();
            s3 *= !is_v3_filtered();
        } else {
            fs = s1 * is_v1_filtered() + s2 * is_v2_filtered() + s3 * is_v3_active();
            fs = _filter(fs);
        }

        fp_t us = s1 * (!is_v1_filtered()) + s2 * (!is_v2_filtered()) + s3 * (!is_v3_filtered());

        _v[_sample_index] = fs + us;

        ++_sample_index;
        if (_sample_index == SAMPLES) {
            play();
            _v4.fill(0.0);
            _sample_index = 0;
        }
    }

    return _samples_cycles;
}

void Mos6581::play()
{
    auto v16 = _audio_buffer();
    if (v16) {
        for (size_t i = 0; i < SAMPLES; ++i) {
            fp_t value = _v[i] * _volume + _v4[i] * 0.3;
            value = std::clamp(value, -0.8, 0.8);
            v16[i] = utils::to_i16(value);
        }
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
    return !(_voice_3_filtered && _voice_3_off);
}

}
}
}
