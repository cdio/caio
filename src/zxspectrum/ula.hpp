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

#include <memory>
#include <string>

#include "joystick.hpp"
#include "ram.hpp"
#include "rom.hpp"
#include "zilog_z80.hpp"

#include "ula_aspace.hpp"
#include "ula_audio.hpp"
#include "ula_video.hpp"

namespace caio {
namespace sinclair {
namespace zxspectrum {

/**
 * ZX-Spectrum Uncommited Logic Array (ULA).
 * Implemntation of the ULA chip plus other things:
 *   - Address space mappings (memory and I/O port accesses)
 *   - Audio interface
 *   - Video interface
 * @see ULAASpace
 * @see ULAAudio
 * @see ULAVideo
 */
class ULA {
public:
    /**
     * Initialise this ULA device.
     * @param cpu  CPU;
     * @param ram  RAM (48K);
     * @param rom  ROM (16K);
     * @param kbd  Keyboard;
     * @param joy  Kempston joystick;
     * @param tape Cassette interface.
     */
    ULA(const sptr_t<Z80>& cpu, const sptr_t<RAM>& ram, const sptr_t<ROM>& rom,
        const sptr_t<ZXSpectrumKeyboard>& kbd, const sptr_t<Joystick>& joy,
        const sptr_t<ZXSpectrumTape>& tape);

    virtual ~ULA() {
    }

    void reset();

    sptr_t<class ULAAudio> audio() {
        return _audio;
    }

    sptr_t<class ULAVideo> video() {
        return _video;
    }

    sptr_t<class ULAASpace> mmap() {
        return _aspace;
    }

    std::string to_string() const;

private:
    sptr_t<ULAAudio>  _audio;
    sptr_t<ULAVideo>  _video;
    sptr_t<ULAASpace> _aspace;
};

}
}
}
