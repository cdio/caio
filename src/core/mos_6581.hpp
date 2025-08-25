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

#include "clock.hpp"
#include "device.hpp"
#include "signal.hpp"
#include "ui.hpp"
#include "utils.hpp"

namespace caio {
namespace mos {
namespace mos_6581 {

constexpr static const unsigned SAMPLING_RATE = 44100;
constexpr static const float DT               = 1.0f / SAMPLING_RATE;
constexpr static const float  SAMPLES_TIME    = 0.020f;
constexpr static const size_t SAMPLES         = static_cast<size_t>(utils::ceil(SAMPLING_RATE * SAMPLES_TIME));
constexpr static const size_t CHANNELS        = 1;

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
    fp_t  freq() const;
    void  width_hi(uint8_t wh);
    void  width_lo(uint8_t wl);
    fp_t  width() const;
    void  type(unsigned wt);
    void  ring(bool rb);
    void  test(bool tb);
    void  sync(bool sb);
    bool  is_test() const;
    fp_t  amplitude() const;
    fp_t  time() const;
    void  rand_reset();
    fp_t  tick();

private:
    void    setfreq();
    void    setwidth();
    uint8_t rand();
    fp_t    noise();

    fp_t        _clkf;
    Oscillator& _syncos;

    WaveType    _type;

    bool        _ring;
    bool        _test;
    bool        _sync;

    uint16_t    _ufreq;
    fp_t        _freq;
    fp_t        _T;

    uint16_t    _uwidth;
    fp_t        _width;

    uint32_t    _rreg;
    int         _ndelay;
    fp_t        _nvalue;

    fp_t        _A;
    fp_t        _t;

    friend Serializer& operator&(Serializer&, Oscillator&);
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
    fp_t  amplitude() const;
    fp_t  tick();

private:
    fp_t  _tadj;                /* Timing adjustment factor: 1MHz / system_clock_freq */

    fp_t  _attack_time;
    fp_t  _decay_time;
    fp_t  _sustain;
    fp_t  _release_time;
    fp_t  _release_A;

    fp_t  _t;
    fp_t  _A;

    bool  _gate;
    Cycle _cycle;

    static const std::array<fp_t, 16> attack_times;
    static const std::array<fp_t, 16> decay_times;

    friend Serializer& operator&(Serializer&, Envelope&);
};

class Voice {
public:
    Voice(unsigned clkf, Voice& svoice);

    void reset();
    void freq_hi(uint8_t fh);
    void freq_lo(uint8_t fl);
    void width_hi(uint8_t pwh);
    void width_lo(uint8_t pwl);
    void attack(uint8_t value);
    void decay(uint8_t value);
    void sustain(uint8_t value);
    void release(uint8_t value);
    void control(uint8_t value);
    fp_t tick();
    Oscillator& osc();
    const Oscillator& osc() const;
    const Envelope& env() const;

private:
    Oscillator _osc;
    Envelope   _env;

    friend Serializer& operator&(Serializer&, Voice&);
};

class Filter {
public:
    enum FilterType : uint8_t {
        FILTER_NONE = 0x00,
        FILTER_LO   = 0x10,
        FILTER_HI   = 0x20,
        FILTER_BA   = 0x40,
        FILTER_LOHI = FILTER_LO | FILTER_HI,
        FILTER_LOBA = FILTER_LO | FILTER_BA,
        FILTER_HIBA = FILTER_HI | FILTER_BA,
        FILTER_LHBA = FILTER_LO | FILTER_HI | FILTER_BA,
        FILTER_MASK = FILTER_LO | FILTER_HI | FILTER_BA
    };

    Filter();

    void reset();
    void freq_hi(uint8_t hi);
    void freq_lo(uint8_t lo);
    void resonance(uint8_t rs);
    void type(uint8_t type);
    bool is_enabled() const;
    bool is_disabled() const;
    fp_t frequency() const;
    fp_t Q() const;
    fp_t operator()(fp_t v);

private:
    void generate();

    uint16_t                _ufc{};
    fp_t                    _pufc{};
    uint8_t                 _res{};
    fp_t                    _pres{};
    FilterType              _type{};
    FilterType              _ptype{};
    signal::Filter<3, 3>    _flt{};
    fp_t                    _gain_comp{};

    friend Serializer& operator&(Serializer&, Filter&);
};

/**
 * MOS-6581 Sound Interfaace Device (SID).
 * The real 6581 does not respect its own specifications consequently
 * it is very difficult to implement an accurate emulator of this chip.
 *
 * This implementation is quite accurate but it is not (and it does
 * not pretend to be) at the level of other solutions that used a big
 * deal of reverse engineering such as reSID.
 *
 * Follows an extract of an interview done to Bob Yannes (the author
 * of this chip):
 *
 * > "Can you give us a short overview of the SIDs internal architecture ?
 * > It's pretty brute-force, I didn't have time to be elegant. Each "voice" consisted of
 * > an Oscillator, a Waveform Generator, a Waveform Selector, a Waveform D/A converter,
 * > a Multiplying D/A converter for amplitude control and an Envelope Generator for modulation.
 * > The analog output of each voice could be sent through a Multimode Analog Filter or bypass
 * > the filter and a final Multiplying D/A converter provided overall manual volume control.
 * > As I recall, the Oscillator is a 24-bit phase-accumulating design of which thelower 16-bits
 * > are programmable for pitch control. The output of the accumulator goes directly to a D/A
 * > converter through a waveform selector. Normally, the output of a phase-accumulating
 * > oscillator would be used as an address into memory which contained a wavetable, but SID had
 * > to be entirely self-contained and there was no room at all for a wavetable on the chip.
 * >
 * > The Sawtooth waveform was created by sending the upper 12-bits of the accumulator to the
 * > 12-bit Waveform D/A.
 * >
 * > The Triangle waveform was created by using the MSB of the accumulator to invert the
 * > remaining upper 11 accumulator bits using EXOR gates. These 11 bits were then left-shifted
 * > (throwing away the MSB) and sent to the Waveform D/A (so the resolution of the triangle
 * > waveform was half that of the sawtooth, but the amplitude and frequency were the same).
 * > The Pulse waveform was created by sending the upper 12-bits of the accumulator to a 12-bit
 * > digital comparator. The output of the comparator was either a one or a zero. This single
 * > output was then sent to all 12 bits of the Waveform D/A.
 * >
 * > The Noise waveform was created using a 23-bit pseudo-random sequence generator (i.e., a
 * > shift register with specific outputs fed back to the input through combinatorial logic).
 * > The shift register was clocked by one of the intermediate bits of the accumulator to keep
 * > the frequency content of the noise waveform relatively the same as the pitched waveforms.
 * > The upper 12-bits of the shift register were sent to the Waveform D/A.
 * >
 * > Since all of the waveforms were just digital bits, the Waveform Selector consisted of
 * > multiplexers that selected which waveform bits would be sent to the Waveform D/A. The
 * > multiplexers were single transistors and did not provide a "lock-out", allowing
 * > combinations of the waveforms to be selected. The combination was actually a logical
 * > ANDing of the bits of each waveform, which produced unpredictable results, so I didn't
 * > encourage this, especially since it could lock up the pseudo-random sequence generator by
 * > filling it with zeroes.
 * >
 * > The output of the Waveform D/A (which was an analog voltage at this point) was fed into
 * > the reference input of an 8-bit multiplying D/A, creating a DCA (digitally-controlled-amplifier).
 * > The digital control word which modulated the amplitude of the waveform came from the
 * > Envelope Generator.
 * >
 * > The Envelope Generator was simply an 8-bit up/down counter which, when triggered by the
 * > Gate bit, counted from 0 to 255 at the Attack rate, from 255 down to the programmed
 * > Sustain value at the Decay rate, remained at the Sustain value until the Gate bit was
 * > cleared then counted down from the Sustain value to 0 at the Release rate.
 * >
 * > A programmable frequency divider was used to set the various rates (unfortunately I don't
 * > remember how many bits the divider was, either 12 or 16 bits). A small look-up table
 * > translated the 16 register-programmable values to the appropriate number to load into
 * > the frequency divider. Depending on what state the Envelope Generator was in
 * > (i.e. ADS or R), the appropriate register would be selected and that number would be
 * > translated and loaded into the divider. Obviously it would have been better to have
 * > individual bit control of the divider which would have provided great resolution for each
 * > rate, however I did not have enough silicon area for a lot of register bits. Using this
 * > approach, I was able to cram a wide range of rates into 4 bits, allowing the ADSR to be
 * > defined in two bytes instead of eight. The actual numbers in the look-up table were
 * > arrived at subjectively by setting up typical patches on a Sequential Circuits Pro-1
 * > and measuring the envelope times by ear (which is why the available rates seem strange)!
 * >
 * > In order to more closely model the exponential decay of sounds, another look-up table on
 * > the output of the Envelope Generator would sequentially divide the clock to the Envelope
 * > Generator by two at specific counts in the Decay and Release cycles. This created a
 * > piece-wise linear approximation of an exponential. I was particularly happy how well
 * > this worked considering the simplicity of the circuitry. The Attack, however, was linear,
 * > but this sounded fine.
 * >
 * > A digital comparator was used for the Sustain function. The upper four bits of the Up/Down
 * > counter were compared to the programmed Sustain value and would stop the clock to the
 * > Envelope Generator when the counter counted down to the Sustain value. This created 16
 * > linearly spaced sustain levels without havingto go through a look-up table translation
 * > between the 4-bit register value and the 8-bit Envelope Generator output. It also meant
 * > that sustain levels were adjustable in steps of 16. Again, more register bits would have
 * > provided higher resolution.
 * >
 * > When the Gate bit was cleared, the clock would again be enabled, allowing the counter to
 * > count down to zero. Like an analog envelope generator, the SID Envelope Generator would
 * > track the Sustain level if it was changed to a lower value during the Sustain portion of
 * > the envelope, however, it would not count UP if the Sustain level were set higher.
 * > The 8-bit output of the Envelope Generator was then sent to the Multiplying D/A converter
 * > to modulate the amplitude of the selected Oscillator Waveform (to be technically accurate,
 * > actually the waveform was modulating the output of the Envelope Generator, but the
 * > result is the same).
 * >
 * > Hard Sync was accomplished by clearing the accumulator of an Oscillator based on the
 * > accumulator MSB of the previous oscillator.
 * >
 * > Ring Modulation was accomplished by substituting the accumulator MSB of an oscillator
 * > in the EXOR function of the triangle waveform generator with the accumulator MSB of the
 * > previous oscillator. That is why the triangle waveform must be selected to use
 * > Ring Modulation.
 * >
 * > The Filter was a classic multi-mode (state variable) VCF design. There was no way to
 * > create a variable transconductance amplifier in our NMOS process, so I simply used FETs
 * > as voltage-controlled resistors to control the cutoff frequency. An 11-bit D/A converter
 * > generates the control voltage for the FETs (it's actually a 12-bit D/A, but the LSB had
 * > no audible affect so I disconnected it!).
 * > Filter resonance was controlled by a 4-bit weighted resistor ladder. Each bit would turn
 * > on one of the weighted resistors and allow a portion of the output to feed back to the
 * > input. The state-variable design provided simultaneous low-pass, band-pass and high-pass
 * > outputs. Analog switches selected which combination of outputs were sent to the final
 * > amplifier (a notch filter was created by enabling both the high and low-pass outputs
 * > simultaneously).
 * >
 * > The filter is the worst part of SID because I could not create high-gain op-amps in NMOS,
 * > which were essential to a resonant filter. In addition, the resistance of the FETs
 * > varied considerably with processing, so different lots of SID chips had different
 * > cutoff frequency characteristics. I knew it wouldn't work very well, but it was better
 * > than nothing and I didn't have time to make it better.
 * > Analog switches were also used to either route an Oscillator output through or around
 * > the filter to the final amplifier. The final amp was a 4-bit multiplying D/A converter
 * > which allowed the volume of the output signal to be controlled. By stopping an Oscillator,
 * > it was possible to apply a DC voltage to this D/A. Audio could then be created by having
 * > the microprocessor write the Final Volume register in real-time. Game programs often used
 * > this method to synthesize speech or play "sampled" sounds.
 * > An external audio input could also be mixed in at the final amp or processed through the
 * > filter.
 * >
 * > The Modulation registers were probably never used since they could easily be simulated in
 * > software without having to give up a voice. For novice programmers they provided a way to
 * > create vibrato or filter sweeps without having to write much code (just read the value
 * > from the modulation register and write it back to the frequency register). These registers
 * > just give microprocessor access to the upper 8 bits of the instantaneous value of the
 * > waveform and envelope of Voice 3. Since you probably wouldn't want to hear the modulation
 * > source in the audio output, an analog switch was provided to turn off the audio output
 * > of Voice 3."
 *
 * @see https://en.wikipedia.org/wiki/MOS_Technology_SID
 * @see https://www.c64-wiki.com/wiki/Commodore_64_Programmer%27s_Reference_Guide
 * @see http://www.sidmusic.org/sid/sidtech5.html
 * @see http://sidmusic.org/sid/yannes.html
 */
class Mos6581 : public Device, public Clockable {
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

    using AudioBufferCb = std::function<ui::AudioBuffer()>;

    /**
     * Initalise this SID instance.
     * @param label Label assigned to this instance;
     * @param clkf  System clock frequency.
     */
    Mos6581(std::string_view label, unsigned clkf);

    virtual ~Mos6581() = default;

    /**
     * Set the audio buffer provider.
     * @param abuf Audio buffer provider.
     */
    void audio_buffer(const AudioBufferCb& abuf)
    {
        _audio_buffer = abuf;
    }

    /**
     * @see Device::reset()
     */
    void reset() override;

    /**
     * @see Device::size()
     */
    size_t size() const override
    {
        return REGMAX;
    }

    /**
     * @see Device::dev_read()
     */
    uint8_t dev_read(size_t addr, ReadMode mode = ReadMode::Read) override;

    /**
     * @see Device::dev_write()
     */
    void dev_write(size_t addr, uint8_t value) override;

    /**
     * @see Device::dump()
     */
    std::ostream& dump(std::ostream& os, size_t base = 0) const override;

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

    size_t                      _samples_cycles;
    Voice                       _voice_1;
    Voice                       _voice_2;
    Voice                       _voice_3;

    std::array<fp_t, SAMPLES>   _v{};
    std::array<fp_t, SAMPLES>   _v4{};
    bool                        _voice_1_filtered{};
    bool                        _voice_2_filtered{};
    bool                        _voice_3_filtered{};
    bool                        _voice_3_off{};
    Filter                      _filter{};
    fp_t                        _volume{};
    fp_t                        _prev_volume{};
    size_t                      _sample_index{};
    size_t                      _prev_index{};
    uint8_t                     _last_value{};
    AudioBufferCb               _audio_buffer{};

    friend Serializer& operator&(Serializer&, Mos6581&);
};

}
}

using Mos6581 = mos::mos_6581::Mos6581;

}
