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

#include "aspace.hpp"
#include "gpio.hpp"
#include "c64_crt.hpp"


namespace cemu {
namespace c64 {

/**
 * C64 Cartridge.
 * A C64 Cartridge implements the I/O expansion device and it is associated to a CRT file.
 * This class implements a device that handles the C64 addresses from $DE00 to $DFFF.
 * It also controls the GAME and EXROM pins, devices that want to get noticed when those
 * pins change must register themselves as GPIO callbacks.
 * @see c64::C64IO
 * @see c64::Crt
 * @see Device
 * @see Gpio
 */
class Cartridge : public Device {
public:
    constexpr static const char *TYPE        = "I/O-EXPANSION";
    constexpr static const size_t IOEXP_SIZE = 512;
    constexpr static const uint8_t GAME      = 0x01;
    constexpr static const uint8_t EXROM     = 0x02;

    /**
     * Instantiate a cartridge device from a CRT file.
     * @param fname Name of the CRT file to load.
     * @return A pointer to the new cartridge device.
     * @exception InvalidCartridge
     * @see c64::Crt
     */
    static std::shared_ptr<Cartridge> create(const std::string &fname);

    virtual ~Cartridge();

    /**
     * @return The name of this cartridge.
     * @see Crt::name()
     */
    std::string name() const;

    /**
     * @see Device::size()
     */
    size_t size() const override;

    /**
     * @see Device::dump()
     */
    std::ostream &dump(std::ostream &os, addr_t base = 0) const override;

    /**
     * Add an input callback.
     * @param ior  Input callback;
     * @param mask Bits used by the callback.
     * @see Gpio::add_ior()
     */
    void add_ior(const Gpio::ior_t &ior, uint8_t mask);

    /**
     * Add an ouput callback.
     * @param iow  Output callback;
     * @param mask Bits used by the callback.
     * @see Gpio::add_iow()
     */
    void add_iow(const Gpio::iow_t &iow, uint8_t mask);

    /**
     * Retrieve the cartridge internal device that must handle a specific memory address.
     * @param addr Memory bank starting address;
     * @param romh Status of ROMH line;
     * @param roml Status of ROML line.
     * @return A pointer to the device or nullptr if the specified address is not handled by this cartridge.
     */
    virtual std::pair<ASpace::devmap_t, ASpace::devmap_t> getdev(addr_t addr, bool romh, bool roml) = 0;

    /**
     * @return The total size of this cartridge ROMs.
     */
    virtual size_t cartsize() const = 0;

    /**
     * Reset this cartridge.
     * What this method does depends on the implementation.
     */
    virtual void reset() = 0;

protected:
    Cartridge(const std::string &type, const std::shared_ptr<Crt> &crt);

    std::shared_ptr<Crt> _crt{};
    Gpio                 _ioport{};
};

}
}
