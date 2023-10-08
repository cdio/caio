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

#include <array>
#include <functional>
#include <utility>

#include "aspace.hpp"
#include "types.hpp"


namespace caio {
namespace c64 {

/**
 * C64 PLA.
 * This class is an hybrid, it implements some of the real PLA functions and uses
 * those results to implement the decoding of the entire address space of the C64.
 * @see ASpace
 */
class PLA : public ASpace {
public:
    constexpr static const uint8_t LORAM    = 0x01;
    constexpr static const uint8_t HIRAM    = 0x02;
    constexpr static const uint8_t CHAREN   = 0x04;
    constexpr static const uint8_t GAME     = 0x08;
    constexpr static const uint8_t EXROM    = 0x10;
    constexpr static const uint8_t INVALID  = 0xFF;
    constexpr static const uint8_t MASK     = LORAM | HIRAM | CHAREN | GAME | EXROM;

    constexpr static const addr_t A15       = 1 << 15;
    constexpr static const addr_t A14       = 1 << 14;
    constexpr static const addr_t A13       = 1 << 13;

    constexpr static const addr_t ADDR_MASK = 0xFFFF;
    constexpr static const size_t BANKS     = 32;
    constexpr static const size_t BLOCKS    = 16;

    using bank_t = std::array<devmap_t, BLOCKS>;
    using extmap_cb_t = std::function<std::pair<devmap_t, devmap_t>(addr_t, bool, bool)>;

    /**
     * Initialise this PLA.
     * The default memory map is created.
     * @param ram     System RAM;
     * @param basic   Basic ROM;
     * @param kernel  Kernel ROM;
     * @param chargen Character set ROM;
     * @param io      I/O device (see C64IO)
     * @see C64IO
     */
    PLA(const devptr_t& ram, const devptr_t& basic, const devptr_t& kernal, const devptr_t& chargen,
        const devptr_t& io);

    virtual ~PLA() {
    }

    /**
     * Reset this PLA.
     * Reinitialise this PLA and set the default mappings.
     * (This is more like a re-init rather than a reset).
     */
    void reset() override;

    /**
     * Set PLA input pins.
     * @param pins  Input pins to set o clear (based on mask);
     * @param mask  Mask of the input pins to set or clear;
     * @param force Force pin settings even if the current mode is the same.
     */
    void mode(uint8_t pins, uint8_t mask, bool force = false);

    /**
     * @return The status of the PLA input pins.
     */
    uint8_t mode() const;

    /**
     * Set the I/O extension mappings callback.
     * This callback is called each time the memory mappings are changed.
     * The specified callback receives a memory address and the status of ROML and ROMH pins;
     * the callback must respond with a pair of devices (devptr_t), the first one to process
     * read operatons and the second one to process write operations.
     * @param extmap I/O extension mappings callback.
     * @see ASpace::devptr_t
     */
    void extmap(const extmap_cb_t& extmap);

private:
    /**
     * Reset the address space mappings.
     * This method is called when at least one input pin is changed.
     */
    void remap();

    bool romh(addr_t addr) const;

    bool roml(addr_t addr) const;

    uint8_t                   _mode{INVALID};   /* Bitwise combination of LORAM, HIRAM, CHAREN, GAME, and EXROM */
    extmap_cb_t               _extmap{};        /* I/O Extension (cartridge) callback                           */
    std::array<bank_t, BANKS> _rmodes{};        /* Default read mapping modes                                   */
    std::array<bank_t, BANKS> _wmodes{};        /* Default write mapping modes                                  */
    bank_t                    _rmaps{};         /* Current read mappings                                        */
    bank_t                    _wmaps{};         /* Current write mappings                                       */
};

}
}
