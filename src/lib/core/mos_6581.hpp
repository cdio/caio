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

#include "mos_6581_i.hpp"
#include "signal.hpp"


namespace caio {
namespace mos_6581 {

class Oscillator {
public:
    constexpr static const uint32_t RANDOM_IV   = 0x007FFFF8;
    constexpr static const uint32_t NOISE_DELAY = 0x00100000;

    enum WaveType : unsigned {
        WAVE_NONE     = 0x00,
        WAVE_TRIANGLE = 0x01,
        WAVE_SAWTOOTH = 0x02,
        WAVE_PULSE    = 0x04,
        WAVE_NOISE    = 0x08
    };

    Oscillator(unsigned clkf, Oscillator& syncos);

    void  reset();
    void  freq_hi(uint8_t fh);
    void  freq_lo(uint8_t fl);
    float freq() const;
    void  width_hi(uint8_t wh);
    void  width_lo(uint8_t wl);
    float width() const;
    void  type(unsigned wt);
    void  ring(bool rb);
    void  test(bool tb);
    void  sync(bool sb);
    bool  is_test() const;
    float amplitude() const;
    float time() const;
    void  rand_reset();
    float tick();

private:
    void    setfreq();
    void    setwidth();
    uint8_t rand();
    float   noise();

    float       _clkf;
    Oscillator& _syncos;

    WaveType    _type;

    bool        _ring;
    bool        _test;
    bool        _sync;

    uint16_t    _ufreq;
    float       _freq;
    float       _T;

    uint16_t    _uwidth;
    float       _width;

    uint32_t    _rreg;
    int         _ndelay;
    float       _nvalue;

    float       _A;
    float       _t;
};

class Envelope {
public:
    enum Cycle {
        CYCLE_NONE,
        CYCLE_ATTACK,
        CYCLE_DECAY,
        CYCLE_SUSTAIN,
        CYCLE_RELEASE
    };

    Envelope(unsigned clkf);

    void  reset();
    void  attack(uint8_t value);
    void  decay(uint8_t value);
    void  sustain(uint8_t value);
    void  release(uint8_t value);
    void  gate(bool gb);
    float amplitude() const;
    float tick();

private:
    float _tadj;              /* Timing adjustment factor: 1MHz / system_clock_freq */

    float _attack_time;
    float _decay_time;
    float _sustain;
    float _release_time;
    float _release_A;

    float _t;
    float _A;

    bool  _gate;
    Cycle _cycle;

    static const std::array<float, 16> attack_times;
    static const std::array<float, 16> decay_times;
};

class Voice {
public:
    Voice(unsigned clkf, Voice& svoice);

    void  reset();
    void  freq_hi(uint8_t fh);
    void  freq_lo(uint8_t fl);
    void  width_hi(uint8_t pwh);
    void  width_lo(uint8_t pwl);
    void  attack(uint8_t value);
    void  decay(uint8_t value);
    void  sustain(uint8_t value);
    void  release(uint8_t value);
    void  control(uint8_t value);
    float tick();
    Oscillator& osc();
    const Oscillator& osc() const;
    const Envelope& env() const;

private:
    Oscillator _osc;
    Envelope   _env;
};

class Filter {
public:
    constexpr static const size_t MAX_KERNEL_SIZE = signal::kernel_size(200.0f, SAMPLING_RATE);

    Filter();

    void       reset();
    void       freq_hi(uint8_t hi);
    void       freq_lo(uint8_t lo);
    void       resonance(uint8_t rs);
    void       lopass(bool active);
    void       hipass(bool active);
    void       bandpass(bool active);
    bool       lopass() const;
    bool       hipass() const;
    bool       bandpass() const;
    bool       is_enabled() const;
    bool       is_disabled() const;
    float      frequency() const;
    float      Q() const;
    samples_fp apply(samples_fp& v);

private:
    void generate();

    uint16_t   _ufc{};
    float      _pufc{};
    uint8_t    _res{};
    float      _pres{};
    bool       _lopass{};
    bool       _hipass{};
    bool       _bandpass{};

    samples_fp _klo{};
    samples_fp _khi{};
    samples_fp _kba{};

    std::array<float, MAX_KERNEL_SIZE> _klo_data{};
    std::array<float, MAX_KERNEL_SIZE> _khi_data{};
    std::array<float, MAX_KERNEL_SIZE> _kba_data{};
};

/**
 * MOS6581 implementation from the specs.
 * The real 6581 chip does not respect its own specifications
 * consequently this implementation does not sound exactly like the real chip.
 *
 * @see Mos6581_
 * @see https://en.wikipedia.org/wiki/MOS_Technology_SID
 * @see https://web.archive.org/web/20070222065716/http://stud1.tuwien.ac.at/~e9426444/yannes.html
 * @see https://www.c64-wiki.com/wiki/Commodore_64_Programmer%27s_Reference_Guide
 * @see http://www.sidmusic.org/sid/sidtech5.html
 */
class Mos6581 : public Mos6581_ {
public:
    constexpr static const float VOUT_MAX = 0.3f;

    /**
     * Initalise this SID instance.
     * @param label Label assigned to this instance;
     * @param clkf  System clock frequency.
     */
    Mos6581(const std::string& label, unsigned clkf);

    virtual ~Mos6581() {
    }

    /**
     * @see Mos6581I::reset()
     */
    void reset() override;

    /**
     * @see Mos6581I::read()
     */
    uint8_t read(addr_t addr, ReadMode mode = ReadMode::Read) override;

    /**
     * @see Mos6581I::write()
     */
    void write(addr_t addr, uint8_t value) override;

private:
    /**
     * Generate and play an audio signal.
     * This method must be called by the system clock.
     * @param clk Caller clock.
     * @return The number of clock cycles that must pass before this method could be called again.
     * @see Clockable::tick(const Clock &)
     */
    size_t tick(const Clock& clk) override;

    void play();

    bool is_v1_filtered() const;

    bool is_v2_filtered() const;

    bool is_v3_filtered() const;

    bool is_v3_active() const;

    Voice      _voice_1;
    Voice      _voice_2;
    Voice      _voice_3;

    std::array<float, SAMPLES> _v1{};
    std::array<float, SAMPLES> _v2{};
    std::array<float, SAMPLES> _v3{};
    std::array<float, SAMPLES> _v4{};

    bool       _voice_1_filtered{};
    bool       _voice_2_filtered{};
    bool       _voice_3_filtered{};
    bool       _voice_3_off{};

    Filter     _filter{};

    float      _volume{};
    float      _prev_volume{};

    size_t     _sample_index{};
    size_t     _prev_index{};

    uint8_t    _last_value{};
};

}

using Mos6581 = mos_6581::Mos6581;

}
