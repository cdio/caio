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
#include "c64_crt.hpp"

#include "endian.hpp"
#include "ram.hpp"
#include "rom.hpp"
#include "utils.hpp"

namespace caio {
namespace commodore {
namespace c64 {

void Crt::open(const fs::Path& fname)
{
    std::ifstream is{fname, std::ios_base::binary | std::ios_base::in};
    if (!is) {
        throw InvalidCartridge{"Can't open: {}: {}", fname.string(), Error::to_string()};
    }

    _hdr   = {};
    _chips = {};
    _fname = fname;

    open(is);
}

void Crt::open(std::istream& is)
{
    try {
        load_header(is, _hdr);
        if (!is_valid(_hdr)) {
            throw Error{"Invalid CRT header"};
        }

        for (size_t i = 0;; ++i) {
            Chip ch{};
            try {
                load_chip(is, ch);
            } catch (const IOError&) {
                /* No more CHIPS to read */
                break;
            }

            if (!is_valid(ch)) {
                throw Error{"Invalid CHIP section on CRT header"};
            }

            switch (ch.type) {
            case CHIP_TYPE_ROM:
            case CHIP_TYPE_FLASH:
            case CHIP_TYPE_EEPROM: {
                auto rom = load_rom(is, ch);
                rom->label(name() + ", chip " + std::to_string(i + 1));
                _roms.push_back(rom);
                _chips.push_back(ch);
                break;
            }

            case CHIP_TYPE_RAM: {
                auto ram = std::make_shared<RAM>(+ch.rsiz, "");
                ram->label(name() + ", chip " + std::to_string(i + 1));
                _roms.push_back(ram);
                _chips.push_back(ch);
                break;
            }

            default:
                throw Error{"Invalid CHIP type: ${}", utils::to_string(ch.type)};
            }
        }

    } catch (const std::exception& err) {
        throw InvalidCartridge{"{}: {}", _fname.string(), err.what()};
    }
}

std::pair<const Crt::Chip&, devptr_t> Crt::operator[](size_t n) const
{
    if (n > _chips.size()) {
        throw InvalidArgument{};
    }

    return {_chips[n], _roms[n]};
}

std::string Crt::to_string() const
{
    std::stringstream ss{};

    ss << "CRT " << std::quoted(_fname.string())
       << ", " << to_string(_hdr);

    for (const auto& ch : _chips) {
        ss << ", chip(" <<  to_string(ch) << ")";
    }

    return ss.str();
}

std::string Crt::name() const
{
    return std::string{reinterpret_cast<const char*>(_hdr.name),
        std::strlen(reinterpret_cast<const char*>(_hdr.name))};
}

bool Crt::is_crt(const fs::Path& fname)
{
    try {
        std::ifstream is{fname, std::ios_base::binary | std::ios_base::in};
        if (is) {
            Header hdr{};
            load_header(is, hdr);
            return is_valid(hdr);
        }
    } catch (const std::exception&) {
    }

    return false;
}

bool Crt::is_valid(const Crt::Header& hdr)
{
    std::string sign{reinterpret_cast<const char*>(hdr.sign), sizeof(hdr.sign)};

    return (sign == HDRSIGN && hdr.size >= HDRMINSIZ);
}

bool Crt::is_valid(const Crt::Chip& ch)
{
    std::string sign{reinterpret_cast<const char*>(ch.sign), sizeof(ch.sign)};

    return ((sign == CHIPSIGN) &&
        (ch.type == CHIP_TYPE_ROM || ch.type == CHIP_TYPE_RAM || ch.type == CHIP_TYPE_FLASH) &&
        (ch.size == sizeof(Chip) + ch.rsiz));
}

void Crt::load_header(std::istream& is, Header& hdr)
{
    if (!is.read(reinterpret_cast<char*>(&hdr), sizeof(hdr))) {
        throw IOError{"Can't read CRT header: {}", Error::to_string()};
    }

    to_host(hdr);
}

void Crt::load_chip(std::istream& is, Chip& ch)
{
    if (!is.read(reinterpret_cast<char*>(&ch), sizeof(ch))) {
        throw IOError{"Can't read CHIP header: {}", Error::to_string()};
    }

    to_host(ch);
}

devptr_t Crt::load_rom(std::istream& is, const Chip& ch)
{
    return std::make_shared<ROM>(is, ch.rsiz);
}

std::string Crt::to_string(const Crt::Header& hdr)
{
    std::string_view name{reinterpret_cast<const char*>(hdr.name), std::strlen(reinterpret_cast<const char*>(hdr.name))};
    return std::format("name \"{}\", size {}, hwtype ${:02X}, exrom {}, game {}",
        name, hdr.size, hdr.hwtype, hdr.exrom, hdr.game);
}

std::string Crt::to_string(const Crt::Chip& ch)
{
    return std::format("size {}, type ${}, bank ${}, addr ${}, rsiz {}",
        ch.size, utils::to_string(ch.type), utils::to_string(ch.bank), utils::to_string(ch.addr), ch.rsiz);
}

void Crt::to_host(Crt::Header& hdr)
{
    hdr.size    = be32toh(hdr.size);
    hdr.version = be16toh(hdr.version);
    hdr.hwtype  = be16toh(hdr.hwtype);
}

void Crt::to_host(Crt::Chip& ch)
{
    ch.size = be32toh(ch.size);
    ch.type = be16toh(ch.type);
    ch.bank = be16toh(ch.bank);
    ch.addr = be16toh(ch.addr);
    ch.rsiz = be16toh(ch.rsiz);
}

}
}
}
