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
namespace zxspectrum {

constexpr static const char* SEC_ZXSPECTRUM     = "zxspectrum";

constexpr static const char* KEY_OTAPE          = "otape";
constexpr static const char* KEY_ITAPE          = "tape";
constexpr static const char* KEY_FASTLOAD       = "fastload";
constexpr static const char* KEY_SNAPSHOT       = "snap";

constexpr static const char* DEFAULT_OTAPE      = "./";
constexpr static const char* DEFAULT_ITAPE      = "";
constexpr static const char* DEFAULT_FASTLOAD   = "no";
constexpr static const char* DEFAULT_SNAPSHOT   = "";

class ZXSpectrumCmdline : public config::Cmdline {
public:
    using Cmdline::Cmdline;

    virtual ~ZXSpectrumCmdline() {
    }

    std::vector<config::Option> options() const override;
    std::string usage() const override;
    std::string sname() const override;
};

struct ZXSpectrumConfig : public config::Config {
    std::string otape{};
    std::string itape{};
    bool        fastload{};
    std::string snap{};

    ZXSpectrumConfig(config::Section& sec);

    virtual ~ZXSpectrumConfig() {
    }

    std::string to_string() const override;
};

}
}
}
