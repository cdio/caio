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

#include "logger.hpp"

namespace caio {
namespace ricoh {
namespace rp2a03 {

RP2A03::RP2A03(size_t clkf, const sptr_t<ASpace>& mmap)
    : RP2A03{LABEL, clkf, mmap}
{
}

RP2A03::RP2A03(std::string_view label, size_t clkf, const sptr_t<ASpace>& mmap)
    : Mos6502{TYPE, label, mmap},
      _apu{*this, clkf / CPU_DIVIDER}
{
    decimal_enable(false);
}

RP2A03::~RP2A03()
{
}

void RP2A03::audio_buffer(const AudioBufferCb& abuf)
{
    _apu.audio_buffer(abuf);
}

void RP2A03::add_ior(const IorCb& ior, uint8_t mask)
{
    _ioport.add_ior(ior, mask);
}

void RP2A03::add_iow(const IowCb& iow, uint8_t mask)
{
    _ioport.add_iow(iow, mask);
}

void RP2A03::bpadd(addr_t addr, const BreakpointCb& cb, void* arg)
{
    Mos6502::bpadd(addr, *reinterpret_cast<const Mos6502::BreakpointCb*>(&cb), arg);
}

void RP2A03::reset()
{
    Mos6502::reset();
    decimal_enable(false);
    _regs.S -= 3;

    _oamdma_addr = 0;
    _oamdma_size = 0;
    _oamdma_data = 0;
    _oamdma_loaded = 0;

    _apu.reset();
    rdy_pin(0);

    write(SND_CHN, 0);
    write(PORT2_FRAMECNT, 0);   /* Enable frame counter IRQ */
}

inline bool RP2A03::oamdma_is_running() const
{
    return (_oamdma_size != 0);
}

inline void RP2A03::oamdma_start(addr_t addr)
{
    _oamdma_addr = addr;
    _oamdma_size = 256;
    _oamdma_loaded = false;
}

bool RP2A03::oamdma_transfer(bool put_cycle)
{
    if (put_cycle) {
        if (_oamdma_loaded) {
            /*
             * Write data previously read during the GET cycle.
             */
            _mmap->write(OAMDATA_ADDR, _oamdma_data);
            _oamdma_loaded = false;
            --_oamdma_size;
        }
    } else if (_oamdma_size) {
        /*
         * GET cycle.
         */
        _oamdma_data = _mmap->read(_oamdma_addr);
        _oamdma_loaded = true;
        ++_oamdma_addr;
    }

    return oamdma_is_running();
}

bool RP2A03::dmcdma_transfer(bool put_cycle)
{
    if (put_cycle) {
        return true;
    }

    /*
     * GET cycle: Read data.
     */
    _apu.dmc_transfer();
    return false;
}

void RP2A03::dma_transfer(bool put_cycle)
{
    /*
     * See https://www.nesdev.org/wiki/DMA
     */
    bool dmc_active = _apu.dmc().need_data();
    bool oam_active = oamdma_is_running();
    bool active = dmc_active || oam_active;

    if (active && rdy_pin()) {
        /*
         * CPU halted on GET cycle.
         */
        if (!put_cycle) {
            /*
             * Halt costs one cycle.
             */
            rdy_pin(false);
        } else {
            /*
             * Alignment cycle.
             */
            ;
        }

        return;
    }

    if (dmc_active) {
        dmc_active = dmcdma_transfer(put_cycle);
    }

    if (oam_active) {
        oam_active = oamdma_transfer(put_cycle);
    }

    active = dmc_active || oam_active;
    if (!active) {
        /*
         * DMA ended, unhalt the CPU.
         */
        rdy_pin(true);
    }
}

size_t RP2A03::tick(const Clock& clk)
{
    _even_tick ^= 1;
    dma_transfer(_even_tick);

    _apu.tick();

    if (_cpu_cycles == 0) {
        _cpu_cycles = Mos6502::tick(clk);
        if (_cpu_cycles == HALT) {
            return HALT;
        }
    }

    --_cpu_cycles;
    return CPU_DIVIDER;
}

uint8_t RP2A03::read(addr_t addr, Device::ReadMode mode)
{
    if (addr >= REG_ADDR_START && addr < REG_ADDR_END) {
        uint8_t data{};

        switch (addr) {
        case SND_CHN:
            /*
             * Sound channel status:
             *   D7 D6 D5 D4 D3 D2 D1 D0
             *    |  |  |  |  |  |  |  |
             *    |  |  |  |  |  |  |  +-> Pulse-1 status (0: Halted/disabled, 1: Counting)
             *    |  |  |  |  |  |  +----> Pulse-2 status (0: Halted/disabled, 1: Counting)
             *    |  |  |  |  |  +-------> Triangle status (Status of linear counter irrelevant)
             *    |  |  |  |  +----------> Noise status (0: Halted/disabled, 1: Counting)
             *    |  |  |  +-------------> DMC status (0: No samples, 1: There are samples)
             *    |  |  +----------------> Open bus
             *    |  +-------------------> Frame interrupt Flag
             *    +----------------------> DMC interrupt Flag
             *
             * - Frame interrupt flag cleared after read.
             * - If an interrupt flag is set during read, it will be read as 1 but it is not cleared.
             */
            data = (_apu.pulse1().lc().is_running()     ? D0 : 0) |
                   (_apu.pulse2().lc().is_running()     ? D1 : 0) |
                   (_apu.triangle().lc().is_running()   ? D2 : 0) |
                   (_apu.noise().lc().is_running()      ? D3 : 0) |
                   (_apu.dmc().is_running()             ? D4 : 0) |
                   (_apu.frame_irq_flag()               ? D6 : 0) |
                   (_apu.dmc_irq_flag()                 ? D7 : 0) |
                   (_mmap->data_bus() & D5);
            _apu.frame_irq_ack();
            return data;

        case PORT1:
            /*
             * Read from Input Port 1 (/OE1 active during read).
             */
            return _ioport.ior(IOPORT_IN1);

        case PORT2_FRAMECNT:
            /*
             * Read from Input Port 2 (/OE2 active during read).
             */
            return _ioport.ior(IOPORT_IN2);

        default:
            return _mmap->data_bus();
        }
    }

    return Mos6502::read(addr, mode);
}

void RP2A03::write(addr_t addr, uint8_t value)
{
    if (addr >= REG_ADDR_START && addr < REG_ADDR_END) {
        auto& pulse1 = _apu.pulse1();
        auto& pulse2 = _apu.pulse2();
        auto& triangle = _apu.triangle();
        auto& noise = _apu.noise();
        auto& dmc = _apu.dmc();

        switch (addr) {
        case SQ1_VOL:
            /*
             * Pulse-1 Volume, loop flag, and duty cycle:
             *   D7 D6 D5 D4 D3 D2 D1 D0
             *    |  |  |  |  |  |  |  |
             *    |  |  |  |  +--+--+--+-> Volume/Envelope
             *    |  |  |  +-------------> Constant volume flag
             *    |  |  +----------------> Length counter halt flag / Envelope loop flag
             *    +--+-------------------> Duty cycle (00: 12.5%, 01: 25%, 10: 50%, 11: -25%/75%)
             *
             * Side effects:
             * - The duty cycle sequencer is not changed.
             */
            pulse1.envelope().reset(value & (D3 | D2 | D1 | D0), value & D4, value & D5);
            pulse1.lc().halt(value & D5);
            pulse1.duty_cycle(value >> 6);
            break;

        case SQ1_SWEEP:
            /*
             * Pulse-1 Sweep configuration:
             *   D7 D6 D5 D4 D3 D2 D1 D0
             *    |  |  |  |  |  |  |  |
             *    |  |  |  |  |  +--+--+-> Shift count
             *    |  |  |  |  +----------> Negate flag (0: Add to period, 1: Subtract from period)
             *    |  +--+--+-------------> Divider Period
             *    +----------------------> Sweep enable flag (0: Disable, 1: Enable)
             *
             * Side effects:
             * - Sets the reload flag
             */
            pulse1.sweep().reset(value & D7, (value >> 4) & 7, value & D3, value & 7);
            break;

        case SQ1_LO:
            /*
             * Pulse-1 timer configuration (Lo value):
             *   D7 D6 D5 D4 D3 D2 D1 D0
             *    |  |  |  |  |  |  |  |
             *    +--+--+--+--+--+--+--+-> Timer lo value
             */
            pulse1.timer().period_lo(value);
            break;

        case SQ1_HI:
            /*
             * Pulse-1 Length counter and timer configuration (Hi value):
             *   D7 D6 D5 D4 D3 D2 D1 D0
             *    |  |  |  |  |  |  |  |
             *    |  |  |  |  |  +--+--+-> Timer hi value
             *    +--+--+--+--+----------> Length counter
             *
             * Side effects:
             * - The sequencer is restarted at the first value of the current sequence
             * - The envelope is restarted
             * - The period divider is not reset
             */
            pulse1.timer().period_hi(value & (D2 | D1 | D0));
            pulse1.lc().period(value >> 3);
            pulse1.envelope().reload();
            pulse1.duty_cycle_reset();
            break;

        case SQ2_VOL:
            /*
             * Pulse-2 Volume, loop flag, and duty cycle:
             *   D7 D6 D5 D4 D3 D2 D1 D0
             *    |  |  |  |  |  |  |  |
             *    |  |  |  |  +--+--+--+-> Volume/Envelope
             *    |  |  |  +-------------> Constant volume flag
             *    |  |  +----------------> Length counter halt flag / Envelope loop flag
             *    +--+-------------------> Duty cycle (00: 12.5%, 01: 25%, 10: 50%, 11: -25%/75%)
             *
             * Side effects:
             * - The duty cycle sequencer is not changed.
             */
            pulse2.envelope().reset(value & (D3 | D2 | D1 | D0), value & D4, value & D5);
            pulse2.lc().halt(value & D5);
            pulse2.duty_cycle(value >> 6);
            break;

        case SQ2_SWEEP:
            /*
             * Pulse-2 Sweep configuration:
             *   D7 D6 D5 D4 D3 D2 D1 D0
             *    |  |  |  |  |  |  |  |
             *    |  |  |  |  |  +--+--+-> Shift count
             *    |  |  |  |  +----------> Negate flag (0: Add to period, 1: Subtract from period)
             *    |  +--+--+-------------> Divider Period
             *    +----------------------> Sweep enable flag (0: Disable, 1: Enable)
             *
             * Side effects:
             * - Sets the reload flag
             */
            pulse2.sweep().reset(value & D7, (value >> 4) & 7, value & D3, value & 7);
            break;

        case SQ2_LO:
            /*
             * Pulse-2 timer configuration (Lo value):
             *   D7 D6 D5 D4 D3 D2 D1 D0
             *    |  |  |  |  |  |  |  |
             *    +--+--+--+--+--+--+--+-> Timer lo value
             */
            pulse2.timer().period_lo(value);
            break;

        case SQ2_HI:
            /*
             * Pulse-2 timer configuration (Hi value):
             *   D7 D6 D5 D4 D3 D2 D1 D0
             *    |  |  |  |  |  |  |  |
             *    |  |  |  |  |  +--+--+-> Timer hi value
             *    +--+--+--+--+----------> Length counter
             *
             * Side effects:
             * - The sequencer is immediately restarted at the first value of the current sequence
             * - The envelope is also restarted
             * - The period divider is not reset
             */
            pulse2.timer().period_hi(value & (D2 | D1 | D0));
            pulse2.lc().period(value >> 3);
            pulse2.envelope().reload();
            pulse1.duty_cycle_reset();
            break;

        case TRI_LINEAR:
            /*
             * Triangle linear counter setup:
             *   D7 D6 D5 D4 D3 D2 D1 D0
             *    |  |  |  |  |  |  |  |
             *    |  +--+--+--+--+--+--+-> Linear counter load value
             *    +----------------------> Length counter halt flag / Linear counter control
             */
            triangle.linearcnt_period(value & ~D7);
            triangle.lc().halt(value & D7);
            break;

        case TRI_LO:
            /*
             * Triangle timer low:
             *   D7 D6 D5 D4 D3 D2 D1 D0
             *    |  |  |  |  |  |  |  |
             *    +--+--+--+--+--+--+--+-> Timer lo value
             */
            triangle.timer().period_lo(value);
            break;

        case TRI_HI:
            /*
             * Triangle length counter and timer high:
             *   D7 D6 D5 D4 D3 D2 D1 D0
             *    |  |  |  |  |  |  |  |
             *    |  |  |  |  |  +--+--+-> Timer hi value
             *    +--+--+--+--+----------> Length counter load
             *
             * Side effects:
             * - Linear counter reload flag is set.
             */
            triangle.timer().period_hi(value & (D2 | D1 | D0));
            triangle.lc().period(value >> 3);
            triangle.linearcnt_reload();
            break;

        case NOISE_VOL:
            /*
             * Noise envelope and length counter halt flag:
             *   D7 D6 D5 D4 D3 D2 D1 D0
             *    |  |  |  |  |  |  |  |
             *    x  x  |  |  +--+--+--+-> Volume/envelope divider period
             *          |  +-------------> Constant volume/envelope flag
             *          +----------------> Length counter halt flag
             */
            noise.envelope().reset(value & (D3 | D2 | D1 | D0), value & D4, value & D5);
            noise.lc().halt(value & D5);
            break;

        case NOISE_MODE:
            /*
             * Noise mode and timer period:
             *   D7 D6 D5 D4 D3 D2 D1 D0
             *    |  |  |  |  |  |  |  |
             *    |  x  x  x  +--+--+--+-> Timer period
             *    +----------------------> Mode flag
             */
            noise.reset(value & D7, value & (D3 | D2 | D1 | D0));
            break;

        case NOISE_LC:
            /*
             * Noise length counter and envelope restart:
             *   D7 D6 D5 D4 D3 D2 D1 D0
             *    |  |  |  |  |  |  |  |
             *    |  |  |  |  |  x  x  x
             *    +--+--+--+--+----------> Length counter load and envelope restart
             */
            noise.lc().period(value >> 3);
            noise.envelope().reload();
            break;

        case DMC_FREQ:
            /*
             * DMC flags and rate:
             *   D7 D6 D5 D4 D3 D2 D1 D0
             *    |  |  |  |  |  |  |  |
             *    |  |  x  x  +--+--+--+-> Rate index
             *    |  +-------------------> Loop flag
             *    +----------------------> IRQ enable flag
             */
            dmc.irq_en(value & D7);
            dmc.sample_loop(value & D6);
            dmc.rate(value & (D3 | D2 | D1 | D0));
            break;

        case DMC_RAW:
            /*
             * DMC direct sample load:
             *   D7 D6 D5 D4 D3 D2 D1 D0
             *    |  |  |  |  |  |  |  |
             *    x  +--+--+--+--+--+--+-> Sample
             */
            dmc.sample_load(value & ~D7);
            break;

        case DMC_START:
            /*
             * DMC sample start address:
             *   D7 D6 D5 D4 D3 D2 D1 D0
             *    |  |  |  |  |  |  |  |
             *    +--+--+--+--+--+--+--+-> Sample address
             *
             * Actual address encoded as:
             *   11AA'AAAA'AA00'0000 = $C000 + A * 64
             */
            dmc.sample_start((A15 | A14) | (value << 6));
            break;

        case DMC_LEN:
            /*
             * DMC sample length:
             *   D7 D6 D5 D4 D3 D2 D1 D0
             *    |  |  |  |  |  |  |  |
             *    +--+--+--+--+--+--+--+-> Sample length
             *
             * Actual length encoded as:
             *   0000'LLLL'LLLL'0001 = L * 16 + 1
             */
            dmc.sample_length((value << 4) | 1);
            break;

        case OAMDMA:
            /*
             * Start OAM DMA transfer.
             */
            oamdma_start(value << 8);
            break;

        case SND_CHN:
            /*
             * Sound channels enable/disable flags:
             *   D7 D6 D5 D4 D3 D2 D1 D0
             *    |  |  |  |  |  |  |  |
             *    x  x  x  |  |  |  |  +-> Pulse-1: 0: Silence channel and halt length counter, 1: Enable (*)
             *             |  |  |  +----> Pulse-2: 0: Silence channel and halt length counter, 1: Enable (*)
             *             |  |  +-------> Triangle: 0: Silence channel and halt length counter, 1: Enable (*)
             *             |  +----------> Noise: 0: Silence channel and halt length counter, 1: Enable (*)
             *             +-------------> DMC: 0: Remaining bytes set to 0, silenced when it is empty
             *                                  1: Sample restarted (any remaining bytes are played)
             *
             * DMC interrupt flag cleared after write.
             */
            pulse1.lc().enable(value & D0);
            pulse2.lc().enable(value & D1);
            triangle.lc().enable(value & D2);
            noise.lc().enable(value & D3);
            dmc.enable(value & D4);
            _apu.dmc_irq_ack();
            break;

        case PORT1:
            /*
             * Output ports write (OUT-0..2).
             */
            _ioport.iow(IOPORT_OUT, value & IOPORT_OUT_MASK);
            break;

        case PORT2_FRAMECNT:
            /*
             * Frame Counter configuration:
             *   D7 D6 D5 D4 D3 D2 D1 D0
             *    |  |  |  |  |  |  |  |
             *    |  |  x  x  x  x  x  x
             *    |  +-------------------> IRQ Disable Flag: 0: Generate IRQ (4-step only), 1: Do not generate IRQ
             *    +----------------------> Mode: 0: 4-step sequence, 1: 5-step sequence
             *
             * The frame counter is reset after write.
             */
            _apu.frame_reset((value & D6) == 0,
                ((value & D7) == D7 ? Apu::FrameCounter::MODE_5_STEPS : Apu::FrameCounter::MODE_4_STEPS));
            break;

        default:;
        }
    } else {
        Mos6502::write(addr, value);
    }
}

}
}
}
