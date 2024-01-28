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

#include "snapshot.hpp"


namespace caio {
namespace sinclair {
namespace zxspectrum {

/**
 * Z80 snapshot file format header (version 1).
 *
 *     Offset  Length  Description
 *     ---------------------------------------------------------------------
 *     0       1       A register
 *     1       1       F register
 *     2       2       BC register pair (LSB, i.e. C, first)
 *     4       2       HL register pair
 *     6       2       Program counter
 *     8       2       Stack pointer
 *     10      1       Interrupt register
 *     11      1       Refresh register (Bit 7 is not significant!)
 *     12      1       Bit 0  : Bit 7 of the R-register
 *                     Bit 1-3: Border colour
 *                     Bit 4  : 1=Basic SamRom switched in
 *                     Bit 5  : 1=Block of data is compressed
 *                     Bit 6-7: No meaning
 *     13      2       DE register pair
 *     15      2       BC' register pair
 *     17      2       DE' register pair
 *     19      2       HL' register pair
 *     21      1       A' register
 *     22      1       F' register
 *     23      2       IY register (Again LSB first)
 *     25      2       IX register
 *     27      1       Interrupt flipflop, 0=DI, otherwise EI
 *     28      1       IFF2 (not particularly important...)
 *     29      1       Bit 0-1: Interrupt mode (0, 1 or 2)
 *                     Bit 2  : 1=Issue 2 emulation
 *                     Bit 3  : 1=Double interrupt frequency
 *                     Bit 4-5: 1=High video synchronisation
 *                              3=Low video synchronisation
 *                              0,2=Normal
 *                     Bit 6-7: 0=Cursor/Protek/AGF joystick
 *                              1=Kempston joystick
 *                              2=Sinclair 2 Left joystick (or user
 *                                defined, for version 3 .z80 files)
 *                              3=Sinclair 2 Right joystick
 *
 * @see https://worldofspectrum.org/faq/reference/z80format.htm
 */
struct SnapZ80Header {
    constexpr static const uint8_t FLAGS_R_BIT7             = D0;
    constexpr static const uint8_t FLAGS_BORDER_MASK        = D1 | D2 | D3;
    constexpr static const uint8_t FLAGS_BORDER_SHIFT       = 1;
    constexpr static const uint8_t FLAGS_SAMROM             = D4;
    constexpr static const uint8_t FLAGS_DATA_COMPRESSED    = D5;

    constexpr static const uint8_t PORT_IMODE_MASK          = D0 | D1;
    constexpr static const uint8_t PORT_ISSUE_2_EMULATION   = D2;
    constexpr static const uint8_t PORT_2_INTERRUPT_FREQ    = D3;
    constexpr static const uint8_t PORT_VIDEO_SYNC_MASK     = D4 | D5;
    constexpr static const uint8_t PORT_JOY_MASK            = D6 | D7;

    constexpr static const uint8_t PORT_JOY_CURSOR          = 0x00;
    constexpr static const uint8_t PORT_JOY_KEMPSTON        = D4;
    constexpr static const uint8_t PORT_JOY_I2_LEFT         = D5;
    constexpr static const uint8_t PORT_JOY_I2_RIGHT        = D4 | D5;

    uint8_t     A{};                        /* 00 */
    uint8_t     F{};                        /* 01 */
    uint8_t     C{};                        /* 02 */
    uint8_t     B{};                        /* 03 */
    uint8_t     L{};                        /* 04 */
    uint8_t     H{};                        /* 05 */
    uint8_t     PCl{};                      /* 06 */
    uint8_t     PCh{};                      /* 07 */
    uint8_t     SPl{};                      /* 08 */
    uint8_t     SPh{};                      /* 09 */
    uint8_t     I{};                        /* 10 */
    uint8_t     R{};                        /* 11 */
    uint8_t     flags;                      /* 12 */
    uint8_t     E{};                        /* 13 */
    uint8_t     D{};                        /* 14 */
    uint8_t     aC{};                       /* 15 */
    uint8_t     aB{};                       /* 16 */
    uint8_t     aE{};                       /* 17 */
    uint8_t     aD{};                       /* 18 */
    uint8_t     aL{};                       /* 19 */
    uint8_t     aH{};                       /* 20 */
    uint8_t     aA{};                       /* 21 */
    uint8_t     aF{};                       /* 22 */
    uint8_t     IYl{};                      /* 23 */
    uint8_t     IYh{};                      /* 24 */
    uint8_t     IXl{};                      /* 25 */
    uint8_t     IXh{};                      /* 26 */
    uint8_t     IFF1{};                     /* 27 */
    uint8_t     IFF2{};                     /* 28 */
    uint8_t     port;                       /* 29 */
} __attribute__((packed));

/**
 * Z80 snapshot file format header (versions 2 and 3).
 *
 *     Offset  Length  Description
 *     ---------------------------------------------------------------------
 *      *30    2       Length of additional header block (see below)
 *      *32    2       Program counter
 *      *34    1       Hardware mode (see below)
 *      *35    1       If in SamRam mode, bitwise state of 74ls259.
 *                     For example, bit 6=1 after an OUT 31,13 (=2*6+1)
 *                     If in 128 mode, contains last OUT to 0x7ffd
 *                     If in Timex mode, contains last OUT to 0xf4
 *      *36    1       Contains 0xff if Interface I rom paged
 *                     If in Timex mode, contains last OUT to 0xff
 *      *37    1       Bit 0: 1 if R register emulation on
 *                     Bit 1: 1 if LDIR emulation on
 *                     Bit 2: AY sound in use, even on 48K machines
 *                     Bit 6: (if bit 2 set) Fuller Audio Box emulation
 *                     Bit 7: Modify hardware (see below)
 *      *38    1       Last OUT to port 0xfffd (soundchip register number)
 *      *39    16      Contents of the sound chip registers
 *       55    2       Low T state counter
 *       57    1       Hi T state counter
 *       58    1       Flag byte used by Spectator (QL spec. emulator)
 *                     Ignored by Z80 when loading, zero when saving
 *       59    1       0xff if MGT Rom paged
 *       60    1       0xff if Multiface Rom paged. Should always be 0.
 *       61    1       0xff if 0-8191 is ROM, 0 if RAM
 *       62    1       0xff if 8192-16383 is ROM, 0 if RAM
 *       63    10      5 x keyboard mappings for user defined joystick
 *       73    10      5 x ASCII word: keys corresponding to mappings above
 *       83    1       MGT type: 0=Disciple+Epson,1=Disciple+HP,16=Plus D
 *       84    1       Disciple inhibit button status: 0=out, 0ff=in
 *       85    1       Disciple inhibit flag: 0=rom pageable, 0ff=not
 *     **86    1       Last OUT to port 0x1ffd
 *
 * @see https://worldofspectrum.org/faq/reference/z80format.htm
 */
struct SnapZ80HeaderV23 : public SnapZ80Header {
    constexpr static const uint16_t VERSION_V2  = 23 + 2;
    constexpr static const uint16_t VERSION_V3  = 54 + 2;
    constexpr static const uint16_t VERSION_V3b = 55 + 2;
    constexpr static const uint8_t HW_MODE_48K  = 0x00;

    /* Versions 2 and 3x */
    uint8_t     sizel;                      /* 30 */
    uint8_t     sizeh;                      /* 31 */
    uint8_t     PCl_v2;                     /* 32 */
    uint8_t     PCh_v2;                     /* 33 */
    uint8_t     hw_mode;                    /* 34 */
    uint8_t     last_out;                   /* 35 */
    uint8_t     I_rom_paged;                /* 36 */
    uint8_t     flags_v2;                   /* 38 */
    uint8_t     last_out_fffd;              /* 38 */
    uint8_t     sound_regs[16];             /* 39 */

    /* Version 3x */
    uint16_t    T_statel;                   /* 55 */
    uint8_t     T_stateh;                   /* 57 */
    uint8_t     spectator_flag;             /* 58 */
    uint8_t     mgt_rom_paged;              /* 59 */
    uint8_t     mul_rom_paged;              /* 60 */
    uint8_t     rom_at_0000;                /* 61 */
    uint8_t     rom_at_8192;                /* 62 */
    uint8_t     joystick_mappings[10];      /* 63 */
    uint8_t     key_joystick_mappings[10];  /* 73 */
    uint8_t     mgt_type;                   /* 83 */
    uint8_t     disciple_button;            /* 84 */
    uint8_t     disciple_flag;              /* 85 */

    /* Version 3b */
    uint8_t     last_out_1ffd;              /* 86 */
} __attribute__((packed));

/**
 * SnapZ80 memory block (versions 2 and 3).
 *
 *     Byte    Length  Description
 *     ------------------------------------------------------------------------------
 *     0       2       Length of compressed data (without this 3-byte header)
 *                     If length=0xffff, data is 16384 bytes long and not compressed
 *     2       1       Page number of block
 *     3       [0]     Data/
 *
 *     Page    In '48 mode     In '128 mode    In SamRam mode
 *     ------------------------------------------------------------------------------
 *      0      48K rom         rom (basic)     48K rom
 *      1      Interface I, Disciple or Plus D rom, according to setting
 *      2      -               rom (reset)     samram rom (basic)
 *      3      -               page 0          samram rom (monitor,..)
 *      4      8000-bfff       page 1          Normal 8000-bfff
 *      5      c000-ffff       page 2          Normal c000-ffff
 *      6      -               page 3          Shadow 8000-bfff
 *      7      -               page 4          Shadow c000-ffff
 *      8      4000-7fff       page 5          4000-7fff
 *      9      -               page 6          -
 *     10      -               page 7          -
 *     11      Multiface rom   Multiface rom   -
 *
 * @see https://worldofspectrum.org/faq/reference/z80format.htm
 */
struct SnapZ80Block {
    constexpr static const uint8_t PAGE_48K_RAM_8000        = 0x04;
    constexpr static const uint8_t PAGE_48K_RAM_C000        = 0x05;
    constexpr static const uint8_t PAGE_48K_SHADOW_RAM_8000 = 0x06;
    constexpr static const uint8_t PAGE_48K_SHADOW_RAM_C000 = 0x07;
    constexpr static const uint8_t PAGE_48K_RAM_4000        = 0x08;
    constexpr static const uint16_t UNCOMPRESSED_16K_BLOCK  = 0xFFFF;
    constexpr static const size_t UNCOMPRESSED_BLOCK_SIZE   = 0x4000;

    uint8_t sizel;
    uint8_t sizeh;
    uint8_t page;
    uint8_t data[0];

} __attribute__((packed));

/**
 * Z80 snapshot file.
 * @see SnapZ80Header
 * @see SnapZ80HeaderV23
 * @see SnapZ80BlockV23
 * @see https://worldofspectrum.org/faq/reference/z80format.htm
 */
class SnapZ80 : public Snapshot {
public:
    constexpr static const char* FILE_EXTENSION = ".z80";
    constexpr static const addr_t MAX_DATA_SIZE = 0xC000;

    /**
     * Load a Z80 file.
     * @param fname File name.
     * @exception IOError
     * @see load(const std::string&)
     */
    SnapZ80(const std::string& fname);

    virtual ~SnapZ80();

    /**
     * Detect if a specified file uses the Z80 format.
     * The Z80 format does not have any magic number or other specific
     * signature, this method checks only the file name extension.
     * @param fname File name to check.
     * @return true if the specified file seems to be a Z80 formatted snapshot; false otherwise.
     * @see FILE_EXTENSION
     */
    static bool seems_like(const std::string& fname);

private:
    enum class Version {
        v1,
        v2,
        v3,
        v3b
    };

    void load(const std::string& fname);
    void load_v1(const buffer_t& raw);
    void load_v2(const buffer_t& raw);
    void uncompress_v1(const buffer_t& raw);
    void uncompress_v2(const buffer_t& raw, size_t rawoff);
    buffer_t uncompress(const gsl::span<const uint8_t>& enc, bool v1);

    void throw_ioerror(const std::string& reason = {}) const;

    static Z80::Registers extract_registers(const SnapZ80HeaderV23* hdr);
    static intflags_t extract_intflags(const SnapZ80HeaderV23* hdr);
    static uint8_t extract_border_colour(const SnapZ80HeaderV23* hdr);
    static const SnapZ80HeaderV23* header(const buffer_t& raw);
};

}
}
}
