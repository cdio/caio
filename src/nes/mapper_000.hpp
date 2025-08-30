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

#include "nes_cartridge.hpp"

namespace caio {
namespace nintendo {
namespace nes {

/**
 * Cartridge Mapper 000 (NROM).
 *
 * ### Address range accessed by the CPU:
 *
 *     Mapper Address     CPU Address     Descripton
 *     -------------------------------------------------------------------------------------------------
 *     2000-3FFF          6000-7FFF       Family Basic only: PRG RAM (mirrored to fill 8K)
 *     4000-7FFF          8000-BFFF       16K PRG ROM (NROM-128) / First half of 32K PRG ROM (NROM-256)
 *     8000-BFFF          C000-FFFF       Mirror of 16K PRG ROM / Second half of 32K PRG ROM
 *
 * ### Address range accessed by the PPU:
 *
 *     Mapper Address     PPU Address     Descripton
 *     -------------------------------------------------------------------------------------------------
 *     C000-DFFF          0000-1FFF       8KB Character ROM
 *     E000-E7Ff          2000-27FF       2K VRAM
 *     E800-FFFF          2800-2FFF       2K VRAM (Nametable mirroring)
 *
 * - PRG ROM size:
 *   16K (NROM-128), 32K (NROM-256)
 *
 * - PRG ROM bank size:
 *   Not bank-switched
 *
 * - PRG RAM:
 *   2K or 4K, not bank-switched, only in Family Basic (but most emulators provide 8)
 *
 * - CHR capacity:
 *   8K ROM
 *
 * - CHR bank size
 *   Not bank-switched
 *
 * - Nametable mirroring:
 *   Solder pads select vertical or horizontal mirroring
 *
 * @see https://www.nesdev.org/wiki/NROM
 */
class Mapper_000 : public Cartridge {
public:
    constexpr static const char* TYPE = "CART_NROM";

    /**
     * @see Cartridge::Cartridge(std::string_view, const fs::Path&, const iNES::Header& std::ifstream&)
     */
    Mapper_000(const fs::Path& fname, const iNES::Header& hdr, std::ifstream& is);

    virtual ~Mapper_000() = default;

    void reset() override;
};

}
}
}
