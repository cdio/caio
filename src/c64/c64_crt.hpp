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

#include <fstream>
#include <vector>

#include "types.hpp"
#include "device.hpp"

namespace caio {
namespace commodore {
namespace c64 {

/**
 * C64 CRT files.
 * CRT is a simple file format that holds information
 * about cartridges and the several chips inside.
 *
 * @see https://ist.uwaterloo.ca/~schepers/formats/CRT.TXT
 * @see https://vice-emu.sourceforge.io/vice_17.html#SEC391
 */
class Crt {
public:
    constexpr static const char* HDRSIGN   = "C64 CARTRIDGE   ";
    constexpr static const char* CHIPSIGN  = "CHIP";
    constexpr static uint32_t    HDRMINSIZ = 0x40;

    /**
     * Header of a CRT file.
     * Values stored in the binary file are big endian.
     */
    struct Header {
        uint8_t  sign[16];          /* "C64 CARTRIDGE   "           */
        uint32_t size;              /* Header size (>= $40)         */
        uint16_t version;           /* Cartridge version            */
        uint16_t hwtype;            /* Cartrdige hardware type      */
        uint8_t  exrom;             /* EXROM line status            */
        uint8_t  game;              /* GAME line status             */
        uint8_t  reserved[6];
        uint8_t  name[32];          /* Cartridge's name             */
    } __attribute__((packed));

    /**
     * Header of a chip section of a CRT file.
     * Values stored in the binary file are big endian.
     */
    struct Chip {
        uint8_t  sign[4];           /* "CHIP"                       */
        uint32_t size;              /* Chip packet size             */
        uint16_t type;              /* Chip type                    */
        uint16_t bank;              /* Bank number ($0000 = normal) */
        uint16_t addr;              /* Starting load address        */
        uint16_t rsiz;              /* ROM size (bytes)             */
    } __attribute__((packed));

    /**
     * Chip Types.
     * @see Chip::type
     */
    enum ChipType {
        CHIP_TYPE_ROM    = 0,
        CHIP_TYPE_RAM    = 1,
        CHIP_TYPE_FLASH  = 2,
        CHIP_TYPE_EEPROM = 3
    };

    /**
     * Hardware types.
     * @see Header::hwtype
     */
    enum HardwareType {
        HW_TYPE_GENERIC                 = 0,
        HW_TYPE_ACTION_REPLAY           = 1,
        HW_TYPE_KCS_POWER_CARTRIDGE     = 2,
        HW_TYPE_FINAL_CARTRIDGE_III     = 3,
        HW_TYPE_SIMONS_BASIC            = 4,
        HW_TYPE_OCEAN_TYPE_1            = 5,
        HW_TYPE_EXPERT_CARTRIDGE        = 6,
        HW_TYPE_FUN_PLAY                = 7,
        HW_TYPE_SUPER_GAMES             = 8,
        HW_TYPE_ATOMIC_POWER            = 9,

        HW_TYPE_EPYX_FASTLOAD           = 10,
        HW_TYPE_WESTERMANN_LEARNING     = 11,
        HW_TYPE_REX_UTILITY             = 12,
        HW_TYPE_FINAL_CARTRIDGE_I       = 13,
        HW_TYPE_MAGIC_FORMEL            = 14,
        HW_TYPE_C64_GAME_SYSTEM_3       = 15,
        HW_TYPE_WARP_SPEED              = 16,
        HW_TYPE_DINAMIC                 = 17,
        HW_TYPE_ZAXXON                  = 18,
        HW_TYPE_MAGIC_DESK              = 19,

        HW_TYPE_SUPER_SNAPSHOT_V5       = 20,
        HW_TYPE_COMAL_80                = 21,
        HW_TYPE_STRUCTURED_BASIC        = 22,
        HW_TYPE_ROSS                    = 23,
        HW_TYPE_DELA_EP64               = 24,
        HW_TYPE_DELA_EP7x8              = 25,
        HW_TYPE_DELA_EP256              = 26,
        HW_TYPE_REX_EP256               = 27,
        HW_TYPE_MIKRO_ASSEMBLER         = 28,
        HW_TYPE_FINAL_CARTRIDGE_PLUS    = 29,

        HW_TYPE_ACTION_REPLAY_4         = 30,
        HW_TYPE_STARDOS                 = 31,
        HW_TYPE_EASY_FLASH              = 32,
        HW_TYPE_EASY_FLASH_XBANK        = 33,
        HW_TYPE_CAPTURE                 = 34,
        HW_TYPE_ACTION_REPLAY_3         = 35,
        HW_TYPE_RETRO_REPLAY            = 36,
        HW_TYPE_MMC_64                  = 37,
        HW_TYPE_MMC_REPLAY              = 38,
        HW_TYPE_IDE_64                  = 39,

        HW_TYPE_SUPER_SNAPSHOT_V4       = 40,
        HW_TYPE_IEEE_488                = 41,
        HW_TYPE_GAME_KILLER             = 42,
        HW_TYPE_PROPHET_64              = 43,
        HW_TYPE_EXOS                    = 44,
        HW_TYPE_FREEZE_FRAME            = 45,
        HW_TYPE_FREEZE_MACHINE          = 46,
        HW_TYPE_SNAPSHOT_64             = 47,
        HW_TYPE_SUPER_EXPLODE_V5_0      = 48,
        HW_TYPE_MAGIC_VOICE             = 49,

        HW_TYPE_ACTION_REPLAY_2         = 50,
        HW_TYPE_MACH_5                  = 51,
        HW_TYPE_DIASHOW_MAKER           = 52,
        HW_TYPE_PAGEFOX                 = 53,
        HW_TYPE_KINGSOFT                = 54,
        HW_TYPE_SILVERROCK_128K         = 55,
        HW_TYPE_FORMEL_64               = 56,
        HW_TYPE_RGCD                    = 57,
        HW_TYPE_RR_NET_MK3              = 58,
        HW_TYPE_EASY_CALC               = 59,

        HW_TYPE_GMOD_2                  = 60,
        HW_TYPE_MAX_BASIC               = 61,
        HW_TYPE_GMOD_3                  = 62,
        HW_TYPE_ZIPP_CODE_48            = 63,
        HW_TYPE_BLACKBOX_V8             = 64,
        HW_TYPE_BLACKBOX_V3             = 65,
        HW_TYPE_BLACKBOX_V4             = 66,
        HW_TYPE_REX_RAM_FLOPPY          = 67,
        HW_TYPE_BIS_PLUS                = 68,
        HW_TYPE_SD_BOX                  = 69,

        HW_TYPE_MULTIMAX                = 70,
        HW_TYPE_BLACKBOX_V9             = 71,
        HW_TYPE_LT_KERNAL_HOST_ADAPTOR  = 72,
        HW_TYPE_RAMLINK                 = 73,
        HW_TYPE_HERO                    = 74,
        HW_TYPE_IEEE_FLASH_64           = 75,
        HW_TYPE_TURTLE_GRAPHICS_II      = 76,
        HW_TYPE_FREEZE_FRAME_MK2        = 77
    };

    Crt() {
    }

    /**
     * Create a CRT instance.
     * @param fname Name of the cartridge file to open.
     * @exceptions InvalidCartridge
     * @see open(std::string_view)
     */
    Crt(std::string_view fname) {
        open(fname);
    }

    virtual ~Crt() {
    }

    /**
     * Open and read a CRT cartridge file.
     * The previous content of this instance is lost even on error conditions.
     * @param fname Name of the CRT cartridge file to open.
     * @exception InvalidCartridge
     */
    void open(std::string_view fname);

    /**
     * Get the hardware type of this CRT file.
     * @return The hardware type of this CRT file.
     */
    const HardwareType type() const {
        return static_cast<HardwareType>(_hdr.hwtype);
    }

    /**
     * Get the status of the EXROM pin of this CRT.
     * @return The status of the EXROM pin.
     */
    bool exrom() const {
        return _hdr.exrom;
    }

    /**
     * Get the status of the GAME pin of this CRT.
     * @return The status of the GAME pin.
     */
    bool game() const {
        return _hdr.game;
    }

    /**
     * Get the number of chips embedded in this CRT.
     * @return The number of chip sections in this CRT.
     */
    size_t chips() const {
        return _chips.size();
    }

    /**
     * Get a chip entry.
     * @param index The entry index.
     * @return A reference to the chip header and a pointer to its content.
     * @exception InvalidArgument If the specified index is not valid.
     */
    std::pair<const Chip&, devptr_t> operator[](size_t index) const;

    /**
     * Get a string describing this CRT.
     * @return A string with the description of this CRT.
     */
    std::string to_string() const;

    /**
     * Get the name of this CRT.
     * @return The name of this CRT.
     */
    std::string name() const;

    /**
     * Detect a CRT formatted file.
     * @param fname File name.
     * @return true if the specified file is a CRT file; false otherwise.
     */
    static bool is_crt(std::string_view fname);

    /**
     * Detect whether a CRT header is valid.
     * @param hdr CRT header.
     * @return true if valid; false otherwise.
     */
    static bool is_valid(const Header& hdr);

    /**
     * Detect whether a chip's header is valid.
     * @param ch Chip's header.
     * @return true if valid; false otherwise.
     */
    static bool is_valid(const Chip& ch);

    /**
     * Get a string describing a CRT's header.
     * @param hdr CRT's header.
     * @return A string with the description the specified CRT's header.
     */
    static std::string to_string(const Header& hdr);

    /**
     * Get a string describing a specific chip.
     * @param ch Chip.
     * @return A string with the description of the specified chip.
     */
    static std::string to_string(const Chip& ch);

private:
    /**
     * Open and load a CRT file.
     * @param is Input stream.
     * @exception InvalidCartridge
     */
    void open(std::istream& is);

    /**
     * Load a CRT header from an input stream.
     * The loaded data is translated to host endian order.
     * @param is  Input stream;
     * @param hdr Destination buffer.
     * @exception IOError
     */
    static void load_header(std::istream& is, Header& hdr);

    /**
     * Load a CRT chip section header from an input stream.
     * The loaded data is translated to host endian order.
     * @param is  Input stream;
     * @param hdr Destination buffer.
     * @exception IOError
     */
    static void load_chip(std::istream& is, Chip& hdr);

    /**
     * Load a CRT chip data from an input stream.
     * @param is Input stream;
     * @param ch Information about the data to read.
     * @return A pointer to a rom device with the loaded data.
     * @exception IOError
     */
    static devptr_t load_rom(std::istream& is, const Chip& ch);

    /**
     * Convert a CRT header to the host endianness.
     * @param hdr Header to convert.
     */
    static void to_host(Header& hdr);

    /**
     * Convert a chip header to the host endianness.
     * @param ch Header to convert.
     */
    static void to_host(Chip& ch);

    std::string           _fname{};
    Header                _hdr{};
    std::vector<Chip>     _chips{};
    std::vector<devptr_t> _roms{};
};

}
}
}
