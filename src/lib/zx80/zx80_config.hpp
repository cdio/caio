/*
 * Copyright (C) 2020-2022 Claudio Castiglia
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
namespace zx80 {

/**
 * ZX80 configuation file.
 */
class ZX80Confile : public CaioConfile {
public:
    constexpr static const char *ZX80_CONFIG_SECTION = "zx80";

    ZX80Confile();

    explicit ZX80Confile(Confile &&other);

    virtual ~ZX80Confile();

    ZX80Confile &operator=(Confile &&other);
};

/**
 *  ZX80 configuration.
 */
struct ZX80Config : public Config {
    std::string cartfile{};

    ZX80Config();

    explicit ZX80Config(const Confile &conf);

    virtual ~ZX80Config();

    /**
     * @see Config::operator=(const Confile &)
     */
    ZX80Config &operator=(const Confile &conf);

    /**
     * @see Config::keymaps_file()
     */
    std::string keymaps_file(const std::string &cc) const override;

    /**
     * @see Config::to_string()
     */
    std::string to_string() const override;
};

}
}
