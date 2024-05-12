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
#include "ula_aspace.hpp"

#include "logger.hpp"
#include "zxsp_params.hpp"

namespace caio {
namespace sinclair {
namespace zxspectrum {

ULAASpace::ULAASpace(const sptr_t<Z80>& cpu, const sptr_t<RAM>& ram, const sptr_t<ROM>& rom,
        const sptr_t<ULAAudio>& audio, const sptr_t<ULAVideo>& video,
        const sptr_t<ZXSpectrumKeyboard>& kbd, const sptr_t<Joystick>& joy,
        const sptr_t<ZXSpectrumTape>& tape)
    : ASpace{},
      _cpu{cpu},
      _audio{audio},
      _video{video},
      _kbd{kbd},
      _joy{joy},
      _tape{tape},
      _mmap{{
          { rom, 0x0000 },  /* 0000 - 3FFF - 16K ROM                        */
          { ram, 0x0000 },  /* 4000 - 7FFF - 16K RAM                        */
          { ram, 0x4000 },  /* 8000 - FFFF - 32K RAM (VRAM at 8000-9718)    */
          { ram, 0x8000 }
      }}

{
    CAIO_ASSERT(_cpu && ram && rom && ram->size() == RAM_SIZE && rom->size() == ROM_SIZE &&
        _audio && _video && _kbd && _joy && _tape);

    ASpace::reset(_mmap, _mmap, ADDR_MASK);
}

ULAASpace::~ULAASpace()
{
}

uint8_t ULAASpace::read(addr_t addr, ReadMode mode)
{
    if (!_cpu->iorq_pin() || mode == ReadMode::Peek) {
        if (mode == ReadMode::Read) {
            switch (_cpu->regs().PC) {
            case LD_LOOK_H_ADDR:
                /* Start the tape when a LOAD operation is started */
                _tape->play();
                break;

            case LD_BYTES_ADDR:
                /* Stop the tape after a data block is loaded */
                _stop_tape = (_cpu->regs().A == tape::HeaderBlock::BLOCKTYPE_DATA);
                break;

            case SA_LD_RET_ADDR:
                /* Block loaded */
                if (!_stop_tape) {
                    break;
                }

                /* PASSTHROUGH */

            case REPORT_Da_ADDR:    /* Break from user */
                /* Force tape stop after a break from user */
                _tape->stop();
                _stop_tape = false;
                break;
            }
        }

        return ASpace::read(addr, mode);
    }

    /*
     * I/O port read.
     */
    address_bus(addr);

    if ((addr & 0x00FF) == 0x00FF) {
        /*
         * Dirty port ($FF).
         * FIXME XXX is this ok?
         */
        return data_bus();
    }

    uint8_t data = 255;

    addr_t port = addr & ULA_PORT_MASK;

    if (port == ULA_PORT) {
        uint8_t row_to_scan = addr >> 8;
        _kbd->write(row_to_scan);

        data = _kbd->read() & KBD_SCAN_MASK;
        data |= (_tape->read() ? CAS_INPUT_BIT : 0);
        data |= UNUSED_INPUT_BITS;

        _audio->beep((data & CAS_INPUT_BIT) * 0.3f);

    } else if (port == KEMPSTON_JOY_PORT) {
        data = _joy->position();
    }

    return data_bus(data);
}

void ULAASpace::write(addr_t addr, uint8_t value)
{
    if (_cpu->iorq_pin() && !_cpu->m1_pin()) {
        /*
         * I/O port write.
         */
        if ((addr & ULA_PORT_MASK) == ULA_PORT) {
            _video->border_colour(value & BORDER_OUTPUT_BITS);
            _audio->beep(value & SPEAKER_OUTPUT_BIT);
            _tape->write(value & CAS_OUTPUT_BIT);
        }

    } else {
        /*
         * Memory write.
         */
        ASpace::write(addr, value);
    }
}

}
}
}
