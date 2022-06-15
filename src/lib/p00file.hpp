/*
 * Copyright (C) 2020-2022 Claudio Castiglia
 *
 * This file is part of CAIO.
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

#include <cstdint>
#include <string>

#include "prgfile.hpp"
#include "types.hpp"


namespace caio {

/**
 * PC64 P00 file.
 * @see https://ist.uwaterloo.ca/~schepers/formats/PC64.TXT
 */
class P00File : public PrgFile {
public:
    constexpr static const uint64_t P00_MAGIC = 0x43363446696C6500; /* C64File\0 */

    struct P00Header {
        uint64_t magic;         /* C64File (big endian on file, host order on memory)   */
        uint8_t  fname[17];     /* Original File name (PETASCII)                        */
        uint8_t  rsize;         /* REL file record size (0 if not a REL file)           */
        /* data follows */
    } __attribute__((packed));


    /**
     * Load a P00 file.
     * @param fname If defined, name of the P00 file to load.
     * @exception IOError
     * @see load(const std::string &)
     */
    P00File(const std::string &fname = {});

    virtual ~P00File();

    /**
     * @return The raw header of this P00 file.
     */
    const P00Header &header() const;

    /**
     * Load a P00 file.
     * Any previous content of this P00 is replaced with the new data.
     * @param fname Name of the P00 file to load.
     * @exception IOError
     */
    void load(const std::string &fname) override;

    /**
     * Save this P00 file.
     * @param fname Name of the PRG file to save;
     * @param addr  If non-zero, use this value as the P00's start address ignoring PrgFile::address().
     * @exception IOError
     */
    void save(const std::string &fname, addr_t addr = 0) override;

private:
    P00Header _hdr{};
};

}