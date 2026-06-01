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
#include "nes_cartridge.hpp"

#include "mapper_000.hpp"
#include "mapper_001.hpp"
#include "mapper_002.hpp"
#include "mapper_003.hpp"
#include "mapper_004.hpp"
#include "mapper_071.hpp"
#include "mapper_232.hpp"

namespace caio {
namespace nintendo {
namespace nes {

sptr_t<Cartridge> make_cartridge(const fs::Path& fname)
{
    const auto fullpath = fs::search(fname);
    if (fullpath.empty()) {
        if (fname.empty()) {
            throw InvalidCartridge{"Cartridge file not specified"};
        }
        throw InvalidCartridge{"Invalid cartridge file: {}", fname.string()};
    }

    auto [hdr, is] = iNES::load_header(fullpath);
    const size_t mapper = hdr.mapper();

    switch (mapper) {
    case 0:
        return std::make_shared<Mapper_000>(fullpath, hdr, is);
    case 1:
        return std::make_shared<Mapper_001>(fullpath, hdr, is);
    case 2:
        return std::make_shared<Mapper_002>(fullpath, hdr, is);
    case 3:
        return std::make_shared<Mapper_003>(fullpath, hdr, is);
    case 4:
        return std::make_shared<Mapper_004>(fullpath, hdr, is);
    case 71:
        return std::make_shared<Mapper_071>(fullpath, hdr, is);
    case 232:
        return std::make_shared<Mapper_232>(fullpath, hdr, is);
    default:;
    }

    throw InvalidCartridge{"Can't instantiate cartridge: {}: Mapper not supported: {}",
        fullpath.c_str(), iNES::to_string(hdr)};
}

}
}
}
