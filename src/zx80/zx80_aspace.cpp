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

#include <gsl/assert>


namespace caio {
namespace sinclair {
namespace zx80 {

ZX80ASpace::ZX80ASpace(const sptr_t<Z80>& cpu, const devptr_t& ram, const devptr_t& rom,
    const sptr_t<ZX80Video>& video, const sptr_t<ZX80Keyboard>& kbd)
    : ASpace{},
      _cpu{cpu},
      _rom{rom},
      _video{video},
      _kbd{kbd},
      _mmap{}
{
    using namespace gsl;
    Expects((ram->size() == INTERNAL_RAM_SIZE || ram->size() == EXTERNAL_RAM_SIZE) &&
        (rom->size() == ROM4_SIZE || rom->size() == ROM8_SIZE));

    /*
     * The 64K address space consists of a single bank subdivided into 64 blocks of 1K each.
     */
    if (ram->size() == INTERNAL_RAM_SIZE) {
        /*
         * 1K RAM, 4K ROM (also valid for the first 4K of an 8K ROM).
         *   0000-0FFF  BIOS ROM (4K)
         *   1000-1FFF  ROM mirror
         *   2000-2FFF  ROM mirror
         *   3000-3FFF  ROM mirror
         *   4000-43FF  Internal RAM (1K)
         *   4400-47FF  Internal RAM mirror
         *   4800-4BFF  Internal RAM mirror
         *   4C00-4FFF  Internal RAM mirror
         *   5000-53FF  Internal RAM mirror
         *   5400-57FF  Internal RAM mirror
         *   5800-5BFF  Internal RAM mirror
         *   5C00-5FFF  Internal RAM mirror
         *   6000-63FF  Internal RAM mirror
         *   6400-67FF  Internal RAM mirror
         *   6800-6BFF  Internal RAM mirror
         *   6C00-6FFF  Internal RAM mirror
         *   7000-73FF  Internal RAM mirror
         *   7400-77FF  Internal RAM mirror
         *   7800-7BFF  Internal RAM mirror
         *   7C00-7FFF  Internal RAM mirror
         *   8000-8FFF  ROM mirror          (display)
         *   A000-AFFF  ROM mirror          (display)
         *   B000-BFFF  ROM mirror          (display)
         *   B000-B3FF  Internal RAM mirror (display)
         *   B400-B7FF  Internal RAM mirror (display)
         *   B800-BBFF  Internal RAM mirror (display)
         *   BC00-BFFF  Internal RAM mirror (display)
         *   C000-C3FF  Internal RAM mirror (display)
         *   C400-C7FF  Internal RAM mirror (display)
         *   C800-CBFF  Internal RAM mirror (display)
         *   CC00-CFFF  Internal RAM mirror (display)
         *   D000-D3FF  Internal RAM mirror (display)
         *   D400-D7FF  Internal RAM mirror (display)
         *   D800-DBFF  Internal RAM mirror (display)
         *   DC00-DFFF  Internal RAM mirror (display)
         *   E000-E3FF  Internal RAM mirror (display)
         *   E400-E7FF  Internal RAM mirror (display)
         *   E800-EBFF  Internal RAM mirror (display)
         *   EC00-EFFF  Internal RAM mirror (display)
         */
        _mmap = bank_t{{
            { rom,          0x0000 },   /* 4K ROM at $0000 */           /* 0000 */
            { rom,          0x0400 },
            { rom,          0x0800 },
            { rom,          0x0C00 },

            { rom,          0x0000 },   /* 4K ROM mirror at $1000 */    /* 1000 */
            { rom,          0x0400 },
            { rom,          0x0800 },
            { rom,          0x0C00 },

            { rom,          0x0000 },   /* 4K ROM mirror at $2000 */    /* 2000 */
            { rom,          0x0400 },
            { rom,          0x0800 },
            { rom,          0x0C00 },

            { rom,          0x0000 },   /* 4K ROM mirror at $3000 */    /* 3000 */
            { rom,          0x0400 },
            { rom,          0x0800 },
            { rom,          0x0C00 },

            { ram,          0x0000 },   /* 1K RAM at $4000  */          /* 4000 */
            { ram,          0x0000 },   /* 1K RAM mirror    */
            { ram,          0x0000 },   /* 1K RAM mirror    */
            { ram,          0x0000 },   /* 1K RAM mirror    */

            { ram,          0x0000 },   /* 1K RAM mirror    */          /* 5000 */
            { ram,          0x0000 },   /* 1K RAM mirror    */
            { ram,          0x0000 },   /* 1K RAM mirror    */
            { ram,          0x0000 },   /* 1K RAM mirror    */

            { ram,          0x0000 },   /* 1K RAM mirror    */          /* 6000 */
            { ram,          0x0000 },   /* 1K RAM mirror    */
            { ram,          0x0000 },   /* 1K RAM mirror    */
            { ram,          0x0000 },   /* 1K RAM mirror    */

            { ram,          0x0000 },   /* 1K RAM mirror    */          /* 7000 */
            { ram,          0x0000 },   /* 1K RAM mirror    */
            { ram,          0x0000 },   /* 1K RAM mirror    */
            { ram,          0x0000 },   /* 1K RAM mirror    */

            { rom,          0x0000 },   /* 4K ROM mirror (display) */   /* 8000 */
            { rom,          0x0400 },
            { rom,          0x0800 },
            { rom,          0x0C00 },

            { rom,          0x0000 },   /* 4K ROM mirror (display) */   /* 9000 */
            { rom,          0x0400 },
            { rom,          0x0800 },
            { rom,          0x0C00 },

            { rom,          0x0000 },   /* 4K ROM mirror (display) */   /* A000 */
            { rom,          0x0400 },
            { rom,          0x0800 },
            { rom,          0x0C00 },

            { rom,          0x0000 },   /* 4K ROM mirror (display) */   /* B000 */
            { rom,          0x0400 },
            { rom,          0x0800 },
            { rom,          0x0C00 },

            { ram,          0x0000 },   /* 1K RAM mirror (display)  */  /* C000 */
            { ram,          0x0000 },   /* 1K RAM mirror (display)  */
            { ram,          0x0000 },   /* 1K RAM mirror (display)  */
            { ram,          0x0000 },   /* 1K RAM mirror (display)  */

            { ram,          0x0000 },   /* 1K RAM mirror (display)  */  /* D000 */
            { ram,          0x0000 },   /* 1K RAM mirror (display)  */
            { ram,          0x0000 },   /* 1K RAM mirror (display)  */
            { ram,          0x0000 },   /* 1K RAM mirror (display)  */

            { ram,          0x0000 },   /* 1K RAM mirror (display)  */  /* E000 */
            { ram,          0x0000 },   /* 1K RAM mirror (display)  */
            { ram,          0x0000 },   /* 1K RAM mirror (display)  */
            { ram,          0x0000 },   /* 1K RAM mirror (display)  */

            { ram,          0x0000 },   /* 1K RAM mirror (display)  */  /* F000 */
            { ram,          0x0000 },   /* 1K RAM mirror (display)  */
            { ram,          0x0000 },   /* 1K RAM mirror (display)  */
            { ram,          0x0000 }    /* 1K RAM mirror (display)  */
        }};
    } else {
        /*
         * 16K RAM, 4K ROM (also valid for the first 4K of an 8K ROM).
         *   0000-0FFF  BIOS ROM (4K)
         *   1000-1FFF  ROM mirror
         *   2000-2FFF  ROM mirror
         *   3000-3FFF  ROM mirror
         *   4000-7FFF  External RAM (16K)
         *   8000-8FFF  ROM mirror          (display)
         *   9000-9FFF  ROM mirror          (display)
         *   A000-AFFF  ROM mirror          (display)
         *   B000-BFFF  ROM mirror          (display)
         *   C000-FFFF  External RAM mirror (display)
         */
        _mmap = bank_t{{
            { rom,          0x0000 },   /* 4K ROM */                    /* 0000 */
            { rom,          0x0400 },
            { rom,          0x0800 },
            { rom,          0x0C00 },

            { rom,          0x0000 },   /* 4K ROM mirror */             /* 1000 */
            { rom,          0x0400 },
            { rom,          0x0800 },
            { rom,          0x0C00 },

            { rom,          0x0000 },   /* 4K ROM mirror */             /* 2000 */
            { rom,          0x0400 },
            { rom,          0x0800 },
            { rom,          0x0C00 },

            { rom,          0x0000 },   /* 4K ROM mirror */             /* 3000 */
            { rom,          0x0400 },
            { rom,          0x0800 },
            { rom,          0x0C00 },

            { ram,          0x0000 },   /* 16K RAM */                   /* 4000 */
            { ram,          0x0400 },
            { ram,          0x0800 },
            { ram,          0x0C00 },
            { ram,          0x1000 },
            { ram,          0x1400 },
            { ram,          0x1800 },
            { ram,          0x1C00 },
            { ram,          0x2000 },
            { ram,          0x2400 },
            { ram,          0x2800 },
            { ram,          0x2C00 },
            { ram,          0x3000 },
            { ram,          0x3400 },
            { ram,          0x3800 },
            { ram,          0x3C00 },

            { rom,          0x0000 },   /* 4K ROM mirror (display) */   /* 8000 */
            { rom,          0x0400 },
            { rom,          0x0800 },
            { rom,          0x0C00 },

            { rom,          0x0000 },   /* 4K ROM mirror (display) */   /* 9000 */
            { rom,          0x0400 },
            { rom,          0x0800 },
            { rom,          0x0C00 },

            { rom,          0x0000 },   /* 4K ROM mirror (display) */   /* A000 */
            { rom,          0x0400 },
            { rom,          0x0800 },
            { rom,          0x0C00 },

            { rom,          0x0000 },   /* 4K ROM mirror (display) */   /* B000 */
            { rom,          0x0400 },
            { rom,          0x0800 },
            { rom,          0x0C00 },

            { ram,          0x0000 },   /* 16K RAM mirror (display) */  /* C000 */
            { ram,          0x0400 },
            { ram,          0x0800 },
            { ram,          0x0C00 },
            { ram,          0x1000 },
            { ram,          0x1400 },
            { ram,          0x1800 },
            { ram,          0x1C00 },
            { ram,          0x2000 },
            { ram,          0x2400 },
            { ram,          0x2800 },
            { ram,          0x2C00 },
            { ram,          0x3000 },
            { ram,          0x3400 },
            { ram,          0x3800 },
            { ram,          0x3C00 }
        }};
    }

    if (rom->size() == ROM8_SIZE) {
        /*
         * Adjust mappings for the 8K ROM.
         */
        _mmap[4]  = { rom, 0x1000 };
        _mmap[5]  = { rom, 0x1400 };
        _mmap[6]  = { rom, 0x1800 };
        _mmap[7]  = { rom, 0x1C00 };

        _mmap[12] = { rom, 0x1000 };
        _mmap[13] = { rom, 0x1400 };
        _mmap[14] = { rom, 0x1800 };
        _mmap[15] = { rom, 0x1C00 };

        _mmap[36] = { rom, 0x1000 };
        _mmap[37] = { rom, 0x1400 };
        _mmap[38] = { rom, 0x1800 };
        _mmap[39] = { rom, 0x1C00 };

        _mmap[44] = { rom, 0x1000 };
        _mmap[45] = { rom, 0x1400 };
        _mmap[46] = { rom, 0x1800 };
        _mmap[47] = { rom, 0x1C00 };
    }

    ASpace::reset(_mmap, _mmap, ADDR_MASK);
}

void ZX80ASpace::interrupt_req()
{
    _cpu->int_pin(true);
    _int = true;
}

void ZX80ASpace::interrupt_ack()
{
    _cpu->int_pin(false);
    _int = false;
    _video->hsync();
    _counter = (_counter + 1) % 8;
}

inline uint8_t ZX80ASpace::character_bitmap(addr_t base)
{
    /*
     * The refresh address is equal to (I << 8 | R) which is set
     * to the base address of the character bitmap data inside the ROM.
     * Each character is a bitmap block of 8 lines x 8 bits.
     *
     * bitmap_address = 000RRRRC CCCCCLLL
     * RRRR = Bitmap base address
     * CCCC = Character code (D5..D0)
     * LLL  = Character line (_counter)
     *
     * http://searle.x10host.com/zx80/zx80ScopePics.html
     */
    addr_t offset = static_cast<addr_t>(_chcode & CHCODE_MASK) << 3;
    addr_t bitmap_addr = base | offset | _counter;

    uint8_t invert = ((_chcode & VIDEO_INVERT) ? 255 : 0);
    uint8_t bitmap = _rom->read(bitmap_addr) ^ invert;

    return bitmap;
}

uint8_t ZX80ASpace::read(addr_t addr, ReadMode mode)
{
    if (mode == ReadMode::Peek) {
        return (_cpu->iorq_pin() ? io_read(addr) : ASpace::read(addr, mode));
    }

    if (_cpu->iorq_pin()) {
        if (_int) {
            /*
             * Interrupt acknowledged by the CPU:
             * Generate the video HSYNC signal and
             * increment the character line counter.
             */
            interrupt_ack();
        }

        /*
         * I/O read.
         */
        return io_read(addr);
    }

    /*
     * Memory read.
     */
    uint8_t data = ASpace::read(addr, mode);

    if (_vsync) {
        /*
         * VSYNC period: Return the data.
         */
        return data;
    }

    if (_cpu->m1_pin()) {
        /*
         * M1 cycle: Opcode fetch.
         * The video memory is only accessed during M1.
         */
        bool video_access = (addr & VIDEO_ADDR_MASK);
        if (video_access) {
            /*
             * Display opcode fetch.
             */
            bool force_nop = !(data & VIDEO_HALT);
            if (force_nop) {
                /*
                 * Latch the character code and force the NOP opcode on the data bus
                 * (the CPU will read this opcode and execute it).
                 */
                _chcode = data;
                data = Z80::I_NOP;
            } else {
                /*
                 * Halt instruction (or any other data with the VIDEO_HALT bit set):
                 * The rest of the scanline must be cleared, this is done by
                 * setting the character code to "space".
                 * FIXME: the problem with this is that _chcode is 8 pixels long
                 */
                _chcode = CHCODE_BLANK;
            }
        }
    }

    if (_cpu->rfsh_pin()) {
        /*
         * Refresh cycle: The address bus contains the base address of the character
         * bitmap data. This is when the bitmap of the latched character must be
         * painted on the screen.
         */
        uint8_t bitmap = character_bitmap(addr & BITMAP_ADDR_MASK);
        _video->bitmap(bitmap);

        /*
         * The line INTERRUPT_ADDR_MASK (A6) of the refresh address is hardwired to the
         * CPU's /INT pin so when this bit is cleared an interrupt request must be triggered.
         */
        bool irq = ((addr & INTERRUPT_ADDR_MASK) == 0);
        if (irq && !_int) {
            /*
             * Interrupt request.
             */
            interrupt_req();

        } else if (!irq && _int) {
            /*
             * Interrupt acknowledged by the CPU:
             * Generate the video HSYNC signal and
             * increment the character line counter.
             */
            interrupt_ack();
        }
    }

    return data;
}

uint8_t ZX80ASpace::io_read(addr_t addr)
{
    /*
     * I/O Read:
     * +-----+---------------------------------------------------+
     * |     | D7    D6    D5    D4    D3    D2    D1    D0      |
     * +-----+---------------------------------------------------+
     * | A8  |                   V     C     X     Z     SHIFT   |
     * | A9  |                   G     F     D     S     A       |
     * | A10 |                   T     R     E     W     Q       |
     * | A11 |                   5     4     3     2     1       |
     * | A12 |                   6     7     8     9     0       |
     * | A13 |                   Y     U     I     O     P       |
     * | A14 |                   H     J     K     L     NEWLINE |
     * | A15 |                   B     N     M     .     SPACE   |
     * +-----+---------------------------------------------------+
     *
     * A8-A16: Keyboard row to scan (0->Scan, 1->Do not scan)
     *  D0-D4: Keyboard columns (0->Pressed, 1->Released)
     *     D5: unused
     *     D6: 0->60Hz, 1->50Hz
     *     D7: Cassette input (0->None, 1->Pulse)
     *
     * Officially, the keyboard is scanned by delivering a I/O read
     * to port $xxFE, where xx specifies the row to scan (A8-A16).
     * Actually, any read from any port with A0=0 performs a keyboard row scan.
     *
     * Note that during IN A,(port) the previous value of A is set as A8-A15
     * and during IN r,(C), INI or IND, the B register is set as A8-A15.
     *
     * https://problemkaputt.de/zxdocs.htm#zx80zx81memorymapandsystemarea
     */
    uint8_t data = 255;

    /*
     * The keyboard is read on I/O accesses with A0==0.
     */
    if ((addr & KEYBOARD_ADDR_MASK) == 0) {
        /*
         * A row scan starts the VSYNC period and
         * resets the character line counter.
         */
        _vsync = true;
        _counter = 0;
        _video->vsync();

        data = VIDEO_RATE_50HZ; /* Enforced by hardware */

        uint8_t nrow = addr >> 8;
        _kbd->write(nrow);
        data |= _kbd->read();

        //TODO Casette input
    }

    switch (addr) {
    case 0xFF7E:
        data |= VIDEO_RATE_50HZ;
        break;
    default:;
    }

    /*
     * Normal I/O input
     * TODO
     */
    return data;
}

void ZX80ASpace::write(addr_t addr, uint8_t value)
{
    if (_cpu->iorq_pin()) {
        /*
         * Any I/O write ends the VSYNC period.
         */
#if 0 /* FIXME */
        if ((addr & 0xFE) == 0xFE)
#endif
            _vsync = false;
    } else {
        ASpace::write(addr, value);
    }
}

}
}
}
