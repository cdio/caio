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

ZX80ASpace::ZX80ASpace(const sptr_t<Z80>& cpu, const devptr_t& ram, const devptr_t& rom, const sptr_t<ZX80Video>& video)
    : ASpace{},
      _cpu{cpu},
      _video{video},
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

        _mmap[32] = { rom, 0x0000 };  //display TODO: create two ram/rom devices to be used by video interface, it requires only M1
        _mmap[33] = { rom, 0x0400 };  //display       the ram/rom device send data to the video interface
        _mmap[34] = { rom, 0x0800 };  //display       so these devices control the video signal generation + timing
        _mmap[35] = { rom, 0x0C00 };  //display       reproducing the flickering

        _mmap[36] = { rom, 0x1000 };  //display
        _mmap[37] = { rom, 0x1400 };  //display
        _mmap[38] = { rom, 0x1800 };  //display
        _mmap[39] = { rom, 0x1C00 };  //display
    }

    ASpace::reset(_mmap, _mmap, ADDR_MASK);
}

uint8_t ZX80ASpace::read(addr_t addr, ReadMode mode)
{
    if (mode == ReadMode::Peek) {
        return ASpace::read(addr, mode);
    }

    /*
     * Refresh address A6 bit is hardwired to the cpu's /INT pin.
     */
    bool irq = _cpu->rfsh_pin() && ((addr & A6) == 0);
    if (irq && !_int) {
        _cpu->int_pin(true);
        _int = true;
    } else if (!irq && _int) {
        _cpu->int_pin(false);
        _int = false;
    }

    uint8_t data = ASpace::read(addr, mode);

#if 1
        if (_cpu->rfsh_pin()) {
            /*
             * Refresh cycle:
             * The address bus contains the base address of the character data in ROM,
             * this address is sampled by a shift register to generate the video signal.
             * Here, this data byte is sent to the video emulator which paints the 8 bits
             * at once during the next video tick (see ZX80Video::tick()).
             */
            bool invert = data & VIDEO_INVERT;
            uint8_t vdata = data & VDATA_MASK;
            _video->video_data(vdata, invert);

        } else {
            /*
             * CPU is accessing the address space.
             */
            bool video_access = _cpu->m1_pin() && (addr & VIDEO_ADDRESS);
            if (video_access) {
                /*
                 * Display opcode fetch.
                 */
                bool nop_generator = !(data & VIDEO_NOP);
    log.error(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> video accessed nop generator %d\n", nop_generator);
                if (nop_generator) {
                    /*
                     * Load character data and force a NOP on the data bus.
                     */
                    uint8_t vdata = data & VDATA_MASK;
                    bool invert = data & VIDEO_INVERT;;
                    _video->video_data(vdata, invert);
    log.error(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ch=%02x\n", vdata);
//                    _video->addr_lo(video_data);
                    return Z80::I_NOP; /* The CPU must read a NOP */

                } else {
                    _video->hsync();
                    return data;
                }
            }
        }

        if (_cpu->m1_pin()) {
            /*
             * Video interface ticked at M1.
             */
            //XXX M1 lasts for a long time, it must be edge triggered here
//    log.error(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> tick\n");
//            _video->tick();
        }

        /*
         * I/O Read.
         */
        if (_cpu->iorq_pin()) {
            /* TODO XXX
             *
             * From "https://problemkaputt.de/zxdocs.htm#zx80zx81memorymapandsystemarea" :
             *
             * Input from Port FEh (or any other port with A0 zero)
             * Reading from this port initiates the Vertical Retrace period (and accordingly,
             * Cassette Output becomes Low), and resets the LINECNTR register to zero,
             * LINECNTR remains stopped/zero until user terminates retrace - In the ZX81,
             * all of the above happens only if NMIs are disabled.
             *
             *   Bit  Expl.
             *   0-4  Keyboard column bits (0=Pressed)
             *   5    Not used             (1)
             *   6    Display Refresh Rate (0=60Hz, 1=50Hz)
             *   7    Cassette input       (0=Normal, 1=Pulse)
             *
             * When reading from the keyboard, one of the upper bits (A8-A15) of the I/O
             * address must be "0" to select the desired keyboard row (0-7).
             * (When using IN A,(nn), the old value of the A register is output as upper
             * address bits and <nn> as lower bits. Otherwise, ie. when using IN r,(C) or
             * INI or IND, the BC register is output to the address bus.)
             *
             * The ZX81/ZX80 Keyboard Matrix
             *
             *   Port____Line____Bit__0____1____2____3____4__
             *   FEFEh  0  (A8)     SHIFT  Z    X    C    V
             *   FDFEh  1  (A9)       A    S    D    F    G
             *   FBFEh  2  (A10)      Q    W    E    R    T
             *   F7FEh  3  (A11)      1    2    3    4    5
             *   EFFEh  4  (A12)      0    9    8    7    6
             *   DFFEh  5  (A13)      P    O    I    U    Y
             *   BFFEh  6  (A14)    ENTER  L    K    J    H
             *   7FFEh  7  (A15)     SPC   .    M    N    B
             */
            if ((addr & 1) == 0) {
 //               _video->vsync();
            }

            //XXX
            return 0x7F;
        }
#else

    if (_cpu->halt_pin()) {
        _video->stopline();
        return 0;
    }

    if ((addr & A15) && !(data & CH_NOP_BIT)) {
        if (!_cpu->rfsh_pin()) {
            _video->video_data(addr
            _video->addr_lo(data & CH_MASK);

        } else {
            /*
             * Refresh cycle:
             * The address bus contains the address of the current character line in ROM,
             * this address must be captured by the video interface.
             */
            _video->addr_hi(addr >> 8);
        _video->tick();
            return 0;
        }
    }

//        bool video_access = (addr & A15) && !_cpu->halt_pin() && !(data & CH_NOP_BIT);
//        if (video_access) {
//            _video->addr_lo(addr & CH_MASK);

            /*
             * Display opcode fetch.
             */
//            bool nop_generator = !(data & CH_NOP_BIT);
//            if (nop_generator) {
//                return 0x00; /* The CPU must read a NOP */
//            } else {
                /*
                 * Finish the current scanline.
                 */
//log.error(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> STOPLINE\n");
//                _video->stopline();
//            }
//    }

//    if (_cpu->m1_pin()) {
        /*
         * Video interface ticked at M1.
         */
        //XXX M1 lasts for a long time, it must be edge triggered here
 //       _video->tick();
  //  }
#endif

    return data;
}

void ZX80ASpace::write(addr_t addr, uint8_t value)
{
    /*
     * A6 hard-wired to cpu's INT pin.
     */
    bool irq = _cpu->rfsh_pin() && ((addr & A6) == 0);
    if (irq && !_int) {
        _cpu->int_pin(true);
        _int = true;
    } else if (!irq && _int) {
        _cpu->int_pin(false);
        _int = false;
    }

    if (_cpu->rfsh_pin()) {
        return;
    }

    /*
     * I/O write.
     */
    if (_cpu->iorq_pin()) {
        /* TODO XXX
         *
         * From "https://problemkaputt.de/zxdocs.htm#zx80zx81memorymapandsystemarea" :
         *
         * Output to Port FFh (or ANY other port)
         * Writing any data to any port terminates the Vertical Retrace period,
         * and restarts the LINECNTR counter. The retrace signal is also output
         * to the cassette (ie. the Cassette Output becomes High).
         *
         * Input from Port FEh (or any other port with A0 zero)
         * Reading from this port initiates the Vertical Retrace period (and accordingly,
         * Cassette Output becomes Low), and resets the LINECNTR register to zero,
         * LINECNTR remains stopped/zero until user terminates retrace - In the ZX81,
         * all of the above happens only if NMIs are disabled.
         *
         *   Bit  Expl.
         *   0-4  Keyboard column bits (0=Pressed)
         *   5    Not used             (1)
         *   6    Display Refresh Rate (0=60Hz, 1=50Hz)
         *   7    Cassette input       (0=Normal, 1=Pulse)
         *
         * When reading from the keyboard, one of the upper bits (A8-A15) of the I/O
         * address must be "0" to select the desired keyboard row (0-7).
         * (When using IN A,(nn), the old value of the A register is output as upper
         * address bits and <nn> as lower bits. Otherwise, ie. when using IN r,(C) or
         * INI or IND, the BC register is output to the address bus.)
         *
         * The ZX81/ZX80 Keyboard Matrix
         *
         *   Port____Line____Bit__0____1____2____3____4__
         *   FEFEh  0  (A8)     SHIFT  Z    X    C    V
         *   FDFEh  1  (A9)       A    S    D    F    G
         *   FBFEh  2  (A10)      Q    W    E    R    T
         *   F7FEh  3  (A11)      1    2    3    4    5
         *   EFFEh  4  (A12)      0    9    8    7    6
         *   DFFEh  5  (A13)      P    O    I    U    Y
         *   BFFEh  6  (A14)    ENTER  L    K    J    H
         *   7FFEh  7  (A15)     SPC   .    M    N    B
         */
        _video->hsync();

    } else {
        ASpace::write(addr, value);
    }
}

}
}
}
