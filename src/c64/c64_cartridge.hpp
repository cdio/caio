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

#include "aspace.hpp"
#include "fs.hpp"
#include "gpio.hpp"

#include "c64_crt.hpp"

namespace caio {
namespace commodore {
namespace c64 {

/**
 * C64 Cartridge.
 * A C64 Cartridge implements an I/O expansion device associated to a CRT file.
 * The device part of this class handles the C64 addresses from $DE00 to $DFFF (see Device);
 * The gpio part of this class handles the GAME and EXROM output pins through callbacks (see Gpio).
 * @see c64::C64IO
 * @see c64::Crt
 * @see Device
 * @see Gpio
 */
class Cartridge : public Device {
public:
    constexpr static const char* TYPE            = "I/O-EXPANSION";
    constexpr static const size_t IO_SIZE        = 512;
    constexpr static const size_t IO_ADDR_MASK   = 0x01FF;
    constexpr static const uint8_t GAME          = 0x01;
    constexpr static const uint8_t EXROM         = 0x02;
    constexpr static const uint8_t GAME_EXROM_00 = 0x00;
    constexpr static const uint8_t GAME_EXROM_01 = Cartridge::EXROM;
    constexpr static const uint8_t GAME_EXROM_10 = Cartridge::GAME;
    constexpr static const uint8_t GAME_EXROM_11 = Cartridge::GAME | Cartridge::EXROM;

    enum GameExromMode {
        MODE_8K        = GAME_EXROM_10,
        MODE_16K       = GAME_EXROM_00,
        MODE_ULTIMAX   = GAME_EXROM_01,
        MODE_INVISIBLE = GAME_EXROM_11
    };

    /**
     * Instantiate a cartridge device associated to a CRT file.
     * The returned cartridge cannot be used until its reset() method is called.
     * @param fname Name of the CRT file to associate.
     * @return A pointer to the new cartridge device.
     * @exception InvalidCartridge
     * @see c64::Crt
     * @see reset()
     */
    static sptr_t<Cartridge> instance(const fs::Path& fname);

    virtual ~Cartridge() = default;

    /**
     * @return The name of this cartridge.
     * @see Crt::name()
     */
    std::string name() const;

    /**
     * Reset this cartridge.
     * Load the chips embedded inside the associated CRT
     * file and set the GAME/EXROM lines accordingly.
     * @exception InvalidCartridge
     * @see Crt
     * @see Device::reset()
     */
    void reset() override;

    /**
     * @see Device::size()
     */
    size_t size() const override;

    /**
     * Add an input callback.
     * @param ior  Input callback;
     * @param mask Bits used by the callback.
     * @see Gpio::add_ior()
     */
    void add_ior(const Gpio::IorCb& ior, uint8_t mask);

    /**
     * Add an ouput callback.
     * @param iow  Output callback;
     * @param mask Bits used by the callback.
     * @see Gpio::add_iow()
     */
    void add_iow(const Gpio::IowCb& iow, uint8_t mask);

    /**
     * Retrieve the cartridge internal device that must handle a specific memory address.
     * @param addr Memory bank base address;
     * @param romh Status of ROMH line;
     * @param roml Status of ROML line.
     * @return A pair of read and write devices or an empty pair
     * if the specified base address is not handled by this cartridge.
     */
    virtual std::pair<ASpace::devmap_t, ASpace::devmap_t> getdev(addr_t addr, bool romh, bool roml) = 0;

    /**
     * Get the total size of this cartridge.
     * @return The total size of this cartridge ROMs.
     */
    virtual size_t cartsize() const = 0;

    /**
     * Get the current GAME/EXROM mode.
     * @return The current GAME/EXROM mode.
     */
    GameExromMode mode() const;

protected:
    Cartridge(std::string_view type, const sptr_t<Crt>& crt);

    /**
     * Set a new GAME/EXROM mode and propagate it.
     * @param mode Mode to set.
     * @see GameExromMode
     * @see propagate()
     */
    void mode(GameExromMode mode);

    /**
     * Propagate the GAME/EXROM gpio lines.
     * @param force Force propagation.
     * @see Gpio::iow()
     */
    virtual void propagate(bool force = false);

    /**
     * Get the CRT file backing this cartridge.
     * @return A reference to the backing CRT file.
     * @see Crt
     */
    const Crt& crt() const;

    void throw_invalid_cartridge(ssize_t entry, std::string_view errmsg);

    template<typename... Args>
    void throw_invalid_cartridge(std::format_string<Args...> fmt, Args&&... args)
    {
        throw_invalid_cartridge(-1, std::vformat(fmt.get(), std::make_format_args(args...)));
    }

    template<typename... Args>
    void throw_invalid_cartridge(ssize_t entry, std::format_string<Args...> fmt, Args&&... args)
    {
        throw_invalid_cartridge(entry, std::vformat(fmt.get(), std::make_format_args(args...)));
    }

private:
    sptr_t<Crt>     _crt;
    Gpio            _ioport{};
    GameExromMode   _mode{};

    friend Serializer& operator&(Serializer&, Cartridge&);
};

}
}
}
