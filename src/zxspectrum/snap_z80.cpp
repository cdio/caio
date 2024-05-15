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
#include "snap_z80.hpp"

#include "fs.hpp"
#include "utils.hpp"

#include "zxsp_params.hpp"

namespace caio {
namespace sinclair {
namespace zxspectrum {

SnapZ80::SnapZ80(const std::string& fname)
    : Snapshot{}
{
    load(fname);
}

SnapZ80::~SnapZ80()
{
}

bool SnapZ80::seems_like(const std::string& fname)
{
    auto fullpath = fs::fix_home(fname);
    auto lowcase = caio::tolow(fname);
    return (fs::exists(fullpath) && lowcase.ends_with(FILE_EXTENSION));
}

void SnapZ80::load(const std::string& fname)
{
    _fname = fs::fix_home(fname);

    log.debug("SnapZ80: Loading snapshot file: {}\n", _fname);

    const auto raw = fs::load(_fname);

    if (raw.size() < sizeof(SnapZ80Header)) {
        throw_ioerror();
    }

    const auto* hdr = header(raw);

    if (hdr->flags == 255) {
        /* WTF */
        const_cast<SnapZ80HeaderV23*>(hdr)->flags = 1;
    }

    if (hdr->PCl != 0x00 || hdr->PCh != 0x00) {
        load_v1(raw);
    } else {
        load_v2(raw);
    }

    _regs = extract_registers(hdr);
    _intflags = extract_intflags(hdr);
    _border_colour = extract_border_colour(hdr);
}

void SnapZ80::load_v1(const buffer_t& raw)
{
    log.debug("SnapZ80: {}: Detected version: 1\n", _fname);
    uncompress_v1(raw);
}

void SnapZ80::load_v2(const buffer_t& raw)
{
    const auto* hdr = header(raw);
    const char* ver{};

    /*
     * The specs says that sizeh and sizel specify the extended header size,
     * but actually it seems to be that size minus 2.
     */
    auto ext_size = ((hdr->sizeh << 8) | hdr->sizel) + 2;
    if (raw.size() < ext_size) {
        throw_ioerror("Invalid version 23b. Size: " + std::to_string(ext_size));
    }

    switch (ext_size) {
    case SnapZ80HeaderV23::VERSION_V2:
        ver = "2";
        break;
    case SnapZ80HeaderV23::VERSION_V3:
        ver = "3";
        break;
    case SnapZ80HeaderV23::VERSION_V3b:
        ver = "3b";
        break;
    default:
        throw_ioerror("Invalid version 23b. Size: " + std::to_string(ext_size));
    }

    log.debug("SnapZ80: {}: Detected version: {}\n", _fname, ver);

    /*
     * Only plain 48K is supported (no extra hardware).
     */
    if (hdr->hw_mode != SnapZ80HeaderV23::HW_MODE_48K) {
        throw_ioerror("Hardware mode not supported: $" + caio::to_string(hdr->hw_mode));
    }

    if (hdr->I_rom_paged != 0x00) {
        throw_ioerror("Interface I ROM not supported");
    }

    if (*ver != '2') {
        if (hdr->mgt_rom_paged != 0x00) {
            throw_ioerror("MGT ROM not supported");
        }

        if (hdr->mul_rom_paged != 0x00) {
            throw_ioerror("Multiface ROM not supported");
        }

        if (hdr->rom_at_0000 != 0xFF || hdr->rom_at_8192 != 0xFF) {
            throw_ioerror("RAM at $0000-$3FFF not supported");
        }
    }

    uncompress_v2(raw, sizeof(SnapZ80Header) + ext_size);
}

SnapZ80::buffer_t SnapZ80::uncompress(const std::span<const uint8_t>& enc, bool endmark)
{
    buffer_t dst{};
    size_t i = 0;

    while (i < enc.size() - 4) {
        uint8_t e1 = enc[i + 0];
        uint8_t e2 = enc[i + 1];
        uint8_t e3 = enc[i + 2];
        uint8_t e4 = enc[i + 3];

        if (endmark && e1 == 0x00 && e2 == 0xED && e3 == 0xED && e4 == 0x00) {
            return dst;
        }

        if (e1 == 0xED && e2 == 0xED) {
            /*
             * Encoded sequence: ED ED e3 e4
             * e3 = count
             * e4 = byte
             */
            while (e3--) {
                dst.push_back(e4);
            }

            i += 4;

        } else {
            /*
             * Unencoded.
             */
            dst.push_back(e1);
            ++i;
        }
    }

    /* Last unencoded bytes */
    std::copy(enc.begin() + i, enc.end(), std::back_inserter(dst));

    return dst;
}

void SnapZ80::uncompress_v1(const buffer_t& raw)
{
    const auto* hdr = header(raw);

    size_t offset = sizeof(SnapZ80Header);
    size_t size = raw.size() - offset;
    const std::span<const uint8_t> enc = { raw.data() + offset, size };

    bool compressed = (hdr->flags & SnapZ80Header::FLAGS_DATA_COMPRESSED);

    log.debug("SnapZ80: {}: compressed: {}\n", _fname, compressed);

    if (compressed) {
        _data = uncompress(enc, true);
    } else {
        std::copy(enc.begin(), enc.end(), std::back_inserter(_data));
    }
}

void SnapZ80::uncompress_v2(const buffer_t& raw, size_t rawoff)
{
    while (rawoff < raw.size()) {
        const auto* block = reinterpret_cast<const SnapZ80Block*>(raw.data() + rawoff);
        uint16_t enc_size = (block->sizeh << 8) | block->sizel;
        bool compressed = (enc_size != SnapZ80Block::UNCOMPRESSED_16K_BLOCK);

        log.debug("SnapZ80: {}: Block: page: {}, encsiz: {}, compressed: {}\n", _fname, block->page, enc_size,
            compressed);

        size_t dstoff{};

        switch (block->page) {
        case SnapZ80Block::PAGE_48K_RAM_4000:
            dstoff = 0x4000 - RAM_BASE_ADDRESS;
            break;

        case SnapZ80Block::PAGE_48K_RAM_8000:
        case SnapZ80Block::PAGE_48K_SHADOW_RAM_8000:
            dstoff = 0x8000 - RAM_BASE_ADDRESS;
            break;

        case SnapZ80Block::PAGE_48K_RAM_C000:
        case SnapZ80Block::PAGE_48K_SHADOW_RAM_C000:
            dstoff = 0xC000 - RAM_BASE_ADDRESS;
            break;

        default:
            throw_ioerror("Block page not supported: $" + caio::to_string(block->page));
        }

        rawoff += sizeof(*block);

        auto max_dec_size = dstoff + SnapZ80Block::UNCOMPRESSED_BLOCK_SIZE;
        if (_data.size() < max_dec_size) {
            _data.resize(_data.size() + max_dec_size);
        }

        auto it = _data.begin() + dstoff;

        if (compressed) {
            std::span<const uint8_t> enc{block->data, enc_size};
            auto dec = uncompress(enc, false);

            if (dec.size() > SnapZ80Block::UNCOMPRESSED_BLOCK_SIZE) {
                throw_ioerror("Unexpected uncompressed block size: " + std::to_string(dec.size()));
            }

            std::copy(dec.begin(), dec.end(), it);
            rawoff += enc_size;

        } else {
            std::copy_n(block->data, SnapZ80Block::UNCOMPRESSED_BLOCK_SIZE, it);
            rawoff += SnapZ80Block::UNCOMPRESSED_BLOCK_SIZE;
        }
    }
}

inline Z80::Registers SnapZ80::extract_registers(const SnapZ80HeaderV23* hdr)
{
    Z80::Registers r{
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
        .R   = static_cast<uint8_t>((hdr->R & 0x7F) | ((hdr->flags & SnapZ80Header::FLAGS_R_BIT7) << 7)),
        .SP  = static_cast<uint16_t>((hdr->SPh << 8) | hdr->SPl),
        .PC  = static_cast<uint16_t>((hdr->PCh << 8) | hdr->PCl)
    };

    if (r.PC == 0x0000) {
        /*
         * Z80 snapshot versions 2 or 3.
         */
        r.PC  = static_cast<uint16_t>((hdr->PCh_v2 << 8) | hdr->PCl_v2);
    }

    return r;
}

inline SnapZ80::intflags_t SnapZ80::extract_intflags(const SnapZ80HeaderV23* hdr)
{
    return {
        static_cast<Z80::IMode>(hdr->port & SnapZ80Header::PORT_IMODE_MASK),
        hdr->IFF1,
        hdr->IFF2
    };
}

inline uint8_t SnapZ80::extract_border_colour(const SnapZ80HeaderV23* hdr)
{
    return static_cast<uint8_t>((hdr->flags & SnapZ80Header::FLAGS_BORDER_MASK) >> SnapZ80Header::FLAGS_BORDER_SHIFT);
}

inline const SnapZ80HeaderV23* SnapZ80::header(const buffer_t& raw)
{
    return reinterpret_cast<const SnapZ80HeaderV23*>(raw.data());
}

inline void SnapZ80::throw_ioerror(const std::string& reason) const
{
    Snapshot::throw_ioerror("Z80", reason);
}

}
}
}
