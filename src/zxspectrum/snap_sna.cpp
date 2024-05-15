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
#include "snap_sna.hpp"

#include <iterator>

#include "fs.hpp"

#include "zxsp_params.hpp"

namespace caio {
namespace sinclair {
namespace zxspectrum {

SnapSNA::SnapSNA(const std::string& fname)
    : Snapshot{}
{
    load(fname);
}

SnapSNA::~SnapSNA()
{
}

bool SnapSNA::seems_like(const std::string& fname)
{
    auto fullpath = fs::fix_home(fname);
    auto lowcase = caio::tolow(fname);
    return (fs::file_size(fullpath) == FILE_SIZE && lowcase.ends_with(FILE_EXTENSION));
}

void SnapSNA::load(const std::string& fname)
{
    _fname = fs::fix_home(fname);

    log.debug("SnapSNA: Loading snapshot file: {}\n", _fname);

    auto raw = fs::load(_fname);

    if (raw.size() != FILE_SIZE) {
        throw_ioerror("SNA", "Invalid file size");
    }

    const auto* hdr = reinterpret_cast<const SnapSNAHeader*>(raw.data());

    _regs = Z80::Registers{
        .A   = hdr->A,
        .F   = hdr->F,
        .B   = hdr->B,
        .C   = hdr->C,
        .D   = hdr->D,
        .E   = hdr->E,
        .H   = hdr->H,
        .L   = hdr->L,
        .aA  = hdr->aA,
        .aF  = hdr->aF,
        .aB  = hdr->aB,
        .aC  = hdr->aC,
        .aD  = hdr->aD,
        .aE  = hdr->aE,
        .aH  = hdr->aH,
        .aL  = hdr->aL,
        .IXh = hdr->IXh,
        .IXl = hdr->IXl,
        .IYh = hdr->IYh,
        .IYl = hdr->IYl,
        .I   = hdr->I,
        .R   = hdr->R,
        .SP  = static_cast<uint16_t>((hdr->SPh << 8) | hdr->SPl),
        .PC  = htole16(ROM_RETN_ADDR)
    };

    uint8_t im = (hdr->im & 0x03);
    if (im > 2) {
        im = 2;
        log.warn("SnapSNA: {}: Invalid IM flag: ${:02x}. Set to ${:02X}\n", _fname, hdr->im, im);
    }

    bool iff2 = (hdr->IFF2 & SnapSNAHeader::IFF2_BIT);
    _intflags = { static_cast<Z80::IMode>(im), iff2, iff2 };

    _border_colour = (hdr->bd & 0x07);

    raw.erase(raw.begin(), raw.begin() + sizeof(*hdr));
    _data = std::move(raw);
}

}
}
}
