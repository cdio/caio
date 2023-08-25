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
namespace c64 {

/**
 * C64 configuation file.
 */
class C64Confile : public CaioConfile {
public:
    constexpr static const char* C64_CONFIG_SECTION      = "c64";
    constexpr static const char* C64_CARTFILE_CONFIG_KEY = "cart";
    constexpr static const char* C64_PRGFILE_CONFIG_KEY  = "prg";
    constexpr static const char* C64_RESID_CONFIG_KEY    = "resid";
    constexpr static const char* C64_SWAPJOY_CONFIG_KEY  = "swapj";
    constexpr static const char* C64_UNIT_8_CONFIG_KEY   = "unit8";
    constexpr static const char* C64_UNIT_9_CONFIG_KEY   = "unit9";

    C64Confile() {
    }

    C64Confile(Confile&& other) {
        *this = std::move(other);
    }

    virtual ~C64Confile() {
    }

    C64Confile& operator=(Confile&& other) {
        static_cast<Confile&>(*this) = std::move(other);
        return *this;
    }
};

/**
 *  C64 configuration.
 */
struct C64Config : public Config {
    std::string cartfile{};
    std::string prgfile{};
    bool        resid{};
    bool        swapj{};
    std::string unit8{};
    std::string unit9{};

    C64Config() {
    }

    C64Config(const Confile& conf) {
        *this = conf;
    }

    virtual ~C64Config() {
    }

    /**
     * @see Config::operator=(const Confile&)
     */
    C64Config& operator=(const Confile& conf);

    /**
     * @see Config::palette_file()
     */
    std::string palette_file(const std::string& palette) const override;

    /**
     * @see Config::keymaps_file()
     */
    std::string keymaps_file(const std::string& cc) const override;

    /**
     * @see Config::to_string()
     */
    std::string to_string() const override;
};

}
}
