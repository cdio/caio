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
#include "ricoh_2a03.hpp"

namespace caio {
namespace ricoh {
namespace rp2a03 {

const uint8_t LengthCounter::lc_table[LC_TABLE_SIZE] = {
    10, 254, 20,  2, 40,  4, 80,  6, 160,  8, 60, 10, 14, 12, 26, 14,
    12,  16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
};

const uint8_t Pulse::duty_table[DUTY_TABLE_SIZE][DUTY_SEQUENCE_SIZE] = {
    { 0, 1, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 1, 0, 0, 0 },
    { 1, 0, 0, 1, 1, 1, 1, 1 }
};

const int16_t Triangle::tri_table[TRIANGLE_TABLE_SIZE] = {
    15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
};

const uint16_t Noise::noise_table[NOISE_TABLE_SIZE] = {
    4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
};

const uint16_t Dmc::rate_table[RATE_TABLE_SIZE] = {
    428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72, 54
};

bool Divider::tick()
{
    if (_counter == 0) {
        reset();
        return true;
    }

    --_counter;
    return false;
}

void LengthCounter::enable(bool en)
{
    _enabled = en;
    if (!_enabled) {
        /*
         * The associated channel is silenced.
         */
        _counter = 0;
    }
}

void LengthCounter::period(uint8_t index)
{
    if (_enabled) {
        _counter = lc_table[index & LC_MASK];
    }
}

void LengthCounter::tick()
{
    if (!_halt && _counter != 0) {
        --_counter;
    }
}

void Envelope::reset(uint8_t volume, bool constvol, bool loop)
{
    _divider.period(volume);
    _constvol = constvol;
    _loop = loop;
    _reload = true;
}

void Envelope::tick()
{
    if (_reload) {
        /*
         * Restart the envelope generator.
         */
        _divider.reset();
        _level = VOLUME_MAX;
        _reload = false;

    } else if (_divider.tick()) {
        /*
         * Decay tick: If the level reaches 0
         * it stays there unless the loop flag is set.
         */
        if (_level != 0) {
            --_level;
        } else if (_loop) {
            _level = VOLUME_MAX;
        }
    }
}

void Sweep::reset(bool enable, uint8_t period, bool negate, uint8_t shift)
{
    _enabled = enable;
    _divider.period(period);
    _negate = negate;
    _shift = shift;
    _reload = true;
    calculate_target();     /* Mute flag */
}

void Sweep::calculate_target()
{
    const int period = _pulse.timer().period();
    int change = period >> _shift;

    if (_negate) {
        change = -change - _onec;
    }

    const int target = std::max(0, period + change);
    _target = static_cast<uint16_t>(target);
    _muted = (period < 8) || (_target > 0x7FF);
}

void Sweep::tick()
{
    calculate_target();

    if (_divider.tick() && _enabled && !_muted) {
        _pulse.timer().period(_target);
    }

    if (_reload) {
        _divider.reset();
        _reload = false;
    }
}

void Pulse::tick()
{
    if (_timer.tick()) {
        _seq = (_seq + 1) & DUTY_SEQUENCE_MASK;
        _out = duty_table[_D][_seq] * _env.output() * (!_sweep.is_muted()) * (_lc.counter() != 0);
    }
}

void Triangle::linearcnt_tick()
{
    if (_linear_reload) {
        _linear_cnt.reset();
        _linear_reload = _lc.is_halt();
    } else if (_linear_cnt.counter() != 0) {
        _linear_cnt.tick();
    }
}

void Triangle::tick()
{
    if (_timer.tick()) {
        if (_lc.counter() != 0 && _linear_cnt.counter() != 0) {
            _seq = (_seq + 1) & TRIANGLE_TABLE_MASK;

            /* Silence high frequencies */
            if (_timer.period() > 1) {
                _out = tri_table[_seq];
            }
        }
    }
}

void Noise::reset(bool mode, uint8_t value)
{
    _mode = mode;
    _shreg = 1;

    /* Table is in CPU cycles, we are ticked at APU speed (CPU_clk / 2) */
    _timer.period(noise_table[value & NOISE_TABLE_MASK] >> 1);
}

void Noise::tick()
{
    if (_timer.tick()) {
        const bool bit0 = _shreg & D0;
        const bool bitt = _shreg & (_mode ? D6 : D1);
        const bool feedback = bit0 ^ bitt;
        _shreg = (_shreg >> 1) | (feedback << 14);
        _out = _env.output() * !(_shreg & D0) * (_lc.counter() != 0);
    }
}

bool DmcReader::push(uint8_t data)
{
    _data = data;
    _consumed = false;
    _addr = (_addr == 0xFFFF ? 0x8000 : _addr + 1);
    --_remaining;
    if (_remaining == 0) {
        if (!_loop) {
            /* No more data to read */
            return true;
        }
        reset();
    }
    return false;
}

uint8_t DmcReader::pop()
{
    _consumed = true;
    const uint8_t d = _data;
    _data = 0;
    return d;
}

void DmcReader::reset()
{
    _addr = _start;
    _remaining = _len;
}

bool DmcShifter::tick()
{
    const bool value = _sample & _bit;
    _bit <<= 1;
    return value;
}

void DmcShifter::reset(uint8_t sample)
{
    _sample = sample;
    _bit = 1;
}

void Dmc::reset()
{
    _enabled = false;
    _reader = {};
    _shifter = {};
    _silence = true;
    _out = 0;
}

void Dmc::tick()
{
    if (_timer.tick()) {
        if (_shifter.empty()) {
            _silence = _reader.empty();
            if (!_silence) {
                _shifter.reset(_reader.pop());
            }
        }

        if (_silence) {
            _out = 0;
        } else {
            const auto value = (_shifter.tick() ? 2 : -2);
            if ((value == 2 && _out < 126) || (value == -2 && _out > 1)) {
                _out += value;
            }
        }
    }
}

Apu::Apu(RP2A03& cpu, size_t cpu_clkf)
    : _cpu{cpu},
      _srate_cycles{Clock::cycles(AUDIO_DT, cpu_clkf)}
{
}

void Apu::reset()
{
    frame_irq_ack();
    dmc_irq_ack();
    _dmc.reset();
    _apos = 0;
}

void Apu::audio_buffer(const AudioBufferCb& abuf)
{
    _audio_buffer = abuf;
}

bool Apu::frame_irq_flag()
{
    return _framecnt.irq_flag;
}

void Apu::frame_irq_ack()
{
    if (_framecnt.irq_flag) {
        _framecnt.irq_flag = false;
        _cpu.irq_pin(false);
    }
}

bool Apu::dmc_irq_flag()
{
    return _dmc.irq_flag();
}

void Apu::dmc_irq_ack()
{
    if (_dmc.irq_flag()) {
        _dmc.irq_flag(false);
        _cpu.irq_pin(false);
    }
}

void Apu::dmc_transfer()
{
    auto& reader = _dmc.reader();
    const uint8_t data = _cpu.read(reader.addr());
    const bool transfer_ended = reader.push(data);
    if (transfer_ended && _dmc.irq_en() && !_dmc.irq_flag()) {
        _dmc.irq_flag(true);
        _cpu.irq_pin(true);
    }
}

void Apu::frame_reset(bool irq_en, FrameCounter::Mode mode)
{
    if (!irq_en) {
        /*
         * IRQ cleared if the IRQ enable flag is disabled.
         */
        frame_irq_ack();
    }

    if (mode == FrameCounter::MODE_4_STEPS) {
        _framecnt.irq_en = irq_en;

    } else {
        frame_quarter_tick();
        frame_half_tick();
    }

    /* Reset */
    _framecnt.mode = mode;
    _framecnt.cycle = 0;
    _framecnt.step = 0;
}

void Apu::frame_quarter_tick()
{
    _pulse1.envelope().tick();
    _pulse2.envelope().tick();
    _triangle.linearcnt_tick();
    _noise.envelope().tick();
}

void Apu::frame_half_tick()
{
    _pulse1.lc().tick();
    _pulse1.sweep().tick();
    _pulse2.lc().tick();
    _pulse2.sweep().tick();
    _triangle.lc().tick();
    _noise.lc().tick();
}

void Apu::tick()
{
    _even_tick ^= 1;

    if (_even_tick) {
        ++_framecnt.cycle;
        if (_framecnt.cycle == FRAMECNT_DIVIDER) {
            _framecnt.cycle = 0;
            ++_framecnt.step;

            if (_framecnt.step != 4 || _framecnt.step == _framecnt.mode) {
                /*
                 * Quarter frame: Tick envelopes and linear counter.
                 */
                frame_quarter_tick();
            }

            if (_framecnt.step == 2 || _framecnt.step == _framecnt.mode) {
                /*
                 * Half frame: Tick length counters and sweep units.
                 */
                frame_half_tick();
            }

            if (_framecnt.step == _framecnt.mode) {
                _framecnt.step = 0;
                if (_framecnt.mode == FrameCounter::MODE_4_STEPS && _framecnt.irq_en && !_framecnt.irq_flag) {
                    _framecnt.irq_flag = true;
                    _cpu.irq_pin(true);
                }
            }
        }

        /*
         * Pulse, noise, and DMC ticked at APU speed.
         */
        _pulse1.tick();
        _pulse2.tick();
        _noise.tick();
        _dmc.tick();
    }

    /*
     * Triangle ticked at CPU speed.
     */
    _triangle.tick();

    ++_sample_cycle;
    if (_sample_cycle == _srate_cycles) {
        /*
         * Sample audio and play it.
         */
        _sample_cycle = 0;
        play();
    }
}

int16_t Apu::mixed_sample() const
{
    /*
     * See https://www.nesdev.org/wiki/APU_Mixer
     */
    const float p1 = _pulse1.output();
    const float p2 = _pulse2.output();
    const float tri = _triangle.output() / 8227.0f;
    const float noise = _noise.output() / 12241.0f;
    const float dmc = _dmc.output() / 22638.0f;

    float p12 = p1 + p2;
    float tnd = tri + noise + dmc;

    p12 = (p12 == 0.0f ? 0.0f : 95.88f / ((8128.0f / p12) + 100.0f));
    tnd = (tnd == 0.0f ? 0.0f : 159.79f / ((1.0f / tnd) + 100.0f));

    const float fsample = _filter(p12 + tnd);
    const int16_t sample = utils::to_i16(fsample - 0.5f);
    return sample;
}

void Apu::play()
{
    _abuf[_apos++] = mixed_sample();
    if (_apos == std::size(_abuf)) {
        _apos = 0;
        if (_audio_buffer) {
            if (auto buf = _audio_buffer(); buf) {
                std::copy(std::begin(_abuf), std::end(_abuf), std::begin(buf));
            }
        }
    }
}

}
}
}
