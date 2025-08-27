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

#include "ula_audio.hpp"
#include "ula_video.hpp"
#include "zxsp_config.hpp"
#include "zxsp_keyboard.hpp"
#include "zxsp_tape.hpp"

#include "aspace.hpp"
#include "joystick.hpp"
#include "ram.hpp"
#include "rom.hpp"
#include "zilog_z80.hpp"

namespace caio {
namespace sinclair {
namespace zxspectrum {

/**
 * ULA address space.
 * The ULA address space controls accesses to memory and I/O ports.
 * @see ASpace
 * @see ULAAudio
 * @see ULAVideo
 * @see ZXSpectrumKeyboard
 * @see ZXSpectrumTape
 */
class ULAASpace : public ASpace {
public:
    constexpr static const addr_t ADDR_MASK             = 0xFFFF;
    constexpr static const size_t BLOCKS                = 4;
    constexpr static const addr_t ULA_PORT_MASK         = 0x00FF;
    constexpr static const addr_t ULA_PORT              = 0x00FE;
    constexpr static const addr_t KEMPSTON_JOY_PORT     = 0x001F;
    constexpr static const addr_t KBD_SCAN_MASK         = ZXSpectrumKeyboard::COLUMN_MASK;
    constexpr static const uint8_t CAS_INPUT_BIT        = D6;
    constexpr static const uint8_t UNUSED_INPUT_BITS    = D5 | D7;
    constexpr static const uint8_t BORDER_OUTPUT_BITS   = D0 | D1 | D2;
    constexpr static const uint8_t CAS_OUTPUT_BIT       = D3;
    constexpr static const uint8_t SPEAKER_OUTPUT_BIT   = D4;

    /**
     * Initialise this ULA address space.
     * @param cpu   CPU;
     * @param ram   RAM (48K);
     * @param rom   ROM (16K);
     * @param audio Audio interface;
     * @param video Video interface;
     * @param kbd   Keyboard;
     * @param joy   Kempston joystick;
     * @param tape  Cassette interface.
     */
    ULAASpace(const sptr_t<Z80>& cpu, const sptr_t<RAM>& ram, const sptr_t<ROM>& rom,
        const sptr_t<class ULAAudio>& audio, const sptr_t<class ULAVideo>& video,
        const sptr_t<ZXSpectrumKeyboard>& kbd, const sptr_t<Joystick>& joy,
        const sptr_t<ZXSpectrumTape>& tape);

    virtual ~ULAASpace() = default;

    /**
     * @see ASpace::read()
     */
    uint8_t read(addr_t addr, ReadMode mode = ReadMode::Read) override;

    /**
     * @see ASpace::write()
     */
    void write(addr_t addr, uint8_t value) override;

private:
    sptr_t<Z80>                 _cpu;
    sptr_t<ULAAudio>            _audio;
    sptr_t<ULAVideo>            _video;
    sptr_t<ZXSpectrumKeyboard>  _kbd;
    sptr_t<Joystick>            _joy;
    sptr_t<ZXSpectrumTape>      _tape;
    bank_t<BLOCKS>              _mmap;

    bool                        _stop_tape{};
};

}
}
}
