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
#include <memory>

#include "device_gpio.hpp"
#include "c64_crt.hpp"

#include "logger.hpp"


namespace cemu {
namespace c64 {

/**
 * C64 Cartridge.
 * A C64 Cartridge implements the emulator part of a CRT file.
 * @see c64::Crt
 * TODO This class should implement the expansion port.
 */
class Cartridge : public Device, public Gpio {
public:
    constexpr static const addr_t DE00_BANK = 0;
    constexpr static const addr_t DF00_BANK = 1;

    constexpr static const uint8_t GAME     = 0x01;
    constexpr static const uint8_t EXROM    = 0x02;
    constexpr static const uint8_t ROMH     = 0x04;
    constexpr static const uint8_t ROML     = 0x08;

    /**
     * Instanciate a Cartridge from a CRT file.
     * @param crt   Valid CRT file;
     * @param ioexp C64 I/O expansion device.
     * @exception InvalidArgument
     * @exception InvalidCartridge
     */
    static std::shared_ptr<Cartridge> create(const std::shared_ptr<Crt> &crt, const std::shared_ptr<DeviceGpio> &ioexp);

    virtual ~Cartridge();

    /**
     * @return The name of this cartridge.
     */
    std::string name() const;

protected:
    /**
     * Create a CRT instance.
     * @param type  Cartridge type string;
     * @param crt   Valid cartridge file;
     * @param ioexp C64 I/O expansion device.
     * @exceptions InvalidCartridge
     */
    Cartridge(const std::string &type, const std::shared_ptr<Crt> &crt, const std::shared_ptr<DeviceGpio> &ioexp);

    std::shared_ptr<Crt>        _crt{};
    std::shared_ptr<DeviceGpio> _ioexp{};
};

}
}
