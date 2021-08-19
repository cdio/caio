/*
 * Copyright (C) 2020 Claudio Castiglia
 *
 * This file is part of CEMU.
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


namespace cemu {
namespace c64 {

/**
 * C64 CRT files.
 * CRT is a simple file format that holds information
 * about cartridges and the several chips inside.
 *
 * This implementation is based on information taken
 * from http://ist.uwaterloo.ca/~schepers/formats/CRT.TXT.
 */
class Crt {
public:
    constexpr static const char *HDRSIGN   = "C64 CARTRIDGE   ";
    constexpr static const char *CHIPSIGN  = "CHIP";
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


    enum ChipType {
        CHIP_TYPE_ROM = 0,
        CHIP_TYPE_RAM,
        CHIP_TYPE_FLASH
    };


    Crt() {
    }

    /**
     * Create a CRT instance.
     * @param fname Name of the cartridge file to open.
     * @exceptions InvalidCartridge
     * @see open(const std::string &)
     */
    Crt(const std::string &fname) {
        open(fname);
    }

    virtual ~Crt() {
    }

    /**
     * Open and read a CRT cartridge.
     * The previous content is lost even on error conditions.
     * @param fname Name of the CRT cartridge file to open.
     * @exception InvalidCartridge
     */
    void open(const std::string &fname);

    /**
     * @return The number of CHIP sections in this CRT.
     */
    size_t chips() const {
        return _chips.size();
    }

    /**
     * Get a CHIP entry.
     * @param index The entry index.
     * @return A reference to the chip header and a pointer to its content.
     * @exception InvalidArgument If the specified index is not valid.
     */
    std::pair<const Chip &, devptr_t> operator[](size_t index) const;

    /**
     * @return A human readable description of this CRT.
     */
    std::string to_string() const;

    /**
     * @return The name of this CRT.
     */
    std::string name() const {
        return std::string{reinterpret_cast<const char *>(_hdr.name),
            std::strlen(reinterpret_cast<const char *>(_hdr.name))};
    }

    /**
     * Detect a CRT formatted file.
     * @param fname File name.
     * @return true if the specified file is a CRT file; false otherwise.
     */
    static bool is_crt(const std::string &fname);

    /**
     * Detect whether a CRT header is valid.
     * @param hdr CRT header.
     * @return true if valid; false otherwise.
     */
    static bool is_valid(const Header &hdr);

    /**
     * Detect whether a CHIP header is valid.
     * @param ch CHIP header.
     * @return true if valid; false otherwise.
     */
    static bool is_valid(const Chip &ch);

    /**
     * @return A human readable description of a CRT header.
     */
    static std::string to_string(const Header &hdr);

    /**
     * @return A human readable description of a CHIP header.
     */
    static std::string to_string(const Chip &ch);

private:
    /**
     * Open and load a CRT file.
     * @param is Input stream.
     * @exception InvalidCartridge
     */
    void open(std::istream &is);

    /**
     * Load a CRT header from an input stream.
     * The loaded data is translated to host endian order.
     * @param is  Input stream;
     * @param hdr Destination buffer.
     * @exception IOError
     */
    static void load_header(std::istream &is, Header &hdr);

    /**
     * Load a CRT CHIP section header from an input stream.
     * The loaded data is translated to host endian order.
     * @param is  Input stream;
     * @param hdr Destination buffer.
     * @exception IOError
     */
    static void load_chip(std::istream &is, Chip &hdr);

    /**
     * Load a CRT CHIP data from an input stream.
     * @param is Input stream;
     * @param ch Information about the data to read.
     * @return A pointer to a rom device with the loaded data.
     * @exception IOError
     */
    static devptr_t load_rom(std::istream &is, const Chip &ch);

    /**
     * Convert a CRT header to the host endianness.
     * @param hdr Header to convert.
     */
    static void to_host(Header &hdr);

    /**
     * Convert a CHIP header to the host endianness.
     * @param ch Header to convert.
     */
    static void to_host(Chip &ch);

    std::string           _fname{};
    Header                _hdr{};
    std::vector<Chip>     _chips{};
    std::vector<devptr_t> _roms{};
};

}
}
