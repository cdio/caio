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

#include "fs.hpp"
#include "types.hpp"

#include <fstream>
#include <string>
#include <utility>

namespace caio {
namespace nintendo {
namespace nes {
namespace iNES {

/**
 * iNES file format header.
 * Content of a *.nes* file:
 * - Header (16 bytes)
 * - Trainer, if present (0 or 512 bytes)
 * - PRG ROM data (16384 * x bytes)
 * - CHR ROM data, if present (8192 * y bytes)
 * - PlayChoice INST-ROM, if present (0 or 8192 bytes)
 * - PlayChoice PROM, if present (16 bytes Data, 16 bytes CounterOut)
 * - Some ROM-Images additionally contain a 128-byte (or sometimes 127-byte) title at the end of the file.
 */
struct Header {
    constexpr static const char* HDR_INES_SIGNATURE             = "NES\x1A";
    constexpr static const uint8_t HDR_6_NT_HORIZ_ARRANGEMENT   = 0x01;
    constexpr static const uint8_t HDR_6_PERSISTENT_RAM         = 0x02;
    constexpr static const uint8_t HDR_6_TRAINER                = 0x04;
    constexpr static const uint8_t HDR_6_ALTERNATIVE_NAMETABLE  = 0x08;
    constexpr static const uint8_t HDR_7_UNISYSTEM              = 0x01;
    constexpr static const uint8_t HDR_7_PLAYCHOICE             = 0x02;
    constexpr static const uint8_t HDR_7_NES_V20_FORMAT         = 0x0C;
    constexpr static const uint8_t HDR_7_MAPPER_MASK            = 0xF0;
    constexpr static const uint8_t HDR_9_TV_PAL                 = 0x01;
    constexpr static const size_t TRAINER_SIZE                  = 512;

    uint8_t sign[4];        /* "NES\x1A"                        */
    uint8_t prgsize;        /* Size of PRG ROM in 16KB blocks   */
    uint8_t chrsize;        /* Size of CHR ROM in 8KB blocks    */
    uint8_t flags_6;
    uint8_t flags_7;
    uint8_t flags_8;
    uint8_t flags_9;
    uint8_t pad[6];

    size_t prg_size() const
    {
        return (prgsize * 16384);
    }

    size_t chr_size() const
    {
        return (chrsize * 8192);
    }

    bool is_ines() const
    {
        return (std::memcmp(sign, HDR_INES_SIGNATURE, sizeof(sign)) == 0);
    }

    bool vertical_mirror() const
    {
        /* Horizontal arrangement = Vertical mirroring */
        return (flags_6 & HDR_6_NT_HORIZ_ARRANGEMENT);
    }

    bool persistent_ram() const
    {
        return (flags_6 & HDR_6_PERSISTENT_RAM);
    }

    bool trainer() const
    {
        return (flags_6 & HDR_6_TRAINER);
    }

    bool alternative_nametable() const
    {
        return (flags_6 & HDR_6_ALTERNATIVE_NAMETABLE);
    }

    bool unisystem() const
    {
        return (flags_7 & HDR_7_UNISYSTEM);
    }

    bool playchoice() const
    {
        return (flags_7 & HDR_7_PLAYCHOICE);
    }

    bool is_v20() const
    {
        return ((flags_7 & HDR_7_NES_V20_FORMAT) == 0x08);
    }

    size_t mapper() const
    {
        return ((flags_7 & HDR_7_MAPPER_MASK) | (flags_6 >> 4));
    }

    size_t prg_ram_size() const
    {
        /* iNES version < 2.0, size of prg ram in 8K blocks */
        return ((flags_8 == 0 ? 1 : flags_8) * 8192);
    }

    bool tv_pal() const
    {
        return (flags_9 & HDR_9_TV_PAL);
    }
} __attribute__((packed));

/**
 * Load the header of an iNES cartridge.
 * @param fname Cartridge file to load.
 * @return The iNES header and an input stream pointing to the begining of the PRG data.
 * @exception InvalidCartridge
 * @see Header
 */
std::pair<Header, std::ifstream> load_header(const fs::Path& fname);

/**
 * Get a human readable string representation of an iNES header.
 * @param hdr Header.
 * @return A string representation of the specified header.
 */
std::string to_string(const Header& hdr);

/**
 * Get the signature of an iNES header.
 * @return The signature as a string.
 */
std::string signature(const Header& hdr);

}
}
}
}
