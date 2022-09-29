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
#include "c1541_factory.hpp"

#include "types.hpp"
#include "fs.hpp"

#include "c1541_fs.hpp"


namespace caio {
namespace c1541 {

std::shared_ptr<C1541> create(const std::string &path, uint8_t unit, const std::shared_ptr<cbm_bus::Bus> &bus)
{
    std::shared_ptr<C1541> drive{};

    if (!fs::exists(path)) {
        throw IOError{"Can't create C1541 instance: " + path + ": " + Error::to_string(ENOENT)};
    }

    try {
        if (std::filesystem::is_directory(path)) {
            drive = std::make_shared<C1541Fs>(unit, bus);
            drive->attach(path);
        } else {
            throw IOError{"Can't create C1541 instance: " + path + ": Unsupported format"};
        }
    } catch (const std::filesystem::filesystem_error &err) {
        throw IOError{std::string{"Can't create C1541 instance: "} + err.what()};
    }

    return drive;
}

}
}
