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

#include "config.hpp"


namespace caio {
namespace sinclair {
namespace zx80 {

constexpr static const unsigned CLOCK_FREQ       = 3250000;
constexpr static const uint64_t RAM_INIT_PATTERN = 0x0000000000000000ULL;

constexpr static const char* ROM_FNAME           = "zx80_rom.bin";
constexpr static const size_t ROM_SIZE           = 4096;

constexpr static const char* ROM8_FNAME          = "zx81_rom.bin";
constexpr static const size_t ROM8_SIZE          = 8192;

constexpr static const size_t INTERNAL_RAM_SIZE  = 1024;
constexpr static const size_t EXTERNAL_RAM_SIZE  = 16384;

constexpr static const char* SEC_ZX80            = "zx80";
constexpr static const char* KEY_RAM_16K         = "ram16";
constexpr static const char* KEY_ROM_8K          = "rom8";
constexpr static const char* KEY_PRGFILE         = "prg";

constexpr static const char* DEFAULT_RAM_16K     = "no";
constexpr static const char* DEFAULT_ROM_8K      = "no";
constexpr static const char* DEFAULT_PRGFILE     = "";

class ZX80Cmdline : public config::Cmdline {
public:
    using Cmdline::Cmdline;

    virtual ~ZX80Cmdline() {
    }

    std::vector<config::Option> options() const override;
    std::string usage() const override;
    std::string sname() const override;
};

struct ZX80Config : public config::Config {
    bool        ram16{};
    bool        rom8{};
    std::string prgfile{};

    ZX80Config(config::Section& sec);

    virtual ~ZX80Config() {
    }

    std::string to_string() const override;
};

}
}
}
