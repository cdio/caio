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
#include "c64_cartridge.hpp"

#include "types.hpp"
#include "logger.hpp"
#include "utils.hpp"

#include "device_rom.hpp"


namespace cemu {
namespace c64 {

//XXX move to proper files. c64/cart/...
/*
 * Generic Cartridge.
 */
class CartridgeGeneric : public Cartridge {
public:
    constexpr static const char *TYPE = "CART_GENERIC";

    CartridgeGeneric(const std::shared_ptr<Crt> &crt, const std::shared_ptr<DeviceGpio> &ioexp)
        : Cartridge{TYPE, crt, ioexp}
    {
        uint8_t port = (_crt->game() ? GAME : 0) | (_crt->exrom() ? EXROM : 0);
        switch (port) {
        case 0:
            /*
             * 16K generic cartridge.
             */
            _rom = (*_crt)[0].second;
            break;

        case GAME:
            /*
             * 8K generic cartridge.
             */
            _rom = (*_crt)[0].second;
            break;

        default:
            /* TODO: ultimax support? */
            throw InvalidCartridge{TYPE, "exrom/game combination not supported: " + _crt->to_string()};
        }

        /*
         * Propagate the outout port (GAME and EXROM flags) to the connected devices (f.ex. PLA).
         */
        log.debug("cart: port=%02x\n", port);


        iow(0, port); //<--- PLA not yet registered
#if 1
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

        invece di fare tutto questo casino, guarda se le linee exrom/game rimangono fisse per
        tutti i tipi di cartrige, se cosi fosse allora lo stesso pla puo chiedere all cartridge
        le sue linee durante la sua inizializzazione e settare il modo giusto.

        Guarda anche se sono necessarie i pin di output romh e roml, magari non servono e si semplifica
        tutto.

<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#endif
    }

    virtual ~CartridgeGeneric()
    {
    }

    uint8_t read(addr_t addr) const override
    {
        return _rom->read(addr);
    }

    void write(addr_t addr, uint8_t data) override
    {
        _rom->write(addr, data);
    }

    size_t size() const override
    {
        return _rom->size();
    }

    std::ostream &dump(std::ostream &os, addr_t base = 0) const override
    {
        return _rom->dump(os, base);
    }

private:
    std::shared_ptr<Device> _rom{};
};

class CartridgeOceanType1 : public Cartridge {
public:
    constexpr static const char *TYPE = "CART_OCEAN_TYPE_1";

    CartridgeOceanType1(const std::shared_ptr<Crt> &crt, const std::shared_ptr<DeviceGpio> &ioexp)
        : Cartridge{TYPE, crt, ioexp}
    {
#if 0
        /*
         * OCEAN type 1 cartridge.
         */
        ioexp->add_iow([](uint8_t addr, uint8_t data) {
            auto bank = data & 0x3F;
            if (bank < 16) {
                /*
                 * Banks 0..15 loaded at roml, $8000..$9FFF
                 */
  //              _rom.roml(_crt[bank].second);
            } else {
                /*
                 * Banks 16..63 loaded at romh, $A000..$BFFF
                 */
   //             _rom.romh(_crt[bank].second);
            }
        }, DE00_BANK, 255);
#endif
    }

    virtual ~CartridgeOceanType1() {
    }

    uint8_t read(addr_t addr) const override { return 0; }

    void write(addr_t addr, uint8_t data) override
    {
    }

    size_t size() const override
    {
        return 0;
    }

    std::ostream &dump(std::ostream &os, addr_t base = 0) const override
    {
        return os;
    }
};


std::shared_ptr<Cartridge> Cartridge::create(const std::shared_ptr<Crt> &crt, const std::shared_ptr<DeviceGpio> &ioexp)
{
    if (!crt || !ioexp) {
        throw InvalidArgument{"Cartridge::create", ""};
    }

    Crt::HardwareType type = crt->type();

    switch (type) {
    case Crt::HW_TYPE_GENERIC:
        return std::make_shared<CartridgeGeneric>(crt, ioexp);

    case Crt::HW_TYPE_OCEAN_TYPE_1:
        return std::make_shared<CartridgeOceanType1>(crt, ioexp);

    case Crt::HW_TYPE_ACTION_REPLAY:
    case Crt::HW_TYPE_KCS_POWER_CARTRIDGE:
    case Crt::HW_TYPE_FINAL_CARTRIDGE_III:
    case Crt::HW_TYPE_SIMONS_BASIC:
    case Crt::HW_TYPE_EXPERT_CARTRIDGE:
    case Crt::HW_TYPE_FUN_PLAY:
    case Crt::HW_TYPE_SUPER_GAMES:
    case Crt::HW_TYPE_ATOMIC_POWER:

    case Crt::HW_TYPE_WESTERMANN_LEARNING:
    case Crt::HW_TYPE_REX_UTILITY:
    case Crt::HW_TYPE_FINAL_CARTRIDGE_I:
    case Crt::HW_TYPE_MAGIC_FORMEL:
    case Crt::HW_TYPE_C64_GAME_SYSTEM_3:
    case Crt::HW_TYPE_WARP_SPEED:
    case Crt::HW_TYPE_DINAMIC:
    case Crt::HW_TYPE_ZAXXON:
    case Crt::HW_TYPE_MAGIC_DESK:

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
    case Crt::HW_TYPE_EASY_FLASH:
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

    throw InvalidCartridge{"Cartridge", "Hardware Type not supported: " + crt->to_string()};
}

Cartridge::Cartridge(const std::string &type, const std::shared_ptr<Crt> &crt, const std::shared_ptr<DeviceGpio> &ioexp)
    : Device{type, crt->name()},
      _crt{crt},
      _ioexp{ioexp}
{
}

Cartridge::~Cartridge()
{
}

std::string Cartridge::name() const
{
    return _crt->name();
}

}
}
