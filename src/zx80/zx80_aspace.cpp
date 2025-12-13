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
#include "zx80_aspace.hpp"

#include "logger.hpp"

namespace caio {
namespace sinclair {
namespace zx80 {

ZX80ASpace::ZX80ASpace(const sptr_t<Z80>& cpu, const devptr_t& ram, const devptr_t& rom,
    const sptr_t<ZX80Video>& video, const sptr_t<ZX80Keyboard>& kbd, const sptr_t<ZX80Cassette>& cass)
    : ASpace{},
      _cpu{cpu},
      _ram{ram},
      _ram_mask{INTERNAL_RAM_MASK},
      _rom{rom},
      _rom_mask{ROM_MASK},
      _video{video},
      _kbd{kbd},
      _cass{cass}
{
    CAIO_ASSERT(_cpu && _ram && _rom && _video && _kbd && _cass &&
        (_ram->size() == INTERNAL_RAM_SIZE || _ram->size() == EXTERNAL_RAM_SIZE) &&
        (_rom->size() == ROM_SIZE || _rom->size() == ROM8_SIZE));

    if (_ram->size() == EXTERNAL_RAM_SIZE) {
        _ram_mask = EXTERNAL_RAM_MASK;
    }

    if (_rom->size() == ROM8_SIZE) {
        _rom_mask = ROM8_MASK;
    }

    _cpu->rfsh_pin([this](bool on) {
        if (on) {
            rfsh_cycle();
        }
    });

    _cpu->iorq_pin([this](bool on) {
        if (on && _cpu->m1_pin() && _intreq) {
            /*
             * HSync started when an interrupt request
             * is acknowledged by the CPU.
             */
            hsync();
            _intreq = false;
        }
    });
}

void ZX80ASpace::reset()
{
    _counter = 0;
    _intpin = 0;
    if (_intreq) {
        _cpu->int_pin(false);
        _intreq = 0;
    }
}

ZX80ASpace::AccessType ZX80ASpace::access_type(addr_t addr) const
{
    return (_cpu->iorq_pin() ? AccessType::IO : ((addr & RAM_ADDR_MASK) ? AccessType::RAM : AccessType::ROM));
}

void ZX80ASpace::address_bus(addr_t addr)
{
    ASpace::address_bus(addr);

    /*
     * The line A6 of the address bus is connected to the CPU /INT pin.
     */
    bool intpin = ((addr & A6) == 0);
    if (_intpin != intpin) {
        _intpin = intpin;
        _cpu->int_pin(_intpin);
        if (_intpin) {
            _intreq = true;
        }
    }
}

void ZX80ASpace::vsync(bool on)
{
    _vsync = on;
    _video->vsync(on);
    _counter = 0;
}

void ZX80ASpace::hsync()
{
    if (!_vsync) {
        _video->hsync();
        _counter = (_counter + 1) & 7;
    }
}

void ZX80ASpace::rfsh_cycle()
{
    if (!_blank) {
        /*
         * The refresh address is equal to (I << 8 | R) which is set to the
         * base address (relative to the ROM) of the character bitmap data.
         *
         * bitmap_address = xxxRRRRC CCCCCLLL
         *
         * RRRR = Bitmap base address
         * CCCC = Latched character code (D5..D0)
         * LLL  = Line counter
         *
         * http://searle.x10host.com/zx80/zx80ScopePics.html
         */
        addr_t rfsh_addr = ASpace::address_bus();
        addr_t base = rfsh_addr & BITMAP_ADDR_MASK;
        addr_t offset = (static_cast<addr_t>(_chcode & CHCODE_MASK) << 3) | _counter;

        uint8_t invert = ((_chcode & BITMAP_INVERT) ? 255 : 0);
        uint8_t bdata = _rom->read(base + offset) ^ invert;

        _video->bitmap(bdata);
    }
}

uint8_t ZX80ASpace::io_read(addr_t port)
{
    /*
     * I/O Read:
     * A0 set:
     *      255.
     *
     * A0 cleared:
     *     +-----+---------------------------------------------------+
     *     |     | D7    D6    D5    D4    D3    D2    D1    D0      |
     *     +-----+---------------------------------------------------+
     *     | A8  | c     1     1     V     C     X     Z     SHIFT   |
     *     | A9  | c     1     1     G     F     D     S     A       |
     *     | A10 | c     1     1     T     R     E     W     Q       |
     *     | A11 | c     1     1     5     4     3     2     1       |
     *     | A12 | c     1     1     6     7     8     9     0       |
     *     | A13 | c     1     1     Y     U     I     O     P       |
     *     | A14 | c     1     1     H     J     K     L     NEWLINE |
     *     | A15 | c     1     1     B     N     M     .     SPACE   |
     *     +-----+---------------------------------------------------+
     *     A8-A16: Keyboard row to scan (0=Scan, 1=Do not scan)
     *      D0-D4: Keyboard columns (0=Pressed, 1=Released)
     *         D5: Unused (1)
     *         D6: 0=60Hz, 1=50Hz
     *         D7: Cassette input (0=None, 1=Pulse)
     *
     *     VSync period started
     *     Cassette output forced to 0
     */
    if ((port & A0) == A0) {
        return data_bus();
    }

    /*
     * Start the VSync period.
     */
    vsync(true);

    /*
     * Keyboard scan.
     */
    _kbd->write(port >> 8);
    uint8_t data = _kbd->read() & KBD_SCAN_MASK;

    /*
     * Cassette output forced low.
     */
    _cass->write(0);

    /*
     * The ZX80 does not have a motor control mechanism so the
     * user must press "play" after a LOAD command is delivered.
     * The code below is used to detect when the LOAD procedure
     * is being executed and automatially press "play" by calling
     * the cassette read() method.
     */
    addr_t pc = _cpu->regs().PC;
    if (_rom->size() == ROM_SIZE) {
        switch (pc) {
        case ROM4_LOAD_2_4:
        case ROM4_LOAD_4_4:
        case ROM4_LOAD_6_3:
            data |= (_cass->read() ? CAS_IN : 0);
            break;
        case ROM4_SAVE_5_6:
        case ROM4_SAVE_1_4:
            /* Check for abort during save operation. Do not restart the cassette */
            break;
        default:
            _cass->restart();
        }
    } else {
        switch (pc) {
        case ROM8_IN_BYTE_8:
        case ROM8_GET_BIT_8:
            data |= (_cass->read() ? CAS_IN : 0);
            break;
        case ROM8_BREAK_1_4:
            /* Check for abort during save operation. Do not restart the cassette */
            break;
        default:
            _cass->restart();
        }
    }

    /*
     * Other ports.
     */
    data |= VIDEO_RATE_50HZ;
    data |= D5;                 /* D5 unused */

    return data;
}

uint8_t ZX80ASpace::read(addr_t addr, ReadMode mode)
{
    if (mode == ReadMode::Peek) {
        return ((addr & RAM_ADDR_MASK) ? _ram->read(addr & _ram_mask, mode) : _rom->read(addr & _rom_mask, mode));
    }

    address_bus(addr);

    uint8_t data{};
    auto type = access_type(addr);

    switch (type) {
    case AccessType::IO:
        data = io_read(addr);
        break;
    case AccessType::RAM:
        data = _ram->read(addr & _ram_mask, mode);
        break;
    case AccessType::ROM:
        data = _rom->read(addr & _rom_mask, mode);
        break;
    }

    bool video_access = (addr & VIDEO_ACCESS_MASK);
    bool enable_blank = (data & ENABLE_BLANK);

    _blank = !video_access || enable_blank || _cpu->halt_pin();

    bool force_nop = (!_blank && _cpu->m1_pin());

    if (force_nop) {
        /*
         * Data is a character code, latch this
         * value and make the CPU see a NOP opcode.
         */
        _chcode = data;
        data = Z80::I_NOP;
    }

    return data_bus(data);
}

void ZX80ASpace::write(addr_t addr, uint8_t value)
{
    address_bus(addr);
    data_bus(value);

    switch (access_type(addr)) {
    case AccessType::IO:
        vsync(false);       /* VSync period terminated */
        _cass->write(1);    /* Cassette output forced high */
        break;

    case AccessType::RAM:
        _ram->write(addr & _ram_mask, value);
        break;

    case AccessType::ROM:
        break;
    }
}

std::ostream& ZX80ASpace::dump(std::ostream& os) const
{
    os.setf(std::ios::left, std::ios::adjustfield);

    addr_t addr = 0;

    do {
        const devptr_t& dev{access_type(addr) == AccessType::RAM ? _ram : _rom};
        addr_t last = dev->size() - 1;

        /* Address range */
        os << utils::to_string(addr) << "-" << utils::to_string<addr_t>(addr + last) << "    ";

        /* Read maps */
        os << utils::to_string<addr_t>(0) << "-" << utils::to_string(last) << " ";
        os.width(25);
        os << dev->label() << "  ";

        /* Write maps */
        os << utils::to_string<addr_t>(0) << "-" << utils::to_string(last) << " ";
        os.width(25);
        os << dev->label() << "\n";

        addr += dev->size();

    } while (addr != 0);

    return os;
}

Serializer& operator&(Serializer& ser, ZX80ASpace& mmap)
{
    ser & static_cast<ASpace&>(mmap)
        & mmap._chcode
        & mmap._counter
        & mmap._blank
        & mmap._intpin
        & mmap._intreq
        & mmap._vsync;

    return ser;
}

}
}
}
