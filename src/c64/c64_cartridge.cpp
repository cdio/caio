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
#include "c64_cartridge.hpp"

#include <array>

#include "logger.hpp"
#include "types.hpp"
#include "utils.hpp"

#include "c64_cart_c64_game_system_3.hpp"
#include "c64_cart_easy_flash.hpp"
#include "c64_cart_generic.hpp"
#include "c64_cart_ocean_type_1.hpp"
#include "c64_cart_magic_desk.hpp"
#include "c64_cart_simons_basic.hpp"
#include "c64_cart_zaxxon.hpp"

namespace caio {
namespace commodore {
namespace c64 {

Cartridge::Cartridge(std::string_view type, const sptr_t<Crt>& crt)
    : Device{type, (crt ? crt->name() : "")},
      _crt{crt}
{
    CAIO_ASSERT(crt.get() != nullptr);
}

std::string Cartridge::name() const
{
    return _crt->name();
}

void Cartridge::reset()
{
    log.debug("{}: {}\n", type(), _crt->to_string());

    _mode = static_cast<GameExromMode>((_crt->game() ? GAME : 0 ) | (_crt->exrom() ? EXROM : 0));
}

size_t Cartridge::size() const
{
    return IO_SIZE;
}

void Cartridge::add_ior(const Gpio::IorCb& ior, uint8_t mask)
{
    _ioport.add_ior(ior, mask);
}

void Cartridge::add_iow(const Gpio::IowCb& iow, uint8_t mask)
{
    _ioport.add_iow(iow, mask);
}

void Cartridge::propagate(bool force)
{
    _ioport.iow(0, _mode, force);
}

const Crt& Cartridge::crt() const
{
    return (*_crt);
}

Cartridge::GameExromMode Cartridge::mode() const
{
    return _mode;
}

void Cartridge::mode(Cartridge::GameExromMode mode)
{
    if (mode != _mode) {
        _mode = mode;
        propagate();
    }
}

void Cartridge::throw_invalid_cartridge(ssize_t entry, std::string_view errmsg)
{
    if (entry < 0) {
        throw InvalidCartridge{"{}: {}: {}. {}", type(), name(), errmsg, _crt->to_string()};
    } else {
        throw InvalidCartridge{"{}: {}: Chip entry {}: {}. {}", type(), name(), entry, errmsg, _crt->to_string()};
    }
}

sptr_t<Cartridge> Cartridge::instance(const fs::Path& fname)
{
    auto crt = std::make_shared<Crt>(fname);

    switch (crt->type()) {
    case Crt::HW_TYPE_GENERIC:
        return std::make_shared<CartGeneric>(crt);

    case Crt::HW_TYPE_ACTION_REPLAY:
    case Crt::HW_TYPE_KCS_POWER_CARTRIDGE:
    case Crt::HW_TYPE_FINAL_CARTRIDGE_III:
        break;

    case Crt::HW_TYPE_SIMONS_BASIC:
        return std::make_shared<CartSimonsBasic>(crt);

    case Crt::HW_TYPE_OCEAN_TYPE_1:
        return std::make_shared<CartOceanType1>(crt);

    case Crt::HW_TYPE_EXPERT_CARTRIDGE:
    case Crt::HW_TYPE_FUN_PLAY:
    case Crt::HW_TYPE_SUPER_GAMES:
    case Crt::HW_TYPE_ATOMIC_POWER:
    case Crt::HW_TYPE_WESTERMANN_LEARNING:
    case Crt::HW_TYPE_REX_UTILITY:
    case Crt::HW_TYPE_FINAL_CARTRIDGE_I:
    case Crt::HW_TYPE_MAGIC_FORMEL:
        break;

    case Crt::HW_TYPE_C64_GAME_SYSTEM_3:
        return std::make_shared<CartC64GameSystem3>(crt);

    case Crt::HW_TYPE_WARP_SPEED:
    case Crt::HW_TYPE_DINAMIC:
        break;

    case Crt::HW_TYPE_ZAXXON:
        return std::make_shared<CartZaxxon>(crt);

    case Crt::HW_TYPE_MAGIC_DESK:
        return std::make_shared<CartMagicDesk>(crt);

    case Crt::HW_TYPE_COMAL_80:
    case Crt::HW_TYPE_STRUCTURED_BASIC:
    case Crt::HW_TYPE_ROSS:
    case Crt::HW_TYPE_DELA_EP64:
    case Crt::HW_TYPE_DELA_EP7x8:
    case Crt::HW_TYPE_DELA_EP256:
    case Crt::HW_TYPE_REX_EP256:
    case Crt::HW_TYPE_MIKRO_ASSEMBLER:
    case Crt::HW_TYPE_FINAL_CARTRIDGE_PLUS:
    case Crt::HW_TYPE_ACTION_REPLAY_4:
    case Crt::HW_TYPE_STARDOS:
        break;

    case Crt::HW_TYPE_EASY_FLASH:
        return std::make_shared<CartEasyFlash>(crt);

    case Crt::HW_TYPE_EASY_FLASH_XBANK:
    case Crt::HW_TYPE_CAPTURE:
    case Crt::HW_TYPE_ACTION_REPLAY_3:
    case Crt::HW_TYPE_RETRO_REPLAY:
    case Crt::HW_TYPE_MMC_64:
    case Crt::HW_TYPE_MMC_REPLAY:
    case Crt::HW_TYPE_IDE_64:

    case Crt::HW_TYPE_SUPER_SNAPSHOT_V4:
    case Crt::HW_TYPE_IEEE_488:
    case Crt::HW_TYPE_GAME_KILLER:
    case Crt::HW_TYPE_PROPHET_64:
    case Crt::HW_TYPE_EXOS:
    case Crt::HW_TYPE_FREEZE_FRAME:
    case Crt::HW_TYPE_FREEZE_MACHINE:
    case Crt::HW_TYPE_SNAPSHOT_64:
    case Crt::HW_TYPE_SUPER_EXPLODE_V5_0:
    case Crt::HW_TYPE_MAGIC_VOICE:

    case Crt::HW_TYPE_ACTION_REPLAY_2:
    case Crt::HW_TYPE_MACH_5:
    case Crt::HW_TYPE_DIASHOW_MAKER:
    case Crt::HW_TYPE_PAGEFOX:
    case Crt::HW_TYPE_KINGSOFT:
    case Crt::HW_TYPE_SILVERROCK_128K:
    case Crt::HW_TYPE_FORMEL_64:
    case Crt::HW_TYPE_RGCD:
    case Crt::HW_TYPE_RR_NET_MK3:
    case Crt::HW_TYPE_EASY_CALC:

    case Crt::HW_TYPE_GMOD_2:
    case Crt::HW_TYPE_MAX_BASIC:
    case Crt::HW_TYPE_GMOD_3:
    case Crt::HW_TYPE_ZIPP_CODE_48:
    case Crt::HW_TYPE_BLACKBOX_V8:
    case Crt::HW_TYPE_BLACKBOX_V3:
    case Crt::HW_TYPE_BLACKBOX_V4:
    case Crt::HW_TYPE_REX_RAM_FLOPPY:
    case Crt::HW_TYPE_BIS_PLUS:
    case Crt::HW_TYPE_SD_BOX:

    case Crt::HW_TYPE_MULTIMAX:
    case Crt::HW_TYPE_BLACKBOX_V9:
    case Crt::HW_TYPE_LT_KERNAL_HOST_ADAPTOR:
    case Crt::HW_TYPE_RAMLINK:
    case Crt::HW_TYPE_HERO:
    case Crt::HW_TYPE_IEEE_FLASH_64:
    case Crt::HW_TYPE_TURTLE_GRAPHICS_II:
    case Crt::HW_TYPE_FREEZE_FRAME_MK2:
    default:;
    }

    throw InvalidCartridge{"{}: Hardware type not supported: {}, {}", Cartridge::TYPE, static_cast<int>(crt->type()),
        crt->to_string()};
}

}
}
}
