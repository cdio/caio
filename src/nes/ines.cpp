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
#include "ines.hpp"

#include "utils.hpp"

#include <format>

namespace caio {
namespace nintendo {
namespace nes {
namespace iNES {

std::pair<Header, std::ifstream> load_header(const fs::Path& fname)
{
    if (fname.empty()) {
        throw InvalidCartridge{"Can't open cartridge file: Empty file name"};
    }

    std::ifstream is{fname};
    if (!is) {
        throw InvalidCartridge{"Can't open cartridge file: {}: {}", fname.string(), Error::to_string()};
    }

    Header hdr{};

    try {
        is.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));
    } catch (const std::exception& err) {
        throw InvalidCartridge{"Can't load cartridge header: {}: {}", fname.string(), err.what()};
    }

    if (is.gcount() != sizeof(hdr)) {
        throw InvalidCartridge{"Invalid cartridge file: {}", fname.string()};
    }

    if (!hdr.is_ines()) {
        throw InvalidCartridge{"Invalid iNES signature: {}", fname.string()};
    }

    if (hdr.is_v20()) {
        throw InvalidCartridge{"iNES v2.0 not supported: {}", fname.string()};
    }

    if (hdr.trainer()) {
        /* Bypass trainer and put the file pointer at the begining of the PRG data */
        is.seekg(Header::TRAINER_SIZE, std::ios_base::cur);
    }

    return {std::move(hdr), std::move(is)};
}

std::string to_string(const Header& hdr)
{
    return std::format(
        "Mapper: {}, "
        "PRG RAM size: {}, "
        "PRG size: {}, "
        "CHR size: {}, "
        "Nametable Mirroring: {}, "
        "Presistent RAM: {}, "
        "Trainer: {}, "
        "Alt Nametable: {}, "
        "Unisystem: {}, "
        "Playchoice: {}, "
        "TV System: {}, "
        "Version 2.0: {}",
        hdr.mapper(),
        hdr.prg_ram_size(),
        hdr.prg_size(),
        hdr.chr_size(),
        (hdr.vertical_mirror() ? "Vertical" : "Horizontal"),
        (hdr.persistent_ram() ? "Yes" : "No"),
        (hdr.trainer() ? "Yes" : "No"),
        (hdr.alternative_nametable() ? "Yes" : "No"),
        (hdr.unisystem() ? "Yes" : "No"),
        (hdr.playchoice() ? "Yes" : "No"),
        (hdr.tv_pal() ? "PAL" : "NTSC"),
        (hdr.is_v20() ? "Yes" : "No"));
}

std::string signature(const Header& hdr)
{
    const auto buf = std::span<const uint8_t>{reinterpret_cast<const uint8_t*>(&hdr), sizeof(hdr)};
    return utils::sha256(buf);
}

}
}
}
}
