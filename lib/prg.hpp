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
#pragma once

#include <string>
#include <vector>

#include "types.hpp"


namespace cemu {

/**
 * A PRG file.
 */
class Prg : public std::vector<uint8_t> {
public:
    /**
     * Initialise this PRG.
     * @param fname Name of the PRG file to load (an empty string if this PRG must be empty initialised);
     * @param start If non-zero, load the PRG file at the specified address.
     * If the start parameter is not specified the first two bytes of the PRG
     * file are considered as the start address (not data).
     * @exception IOError
     * @see load()
     */
    Prg(const std::string &fname = {}, addr_t start = {}) {
        load(fname, start);
    }

    /**
     * @return The start address of this PRG in memory.
     */
    addr_t addr() const {
        return _start;
    }

    /**
     * Set the start address of this PRG in memory.
     * @param addr Start address.
     */
    void addr(addr_t addr) {
        _start = addr;
    }

    /**
     * Load a PRG file.
     * @param fname Name of the PRG file to load.
     * @param start If non-zero, load the PRG file at the specified address.
     * If the start parameter is not specified the first two bytes of the PRG
     * file are considered as the start address (not data).
     * @exception IOError
     * @see save()
     */
    void load(const std::string &fname, addr_t start);

    /**
     * Save a PRG file.
     * @param fname Name of the PRG file to save.
     * @param start If non-zero use this value as the PRG's start address ignoring addr().
     * If addr() is zero the start address is not written into the PRG file.
     * @exception IOError
     * @see load()
     */
    void save(const std::string &fname, addr_t start = {});

private:
    addr_t _start{};
};

}
