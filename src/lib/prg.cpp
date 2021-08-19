/*
 * Copyright (C) 2020 Claudio Castiglia
 *
 * This file is part of CEMU.
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
#include "prg.hpp"

#include <endian.h>
#include <fstream>


namespace cemu {

void Prg::load(const std::string &fname, addr_t start)
{
    if (!fname.empty()) {
        try {
            std::ifstream is{fname, std::ios::binary | std::ios::in};
            if (!is) {
                throw IOError{"Can't open PRG file: " + fname + ": " + Error::to_string()};
            }

            if (start == 0) {
                if (!is.read(reinterpret_cast<char *>(&start), sizeof(addr_t))) {
                    throw IOError{"Can't read PRG start address: " + fname + ": " + Error::to_string()};
                }
            }

            _start = le16toh(start);

            uint8_t c{};
            while (is.read(reinterpret_cast<char *>(&c), sizeof(c))) {
                push_back(c);
            }

        } catch (const std::exception &e) {
            throw IOError{e};
        }
    }
}

void Prg::save(const std::string &fname, addr_t start)
{
    if (!fname.empty()) {
        try {
            std::ofstream os{fname, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc};
            if (!os) {
                throw IOError{"Can't create PRG file: " + fname + ": " + Error::to_string()};
            }

            if (start == 0) {
                start = addr();
            }

            addr_t lestart = htole16(start);
            if (lestart != 0) {
                if (!os.write(reinterpret_cast<char *>(&lestart), sizeof(lestart))) {
                    throw IOError{"Can't write PRG address: " + fname + ": " + Error::to_string()};
                }
            }

            for (auto it = begin(); it != end(); ++it) {
                char c = static_cast<char>(*it);
                if (!os.write(&c, sizeof(c))) {
                    throw IOError{"Can't write PRG data: " + fname + ": " + Error::to_string()};
                }
            }

        } catch (const std::exception &e) {
            throw IOError{"Can't save PRG file: " + fname + ": " + e.what()};
        }
    }
}

}
