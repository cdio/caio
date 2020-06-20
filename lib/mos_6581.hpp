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
#pragma once

#include "mos_6581_i.hpp"
#include "signal.hpp"


namespace cemu {

/**
 * MOS6581 implemmentation from the specs.
 * The real 6581 chip does not respect its own specifications
 * consequently this implementation does not sound exactly like the real chip.
 *
 * @see Mos6581I
 * @see https://en.wikipedia.org/wiki/MOS_Technology_SID
 * @see https://web.archive.org/web/20070222065716/http://stud1.tuwien.ac.at/~e9426444/yannes.html
 * @see https://www.c64-wiki.com/wiki/Commodore_64_Programmer%27s_Reference_Guide
 */
class Mos6581 : public Mos6581I {
public:
    constexpr static const float FC_MAX = 12000.0f;
    constexpr static const float FC_MIN = 30.0f;
    constexpr static const float BW     = FC_MAX - FC_MIN;

    class Oscillator {
    public:
        enum WaveType : unsigned {
            WAVE_NONE     = 0x00,
            WAVE_TRIANGLE = 0x01,
            WAVE_SAWTOOTH = 0x02,
            WAVE_PULSE    = 0x04,
            WAVE_NOISE    = 0x08
        };

        Oscillator(unsigned clkf, Oscillator &syncos)
            : _clkf{static_cast<float>(clkf)},
              _syncos{syncos} {
        }

        ~Oscillator() {
        }

        void freq_hi(uint8_t fh) {
            _ufreq = (_ufreq & 0x00FF) | (static_cast<uint16_t>(fh) << 8);
            setfreq();
        }

        void freq_lo(uint8_t fl) {
            _ufreq = (_ufreq & 0xFF00) | fl;
            setfreq();
        }

        float freq() const {
            return _freq;
        }

        void width_hi(uint8_t wh) {
            _uwidth = (_uwidth & 0x00FF) | (static_cast<uint16_t>(wh & 15) << 8);
            setwidth();
        }

        void width_lo(uint8_t wl) {
            _uwidth = (_uwidth & 0x0F00) | wl;
            setwidth();
        }

        float width() const {
            return _width;
        }

        void type(unsigned wt) {
            _type = static_cast<WaveType>(wt);
        }

        void ring(bool rb) {
            _ring = rb;
        }

        void test(bool tb) {
            _test = tb;
        }

        void sync(bool sb) {
            _sync = sb;
        }

        float amplitude() const {
            return _A;
        }

        float time() const {
            return _t;
        }

        float tick();

    private:
        void setfreq() {
            _freq = static_cast<float>(_ufreq) * _clkf / 16777216.0f;
            _T = 1.0f / _freq;
        }

        void setwidth() {
            _width = ((_uwidth == 0) ? 1.0f : static_cast<float>(_uwidth) / 4095.0f);
        }

        float       _clkf;
        Oscillator &_syncos;

        WaveType    _type{};

        bool        _ring{};
        bool        _test{};
        bool        _sync{};

        uint16_t    _ufreq{};
        float       _freq{};
        float       _T{};

        uint16_t    _uwidth{};
        float       _width{};

        float       _A{};
        float       _t{};
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

        Envelope(unsigned clkf)
            : _tadj{1000000.0f / static_cast<float>(clkf)} {
        }

        ~Envelope() {
        }

        void attack(uint8_t value) {
            _attack_time = attack_times[value] * _tadj;
        }

        void decay(uint8_t value) {
            _decay_time = decay_times[value] * _tadj;
        }

        void sustain(uint8_t value) {
            _sustain = static_cast<float>(value) / 15.0f;
        }

        void release(uint8_t value) {
            _release_time = decay_times[value] * _tadj;
        }

        void gate(bool gb);

        float amplitude() const {
            return _A;
        }

        float tick();

    private:
        float _tadj;              /* Timing adjustment factor: 1MHz / system_clock_freq */

        float _attack_time{attack_times[0]};
        float _attack_slope{};
        float _decay_time{decay_times[0]};
        float _sustain{};
        float _release_time{decay_times[0]};
        float _release_A{};

        float _t{};
        float _A{};

        bool  _gate{};
        Cycle _cycle{CYCLE_NONE};

        static const std::array<float, 16> attack_times;
        static const std::array<float, 16> decay_times;
    };


    class Voice {
    public:
        Voice(unsigned clkf, Voice &svoice)
            : _osc{clkf, svoice._osc},
              _env{clkf} {
        }

        ~Voice() {
        }

        void freq_hi(uint8_t fh) {
            _osc.freq_hi(fh);
        }

        void freq_lo(uint8_t fl) {
            _osc.freq_lo(fl);
        }

        void width_hi(uint8_t pwh) {
            _osc.width_hi(pwh);
        }

        void width_lo(uint8_t pwl) {
            _osc.width_lo(pwl);
        }

        void attack(uint8_t value) {
            _env.attack(value);
        }

        void decay(uint8_t value) {
            _env.decay(value);
        }

        void sustain(uint8_t value) {
            _env.sustain(value);
        }

        void release(uint8_t value) {
            _env.release(value);
        }

        void control(uint8_t value);

        float tick() {
            return _osc.tick() * _env.tick() * 0.50f;
        }

    private:
        Oscillator _osc;
        Envelope   _env;

        friend class Mos6581;
    };


    class Filter {
    public:
        Filter()
            : _klo_data(SAMPLES, 0.0f),
              _khi_data(SAMPLES, 0.0f),
              _kbd_data(SAMPLES, 0.0f) {
        }

        ~Filter() {
        }

        void freq_hi(uint8_t hi) {
            _ufc = (_ufc & 7) | (static_cast<uint16_t>(hi) << 3);
            _generated = false;
        }

        void freq_lo(uint8_t lo) {
            _ufc = (_ufc & 0xFFF8) | (lo & 7);
            _generated = false;
        }

        void resonance(uint8_t rs) {
            _resonance = rs & 15;
            _generated = false;
        }

        void lopass(bool active) {
            _lopass = active;
            _generated = false;
        }

        void hipass(bool active) {
            _hipass = active;
            _generated = false;
        }

        void bandpass(bool active) {
            _bandpass = active;
            _generated = false;
        }

        bool lopass() const {
            return _lopass;
        }

        bool hipass() const {
            return _hipass;
        }

        bool bandpass() const {
            return _bandpass;
        }

        void apply(samples_fp &v);

    private:
        void generate();

        uint16_t         _ufc{};
        uint8_t          _resonance{};
        bool             _lopass{};
        bool             _hipass{};
        bool             _bandpass{};
        bool             _generated{};

        samples_fp       _klo_data;
        samples_fp       _khi_data;
        samples_fp       _kbd_data;

        gsl::span<float> _klo{};
        gsl::span<float> _khi{};
        gsl::span<float> _kbd{};
    };


    /**
     * Initalise this SID instance.
     * @param label Label assigned to this instance;
     * @param clkf  System clock frequency.
     */
    Mos6581(const std::string &label, unsigned clkf);

    virtual ~Mos6581() {
    }

    /**
     * @see Mos6581I::read()
     */
    uint8_t read(addr_t addr) const override;

    /**
     * @see Mos6581I::write()
     */
    void write(addr_t addr, uint8_t value) override;

private:
    /**
     * @see Mos6581I::play()
     */
    void play() override;

    bool is_v1_filtered() const {
        return _voice_1_filtered;
    }

    bool is_v2_filtered() const {
        return _voice_2_filtered;
    }

    bool is_v3_filtered() const {
        return _voice_3_filtered;
    }

    bool is_v3_active() const {
        return (_voice_3_filtered || !_voice_3_off);
    }

    Voice      _voice_1;
    Voice      _voice_2;
    Voice      _voice_3;

    samples_fp _v1;
    samples_fp _v2;
    samples_fp _v3;

    bool       _voice_1_filtered{};
    bool       _voice_2_filtered{};
    bool       _voice_3_filtered{};
    bool       _voice_3_off{};

    Filter     _filter{};

    float      _volume{};

    uint8_t    _last_value{};
};

}
