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

#include "gpio.hpp"
#include "mos_6502.hpp"
#include "ui.hpp"
#include "utils.hpp"

namespace caio {
namespace ricoh {
namespace rp2a03 {

constexpr static const size_t AUDIO_CHANNELS        = 1;
constexpr static const unsigned AUDIO_SAMPLING_RATE = 44100;
constexpr static const float AUDIO_DT               = 1.0f / AUDIO_SAMPLING_RATE;
constexpr static const float AUDIO_SAMPLES_TIME     = 0.020f;
constexpr static const size_t AUDIO_SAMPLES         = static_cast<size_t>(
                                                        utils::ceil(AUDIO_SAMPLING_RATE * AUDIO_SAMPLES_TIME));

/**
 * Divider (11-bit cyclic counter).
 */
class Divider {
public:
    constexpr static const uint16_t PERIOD_MASK = A11 - 1;

    void period_lo(uint8_t value)
    {
        _period = (_period & 0xFF00) | value;
    }

    void period_hi(uint8_t value)
    {
        _period = ((_period & 0x00FF) | (value << 8)) & PERIOD_MASK;
    }

    void period(uint16_t period)
    {
        _period = period & PERIOD_MASK;
    }

    uint16_t period() const
    {
        return _period;
    }

    uint16_t counter() const
    {
        return _counter;
    }

    void reset()
    {
        _counter = _period;
    }

    /**
     * Timer tick.
     * When the counter reaches 0 it reloads the period value.
     * @return True if the counter reached 0; false otherwise.
     * @see reset()
     */
    bool tick();

protected:
    uint16_t _period{};
    uint16_t _counter{};
};

/**
 * Length counter (8-bit counter).
 * @see https://www.nesdev.org/wiki/APU#Length_Counter
 */
class LengthCounter {
public:
    constexpr static const size_t LC_TABLE_SIZE = 32;
    constexpr static const uint8_t LC_MASK      = 31;

    void enable(bool en);

    void period(uint8_t index);

    void halt(bool hlt)
    {
        _halt = hlt;
    }

    bool is_halt() const
    {
        return _halt;
    }

    bool is_running() const
    {
        return (_enabled && _counter != 0);
    }

    uint8_t counter() const
    {
        return _counter;
    }

    void tick();

private:
    bool    _enabled{};
    bool    _halt{};
    uint8_t _counter{};

    static const uint8_t lc_table[LC_TABLE_SIZE];
};

/**
 * Envelope generator.
 * The envelope controls the volume level of a channel with
 * a constant value or with a decaying sawtooth like waveform.
 * @see https://www.nesdev.org/wiki/APU_Envelope
 */
class Envelope {
public:
    constexpr static const uint8_t VOLUME_MAX = 15;

    void reset(uint8_t volume, bool constvol, bool loop);

    void reload()
    {
        _reload = true;
    }

    int16_t output() const
    {
        return (_constvol ? _divider.period() : _level);
    }

    void tick();

private:
    bool    _reload{true};
    bool    _constvol{};
    Divider _divider{};
    bool    _loop{};
    uint8_t _level{};
};

/**
 * Base class for waveform generators.
 * Waveform generators have a timer, a length counter, and an envelope.
 */
class Waveform {
public:
    Divider& timer()
    {
        return _timer;
    }

    LengthCounter& lc()
    {
        return _lc;
    }

    Envelope& envelope()
    {
        return _env;
    }

protected:
    Divider         _timer{};
    LengthCounter   _lc{};
    Envelope        _env{};
};

/**
 * Pulse sweep unit.
 * @see https://www.nesdev.org/wiki/APU_Sweep
 */
class Sweep {
public:
    Sweep(class Pulse& pulse, bool onec)
        : _pulse{pulse},
          _onec{onec}
    {
    }

    void reset(bool enable, uint8_t period, bool negate, uint8_t shift);

    bool is_muted() const
    {
        return _muted;
    }

    void tick();

private:
    void calculate_target();

    class Pulse&    _pulse;
    bool            _onec;
    bool            _enabled{};
    bool            _negate{};
    uint8_t         _shift{};
    Divider         _divider{};
    bool            _reload{};
    bool            _muted{};
    uint16_t        _target{};
};

/**
 * Pulse waveform generator.
 * @see https://www.nesdev.org/wiki/APU_Pulse
 */
class Pulse : public Waveform {
public:
    constexpr static const size_t DUTY_TABLE_SIZE    = 4;
    constexpr static const size_t DUTY_TABLE_MASK    = 3;
    constexpr static const size_t DUTY_SEQUENCE_SIZE = 8;
    constexpr static const size_t DUTY_SEQUENCE_MASK = 7;

    Pulse(bool onec)
        : Waveform{},
          _sweep{*this, onec}
    {
    }

    Sweep& sweep()
    {
        return _sweep;
    }

    void duty_cycle(uint8_t D)
    {
        _D = D & DUTY_TABLE_MASK;
    }

    void duty_cycle_reset()
    {
        _seq = 0;
    }

    int16_t output() const
    {
        return _out;
    }

    void tick();

private:
    Sweep   _sweep;
    size_t  _D{};
    size_t  _seq{};
    int16_t _out{};

    static const uint8_t duty_table[DUTY_TABLE_SIZE][DUTY_SEQUENCE_SIZE];
};

/**
 * Triangle waveform generator.
 * @see https://www.nesdev.org/wiki/APU_Triangle
 */
class Triangle : public Waveform {
public:
    constexpr static const uint8_t TRIANGLE_TABLE_SIZE = 32;
    constexpr static const uint8_t TRIANGLE_TABLE_MASK = 31;

    void linearcnt_period(uint8_t value)
    {
        _linear_cnt.period(value);
    }

    void linearcnt_reload()
    {
        _linear_reload = true;
    }

    int16_t output() const
    {
        return _out;
    }

    void linearcnt_tick();

    void tick();

private:
    bool    _linear_reload{};
    Divider _linear_cnt{};
    size_t  _seq{};
    int16_t _out{};

    static const int16_t tri_table[TRIANGLE_TABLE_SIZE];
};

/**
 * Noise waveform generator.
 * @see https://www.nesdev.org/wiki/APU_Noise
 */
class Noise : public Waveform {
public:
    constexpr static const uint8_t NOISE_TABLE_SIZE = 16;
    constexpr static const uint8_t NOISE_TABLE_MASK = 15;

    void reset(bool mode, uint8_t period);

    int16_t output() const
    {
        return _out;
    }

    void tick();

private:
    bool        _mode{};
    uint16_t    _shreg{1};
    int16_t     _out{};

    static const uint16_t noise_table[NOISE_TABLE_SIZE];
};

/**
 * DMC Reader.
 * @see Dmc
 */
class DmcReader {
public:
    bool need_data() const
    {
        return (_consumed && _remaining);
    }

    bool push(uint8_t data);

    uint8_t pop();

    void loop(bool active)
    {
        _loop = active;
    }

    void start(addr_t start)
    {
        _start = start;
        reset();
    }

    void length(addr_t len)
    {
        _len = len;
    }

    addr_t addr() const
    {
        return _addr;
    }

    addr_t remaining() const
    {
        return _remaining;
    }

    bool empty() const
    {
        return _consumed;
    }

    void stop();

private:
    void reset();

    bool    _loop{};
    addr_t  _start{};
    addr_t  _len{};
    addr_t  _addr{};
    addr_t  _remaining{};
    uint8_t _data{};
    bool    _consumed{};
};

/**
 * DMC Shifter.
 * @see Dmc
 */
class DmcShifter {
public:
    bool tick();

    bool empty() const
    {
        return (_bit == 0);
    }

    void reset(uint8_t sample);

private:
    uint8_t _sample{};
    uint8_t _bit{};
};

/*
 * Delta Modulation Channel (DMC).
 * @see https://www.nesdev.org/wiki/APU_DMC
 */
class Dmc {
public:
    constexpr static const size_t RATE_TABLE_SIZE = 16;
    constexpr static const size_t RATE_TABLE_MASK = 15;

    void reset();

    void enable(bool en)
    {
        _enabled = en;
    }

    bool is_enabled() const
    {
        return _enabled;
    }

    void irq_en(bool en)
    {
        _irq_en = en;
    }

    void sample_loop(bool loop)
    {
        _reader.loop(loop);
    }

    void rate(uint8_t rindex)
    {
        _timer.period(rate_table[rindex & RATE_TABLE_MASK] >> 1);   /* Rate table in CPU cycles */
    }

    void sample_load(uint8_t sample)
    {
        _out = sample;
    }

    void sample_start(addr_t addr)
    {
        _reader.start(addr);
    }

    void sample_length(addr_t len)
    {
        _reader.length(len);
    }

    bool need_data() const
    {
        return (is_enabled() && _reader.need_data());
    }

    bool is_running() const
    {
        return (is_enabled() && _reader.remaining() != 0);
    }

    bool irq_en() const
    {
        return _irq_en;
    }

    void irq_flag(bool active)
    {
        _irq_flag = active;
    }

    bool irq_flag() const
    {
        return _irq_flag;
    }

    DmcReader& reader()
    {
        return _reader;
    }

    int16_t output() const
    {
        return _out;
    }

    void tick();

private:
    bool        _enabled{};
    bool        _irq_en{};
    bool        _irq_flag{};
    DmcReader   _reader{};
    DmcShifter  _shifter{};
    Divider     _timer{};
    bool        _silence{};
    int16_t     _out{};

    static const uint16_t rate_table[RATE_TABLE_SIZE];
};

/**
 * Ricoh 2A03 Audio Processing Unit (APU).
 * @see https://www.nesdev.org/wiki/APU
 */
class Apu {
public:
    constexpr static const unsigned FRAMECNT_DIVIDER = 3728;

    using AudioBufferCb = std::function<ui::AudioBuffer()>;

    struct FrameCounter {
        enum Mode {
            MODE_4_STEPS = 4,
            MODE_5_STEPS = 5
        };

        bool        irq_en{};       /* Trigger IRQ on 4-step mode overflow  */
        bool        irq_flag{};     /* IRQ triggered                        */
        Mode        mode{};         /* Counter mode                         */
        unsigned    cycle{};        /* APU cycle counter                    */
        uint8_t     step{};         /* Current step                         */
    };

    /**
     * APU Initialisation.
     * @param cpu The associated CPU;
     * @param clk CPU clock frequency.
     */
    Apu(class RP2A03& cpu, size_t cpu_clkf);

    /**
     * reset this APU.
     */
    void reset();

    /**
     * Set the audio buffer provider.
     * @param abuf Audio buffer provider.
     * @see ui::AudioBuffer
     */
    void audio_buffer(const AudioBufferCb& abuf);

    /**
     * Get the status of the Frame interrupt flag.
     * @return The frame interrupt flag.
     */
    bool frame_irq_flag();

    /**
     * Get the status of the DMC interrupt flag.
     * @return The frame interrupt flag.
     */
    bool dmc_irq_flag();

    /**
     * Acknowledge a Frame interrupt.
     */
    void frame_irq_ack();

    /**
     * Acknowledge a DMC interrupt.
     */
    void dmc_irq_ack();

    /**
     * Set the Frame Counter parameters.
     * @param irq_en True to enable IRQ on FrameCounter:Mode::MODE_4_STEPS; false otherwise;
     * @param mode   Runing mode.
     * @see FrameCounter::Mode
     */
    void frame_reset(bool irq_en, FrameCounter::Mode mode);

    /**
     * Quarter frame cycle tick.
     */
    void frame_quarter_tick();

    /**
     * Half frame cycle tick.
     */
    void frame_half_tick();

    Pulse& pulse1()
    {
        return _pulse1;
    }

    Pulse& pulse2()
    {
        return _pulse2;
    }

    Triangle& triangle()
    {
        return _triangle;
    }

    Noise& noise()
    {
        return _noise;
    }

    Dmc& dmc()
    {
        return _dmc;
    }

    void dmc_transfer();

    /**
     * APU clock tick.
     * This method must be called at CPU frequency.
     */
    void tick();

private:
    int16_t mixed_sample() const;

    void play();

    class RP2A03&   _cpu;
    bool            _even_tick{};               /* CPU frequency divider    */
    FrameCounter    _framecnt{};                /* Frame counter            */
    Pulse           _pulse1{true};              /* Pulse-1 wave generator   */
    Pulse           _pulse2{false};             /* Pulse-2 wave generator   */
    Triangle        _triangle{};                /* Triangle wave generator  */
    Noise           _noise{};                   /* Noise wave generator     */
    Dmc             _dmc{};                     /* DMC wave generator       */
    AudioBufferCb   _audio_buffer{};            /* Audio buffer provider    */
    int16_t         _abuf[AUDIO_SAMPLES];       /* Samples buffer           */
    size_t          _apos{};                    /* Samples buffer position  */
    size_t          _srate_cycles;              /* Sampling rate in cycles  */
    size_t          _sample_cycle{};            /* Sample cycle counter     */
};

/**
 * Ricoh 2A03 SoC.
 * The Ricoh 2A03C embeds:
 * - A clone of the MOS-6502 microprocessor (without BCD capabilities)
 * - An audio processing unit (APU)
 * - A memory mapped set of input and output ports
 *
 * ### Pinout:
 *
 *                +------------+
 *      AD1 01  <-| *          |<-  40 VCC
 *      AD2 02  <-|            |->  39 OUT0
 *      RST 03  ->|            |->  38 OUT1
 *      A00 04  <-|            |->  37 OUT2
 *      A01 05  <-|            |->  36 /OE1
 *      A02 06  <-|            |->  35 /OE2
 *      A03 07  <-|            |->  34 R/W
 *      A04 08  <-|            |<-  33 /NMI
 *      A05 09  <-|            |<-  32 /IRQ
 *      A06 10  <-|    2A03    |->  31 M2
 *      A07 11  <-|            |<-  30 TST
 *      A08 12  <-|            |<-  29 CLK
 *      A09 13  <-|            |<>  28 D0
 *      A10 14  <-|            |<>  27 D1
 *      A11 15  <-|            |<>  26 D2
 *      A12 16  <-|            |<>  25 D3
 *      A13 17  <-|            |<>  24 D4
 *      A14 18  <-|            |<>  23 D5
 *      A15 19  <-|            |<>  22 D6
 *      GND 20  ->|            |<>  21 D7
 *                +------------+
 *
 * - CLK:
 *   21.48 MHz input clock. Internally, this clock is divided by 12.
 *   The actual SoC clock is then 1.79 MHz
 *
 *   *In this implementation the input clock is divided by 3 instead of 12.
 *    It must be clocked at three times the desired frequency.*
 *
 * - AD1:
 *   Analog audio output pin (mixed pulse1 and pulse2 waveforms).
 *
 * - AD2:
 *   Analog audio output pin (mixed triangle, noise and DPCM waveforms).
 *
 * - OUT0-OUT2:
 *   Output ports. These ports are memory mapped at address $4016,
 *   bits 0-2 respectively.
 *
 * - /OE1:
 *   Activated when reading from a 5-bit input port memory mapped at address $4016.
 *
 * - /OE2:
 *   Activated when reading from a 5-bit input port memory mapped at address $4017.
 *
 * ### Memory Map:
 *
 *    Address Range     Size    Description         Provider Device
 *    --------------------------------------------------------------------------------
 *    0000-3FFF         4000    Generic area        Address space mappings
 *    4000-4017         0018    APU and I/O         Internal
 *    4018-401F         0008    Test mode           Internal (active through TST pin)
 *    4020-FFFF         BFE0    Generic area        Address space mappings
 *
 * @see Mos6502
 * @see https://www.nesdev.org/wiki/CPU
 */
class RP2A03 : public mos::Mos6502 {
public:
    constexpr static const char* TYPE               = "RP2A03";
    constexpr static const size_t CPU_DIVIDER       = 3;
    constexpr static const addr_t REG_ADDR_START    = 0x4000;
    constexpr static const addr_t REG_ADDR_END      = 0x4020;
    constexpr static const addr_t OAMDATA_ADDR      = 0x2004;
    constexpr static const uint8_t IOPORT_OUT       = 0x00;             /* OUT0-2 */
    constexpr static const uint8_t IOPORT_IN1       = 0x00;             /* /OE1   */
    constexpr static const uint8_t IOPORT_IN2       = 0x01;             /* /OE2   */
    constexpr static const uint8_t IOPORT_OUT_MASK  = D2 | D1 | D0;

    using BreakpointCb = std::function<void(RP2A03&, void*)>;
    using AudioBufferCb = Apu::AudioBufferCb;
    using IorCb = Gpio::IorCb;
    using IowCb = Gpio::IowCb;

    enum MemoryMappedRegisters : addr_t {
        SQ1_VOL         = 0x4000,
        SQ1_SWEEP       = 0x4001,
        SQ1_LO          = 0x4002,
        SQ1_HI          = 0x4003,
        SQ2_VOL         = 0x4004,
        SQ2_SWEEP       = 0x4005,
        SQ2_LO          = 0x4006,
        SQ2_HI          = 0x4007,
        TRI_LINEAR      = 0x4008,
        unused_TRI      = 0x4009,
        TRI_LO          = 0x400A,
        TRI_HI          = 0x400B,
        NOISE_VOL       = 0x400C,
        unused_NOISE    = 0x400D,
        NOISE_MODE      = 0x400E,
        NOISE_LC        = 0x400F,
        DMC_FREQ        = 0x4010,
        DMC_RAW         = 0x4011,
        DMC_START       = 0x4012,
        DMC_LEN         = 0x4013,
        OAMDMA          = 0x4014,
        SND_CHN         = 0x4015,
        PORT1           = 0x4016,
        PORT2_FRAMECNT  = 0x4017,
        unused_4018     = 0x4018,
        unused_4019     = 0x4019,
        unused_401A     = 0x401A,
        unused_401B     = 0x401B,
        unused_401C     = 0x401C,
        unused_401D     = 0x401D,
        unused_401E     = 0x401E,
        unused_401F     = 0x401F
    };

    /**
     * Initialise this CPU.
     * @param clkf System clock frequency (3 times the desired CPU frequency);
     * @param mmap System mappings.
     * @see ASpace
     */
    RP2A03(size_t clkf, const sptr_t<ASpace>& mmap = {});

    /**
     * Initialise this CPU.
     * @param label Label;
     * @param clkf  Clock frequency (3 times the desired CPU frequency);
     * @param mmap  System mappings.
     * @see ASpace
     */
    RP2A03(std::string_view label, size_t clkf, const sptr_t<ASpace>& mmap);

    virtual ~RP2A03();

    /**
     * @see Apu::audio_buffer(const AudioBufferCb&)
     */
    void audio_buffer(const AudioBufferCb& abuf);

    /**
     * Add an input port callback.
     * @param ior  Input port callback;
     * @param mask Ports (as bit-mask) used by the callback.
     * @see Gpio::add_ior()
     */
    void add_ior(const IorCb& ior, uint8_t mask);

    /**
     * Add an ouput port callback.
     * @param iow  Output port callback;
     * @param mask Ports (as bit-mask) used by the callback.
     * @see Gpio::add_iow()
     */
    void add_iow(const IowCb& iow, uint8_t mask);

    /**
     * Add a breakpoint on a memory address.
     * @see Mos6502::bpadd()
     */
    void bpadd(addr_t addr, const BreakpointCb& cb, void* arg);

    /**
     * Reset this CPU.
     * @see Mos6502::reset()
     */
    void reset() override;

    /**
     * @see Mos6502::read()
     */
    uint8_t read(addr_t addr, Device::ReadMode mode = Device::ReadMode::Read) override;

    /**
     * @see Mos6502::write()
     */
    void write(addr_t addr, uint8_t data) override;

private:
    /**
     * Tick event method.
     * This method has a divider so it must be called by a clock
     * set to a frequency that is three times the desired frequency.
     * @param clk The caller clock.
     * @return The number of clock cycles consumed (the clock will call this method again after
     * this number of cycles have passed); Clockable::HALT if the clock must be terminated.
     * @see Mos6502::tick(const Clock&)
     * @see CPU_DIVIDER
     */
    size_t tick(const Clock& clk) override;

    /**
     * Start the OAM DMA data transfer.
     * @param addr Starting source memory address.
     */
    void oamdma_start(addr_t addr);

    /**
     * Get the status of the OAM DMA transfer.
     * @return True if the OAM DMA data transfer is active; false otherwise.
     */
    bool oamdma_is_running() const;

    /**
     * OAM DMA transfer cycle.
     * Data is read on GET cycles and written on PUT cycles.
     * @param put_cycle True: PUT cycle, false: GET cycle.
     * @return True if the OAM DMA transfer is still active; false otherwise.
     */
    bool oamdma_transfer(bool put_cycle);

    /**
     * DMC DMA transfer cycle.
     * Data is read on GET cycles and sent to the APU's DCM unit.
     * @param put_cycle True: PUT cycle, false: GET cycle.
     * @return True if the DMC DMA transfer is still active; false otherwise.
     */
    bool dmcdma_transfer(bool put_cycle);

    /**
     * DMA transfer.
     * If one or both DMC and OAM DMA data transfers are active
     * the CPU is halted and the DMA transfers take control.
     * When both DMA transfers terminate the CPU is un-halted.
     * @param put_cycle True: PUT cycle, false: GET cycle.
     */
    void dma_transfer(bool put_cycle);

    Apu     _apu;               /* Audio processing unit            */
    bool    _even_tick{};       /* APU clock tick                   */
    addr_t  _oamdma_addr{};     /* OAM DMA source address           */
    addr_t  _oamdma_size{};     /* OAM DMA byte to transfer         */
    addr_t  _oamdma_data{};     /* OAM DMA data being transferred   */
    bool    _oamdma_loaded{};   /* OAM DMA data loaded              */
    Gpio    _ioport{};          /* OUT0-OUT1, /OE1, /OE2            */
    size_t  _cpu_cycles{};      /* CPU cycles count-down            */
};

}
}

using RP2A03 = ricoh::rp2a03::RP2A03;

}
