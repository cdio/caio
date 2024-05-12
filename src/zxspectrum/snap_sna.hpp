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
 * SNA snapshot format header.
 *     $00  I
 *     $01  HL'
 *     $03  DE'
 *     $05  BC'
 *     $07  AF'
 *     $09  HL
 *     $0B  DE
 *     $0D  BC
 *     $0F  IY
 *     $11  IX
 *     $13  IFF2    [Only bit 2 is defined: 1 for EI, 0 for DI]
 *     $14  R
 *     $15  AF
 *     $17  SP
 *     $19  Interrupt mode: 0, 1 or 2
 *     $1A  Border colour
 *
 * @see https://sinclair.wiki.zxnet.co.uk/wiki/SNA_format
 * @see https://worldofspectrum.org/faq/reference/formats.htm#SNA
 */
struct SnapSNAHeader {
    constexpr static uint8_t IFF2_BIT = D3;

    uint8_t     I{};
    uint8_t     aL{};
    uint8_t     aH{};
    uint8_t     aE{};
    uint8_t     aD{};
    uint8_t     aC{};
    uint8_t     aB{};
    uint8_t     aF{};
    uint8_t     aA{};
    uint8_t     L{};
    uint8_t     H{};
    uint8_t     E{};
    uint8_t     D{};
    uint8_t     C{};
    uint8_t     B{};
    uint8_t     IYl{};
    uint8_t     IYh{};
    uint8_t     IXl{};
    uint8_t     IXh{};
    uint8_t     IFF2{};
    uint8_t     R{};
    uint8_t     F{};
    uint8_t     A{};
    uint8_t     SPl{};
    uint8_t     SPh{};
    uint8_t     im{};
    uint8_t     bd{};
} __attribute__((packed));

/**
 * SNA snapshot file.
 * @see SnapSNAHeader
 * @see Snapshot
 * @see https://sinclair.wiki.zxnet.co.uk/wiki/SNA_format
 * @see https://worldofspectrum.org/faq/reference/formats.htm#SNA
 */
class SnapSNA : public Snapshot {
public:
    constexpr static const char* FILE_EXTENSION = ".sna";
    constexpr static const size_t FILE_SIZE     = 49179;

    /**
     * Load a SNA file.
     * @param fname File name.
     * @exception IOError
     * @see load(const std::string&)
     */
    SnapSNA(const std::string& fname);

    virtual ~SnapSNA();

    /**
     * Detect if a specified file uses the SNA format.
     * The SNA format does not have any magic number or other specific
     * signature, this method checks the file name extension and the file size.
     * @param fname File name to check.
     * @return true if the specified file seems to be a SNA formatted snapshot; false otherwise.
     * @see FILE_EXTENSION
     * @see FILE_SIZE
     */
    static bool seems_like(const std::string& fname);

private:
    void load(const std::string& fname);
};

}
}
}
