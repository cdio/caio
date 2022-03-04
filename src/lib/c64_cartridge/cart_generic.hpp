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

#include "../c64_cartridge.hpp"


namespace cemu {
namespace c64 {

/**
 * Generic Cartridge.
 * Generic cartridges can contain one or two 8K CHIP packets, one for ROML and one for ROMH.
 * ROML start address is always $8000, ROMH start address is either $A000 or $E000 depending
 * on the GAME/EXROM config.
 *
 * Type     Size    Game    EXROM   ROML    ROMH    LOAD ADDRESS
 * -------------------------------------------------------------
 * Normal   8K      1       0       $8000   -----   $8000-$9FFF
 * Normal   16K     0       0       $8000   $A000   $8000-$BFFF
 * Ultimax  8K      0       1       -----   $E000   $E000-$FFFF
 * Ultimax  16K     0       1       $8000   $E000   $8000-$9FFF and $E000-$FFFF
 * Ultimax  4K      0       1       $F000   -----   $F000-$F7FF
 *
 * The ROMH and ROML lines are CPU-controlled status lines, used to bank in/out RAM, ROM or I/O,
 * depending on what is needed at the time.
 *
 * @see https://vice-emu.sourceforge.io/vice_17.html#SEC395
 * @see https://ist.uwaterloo.ca/~schepers/formats/CRT.TXT
 */
class CartGeneric : public Cartridge {
public:
    constexpr static const char *TYPE = "CART_GENERIC";
    constexpr static const uint8_t GAME_EXROM_00 = 0x00;
    constexpr static const uint8_t GAME_EXROM_01 = Cartridge::EXROM;
    constexpr static const uint8_t GAME_EXROM_10 = Cartridge::GAME;
    constexpr static const uint8_t GAME_EXROM_11 = Cartridge::GAME | Cartridge::EXROM;

    enum class Mode {
        NORMAL_8K,
        NORMAL_16K,
        ULTIMAX_4K,
        ULTIMAX_8K,
        ULTIMAX_16K,
        NONE
    };

    CartGeneric(const std::shared_ptr<Crt> &crt);

    virtual ~CartGeneric();

    /**
     * @see Device::read()
     */
    uint8_t read(addr_t addr) const override;

    /**
     * @see Device::write();
     */
    void write(addr_t addr, uint8_t data) override;

    /**
     * @see Device::to_string()
     */
    std::string to_string() const override;

    /**
     * @see Cartridge::getdev()
     */
    std::pair<ASpace::devmap_t, ASpace::devmap_t> getdev(addr_t addr, bool romh, bool roml) override;

    /**
     * @see Cartridge::cartsize()
     */
    size_t cartsize() const override;

    /**
     * Reset this cartridge.
     * Propagate the GAME/EXROM ouput lines to the connected devices.
     */
    void reset() override;

private:
    Mode     _mode{Mode::NONE};
    uint8_t  _game_exrom{};
    devptr_t _rom{};
};

}
}

/**
 * Cartridge type: Action Replay (1).
 *
 * Type     Size    Game    EXROM   ROML        ROMH    LOAD ADDRESS
 * ----------------------------------------------------------------------------
 *         32KiB    0       0       $8000       ----    $8000-$9FFF
 *
 * 32KiB (4 banks of 8KiB).
 * Bank selection? TODO
 */

/**
 * Cartridge type: KCS Power Cartridge (2)
 *
 * Type     Size    Game    EXROM   ROML        ROMH    LOAD ADDRESS
 * ----------------------------------------------------------------------------
 *          16KiB   0       0                           $8000-$9FFF (Module 1)
 *                                                      $A000-$BFFF (Module 2)
 * 16KiB (2 banks of 8KiB).
 * Bank selection? TODO
 */

/**
 * Cartridge type: Final Cartridge III (3)
 *
 * Type     Size    Game    EXROM   ROML        ROMH    LOAD ADDRESS
 * ----------------------------------------------------------------------------
 *          64KiB   1       1                           $8000-$BFFF (All modules)
 *
 * 64KiB (4 banks of 16KiB each)
 *
 * The banks are arranged in the following way:
 * Bank 0: BASIC, Monitor, Disk-Turbo
 * Bank 1: Notepad, BASIC (Menu Bar)
 * Bank 2: Desktop, Freezer/Print
 * Bank 3: Freezer, Compression
 *
 * The cartridges uses the entire I/O-1 and I/O-2 range.
 * Bank switching is done by writing the bank number plus $40 into memory location $DFFF.
 * For instance, to select bank 2, $DFFF is set to $42.
 *
 * The CRT file contains four CHIP blocks, each block with a start address of $8000, length $4000
 * and the bank number in the bank field.
 * In the cartridge header, both EXROM ($18) and GAME ($19) are set to 1 to enable the 16 KiB ROM configuration.
 *
 * The registers are arranged in the following way:
 *
 * One register at $DFFF:
 *
 * Bit  Meaning
 * ---  -------
 * 7    Hide this register (1 = hidden)
 * 6    NMI line   (0 = low = active)   (*1)
 * 5    GAME line  (0 = low = active)   (*2)
 * 4    EXROM line (0 = low = active)
 * 2-3  unassigned (usually set to 0)
 * 0-1  number of bank to show at $8000
 *
 * (*1) if either the freezer button is pressed, or bit 6 is 0, then an NMI is generated
 * (*2) if the freezer button is pressed, GAME is also forced low
 *
 * The rest of I/O-1/I/O-2 contain a mirror of the last 2 pages of the currently selected ROM bank
 * (also at $DFFF, contrary to what some other documents say)
 */

/**
 * Cartridge type: Simons' Basic (4)
 *
 * Type     Size    Game    EXROM   ROML        ROMH    LOAD ADDRESS
 * ----------------------------------------------------------------------------
 *          16KiB   1       0                           $8000-$9FFF (Module 1)
 *                                                      $A000-$BFFF (Module 2)
 *
 * Simons' BASIC permanently uses 16 KiB ($4000) bytes of cartridge memory from $8000-$BFFF.
 * However, through some custom bank-switching logic the upper area ($A000-$BFFF) may be disabled so
 * Simons' BASIC may use it as additional RAM. Writing a value of $01 to address location $DE00 banks in ROM,
 * $00 disables ROM and enables RAM.
 *
 * The CRT file contains two CHIP blocks of length $2000 each, the first block having a start address of $8000,
 * the second block $A000. In the cartridge header, EXROM ($18) is set to 0, GAME ($19) is set to 1 to indicate
 * the RESET/power-up configuration of 8 KiB ROM.
 */

/**
 * Cartridge type: Ocean type 1 (5)
 *
 * Type     Size    Game    EXROM   ROML        ROMH    LOAD ADDRESS
 * ----------------------------------------------------------------------------
 *          32KiB   0       0                           $8000-$9FFF (Banks 0-3)
 *
 *          128KiB  0       0                           $8000-$9FFF (Banks 0-15)
 *
 *          256KiB  0       0                           $8000-$9FFF (Banks 0-15)
 *                                                      $A000-$BFFF (Banks 16-31)
 *
 *          512KiB  0       0                           $8000-$9FFF (Banks 0-63)    TERMINATOR 2
 *
 * 32KiB, 128KiB, 256KiB or 512KiB sizes (4, 16, 32 or 64 banks of 8KiB each)
 *
 * Bank switching is done by writing to $DE00.
 * The lower six bits give the bank number (ranging from 0-63), bit 7 is always set.
 */

/**
 * Cartridge type: Expert Cartridge (6)
 *
 * Type     Size    Game    EXROM   ROML        ROMH    LOAD ADDRESS
 * ----------------------------------------------------------------------------
 *          8KiB    1       1                           $8000-$9FFF https://ist.uwaterloo.ca/~schepers/formats/CRT.TXT
 *          8KiB    1       0                           $8000-$9FFF https://vice-emu.sourceforge.io/vice_17.html#SEC401
 */

/**
 * Cartridge type: Fun Play, Power Play (7)
 *
 * Type     Size    Game    EXROM   ROML        ROMH    LOAD ADDRESS
 * ----------------------------------------------------------------------------
 *          128KiB  0       0                           $8000-$9FFF (All modules) https://ist.uwaterloo.ca/~schepers/formats/CRT.TXT
 *          128KiB  1       0                           $8000-$9FFF (All modules) https://vice-emu.sourceforge.io/vice_17.html#SEC402
 *
 * 128KiB (16 banks of 8KiB modules)
 *
 * Uses $DE00 for bank selection, and uses 8KiB banks ($2000) at $8000-$9FFF.
 * There are 16 banks of ROM memory and are referenced by the following values:
 *   $00 -> Bank 0
 *   $08 -> Bank 1
 *   $10 -> Bank 2
 *   $18 -> Bank 3
 *   $20 -> Bank 4
 *   $28 -> Bank 5
 *   $30 -> Bank 6
 *   $38 -> Bank 7
 *   $01 -> Bank 8
 *   $09 -> Bank 9
 *   $11 -> Bank 10
 *   $19 -> Bank 11
 *   $21 -> Bank 12
 *   $29 -> Bank 13
 *   $31 -> Bank 14
 *   $39 -> Bank 15
 *
 * The following bits are used for bank decoding in $DE00 (0 being the LSB, 3 being the MSB).
 * Bit# 76543210
 *      xx210xx3
 *
 * After copying memory from the ROM banks, the selection program writes a value of $86 to $DE00.
 * This seems either to reset or disable the cartridge ROM.
 */

/**
 * Cartridge type: Super Games (8)
 *
 * Type     Size    Game    EXROM   ROML        ROMH    LOAD ADDRESS
 * ----------------------------------------------------------------------------
 *          64KiB   0       0                           $8000-$BFFF (All modules)
 *
 * 4 banks of 16KiB each.
 *
 * Bank selecting is done by writing to $DF00 as follows:
 *
 *  Bit     Meaning
 *  ---     -------
 *  0       Bank Bit 0
 *  1       Bank Bit 1
 *  2       Mode (0 = EXROM/GAME, bridged on the same wire - 16KiB config;
 *                1 = cartridge disabled)
 *  3       Write-protect-latch (1 = No more changes are possible until the next hardware-reset)
 *  4-7     Unused
 */

/**
 * Cartridge type: Atomic Power (9)
 *
 * Type     Size    Game    EXROM   LOAD ADDRESS
 * ----------------------------------------------------------------------------
 *          32KiB   0       0       $8000-$9FFF (All modules) https://ist.uwaterloo.ca/~schepers/formats/CRT.TXT
 *                  1       0                                 https://vice-emu.sourceforge.io/vice_17.html#SEC404
 *
 * 4 banks of 8KiB modules.
 *
 * Writing to I/O-1 will do the following:
 *
 *  Bit  Meaning
 *  ---  -------
 *  7    Extra ROM bank selector (A15) (unused)
 *  6    1 = Resets FREEZE-mode (turns back to normal mode)
 *  5    1 = Enable RAM at ROML ($8000-$9FFF) & I/O-2 ($DF00-$DFFF = $9F00-$9FFF)
 *  4    ROM bank selector high (A14)
 *  3    ROM bank selector low  (A13)
 *  2    1 = disable cartridge (turn off $DE00)
 *  1    1 = /EXROM high
 *  0    1 = /GAME low
 *
 * If bit 5 (RAM enable) is 1, bit 0,1 (exrom/game) is == 2 (cart off),
 * bit 2,6,7 (cart disable, freeze clear) are 0, then cart ROM (Bank 0..3) is mapped at 8000-9FFF,
 * and cart RAM (Bank 0) is mapped at A000-BFFF and cart RAM (Bank 0) is enabled in the I/O-2 area using
 * 16KiB game config.
 *
 * The cart RAM or ROM is available through a window in the I/O-2 range.
 */

/**
 * Cartridge type: Epyx Fastload (10)
 *
 * Type     Size    Game    EXROM   LOAD ADDRESS
 * ----------------------------------------------------------------------------
 *          8KiB    1       0       $8000-$9FFF https://vice-emu.sourceforge.io/vice_17.html#SEC405
 *          8KiB    1       1       $8000-$9FFF https://ist.uwaterloo.ca/~schepers/formats/CRT.TXT
 *
 * The Epyx FastLoad cart uses a simple capacitor to toggle the ROM on and off.
 * The capacitor is discharged, and 8KiB game config enabled, by either reading ROML or reading I/O-1.
 * If none of those accesses happen the capacitor will charge, and if it is charged (after 512 cycles)
 * then the ROM will get disabled.
 */

/**
 * Cartridge type: Westermann Learning (11)
 *
 * Type     Size    Game    EXROM   LOAD ADDRESS
 * ----------------------------------------------------------------------------
 *          16KiB   0       0       $8000-$BFFF
 *
 * Any read from the I/O-2 range will switch the cart off.
 */

/**
 * Cartridge type: Rex Utility (12)
 *
 * Type     Size    Game    EXROM   LOAD ADDRESS
 * ----------------------------------------------------------------------------
 *          8KiB    1       0       $8000-$9FFF
 *
 * Reading from $DF00-$DFBF disables ROM, reading from $DFC0-$DFFF enables ROM (8KiB game config).
 */

/**
 * Cartridge type: Final Cartridge I (13)
 *
 * Type     Size    Game    EXROM   LOAD ADDRESS
 * ----------------------------------------------------------------------------
 *          16KiB   0       0       $8000-$BFFF https://vice-emu.sourceforge.io/vice_17.html#SEC408
 *          16KiB   1       1       $8000-$BFFF https://ist.uwaterloo.ca/~schepers/formats/CRT.TXT
 *
 * Any access to I/O-1 turns cartridge ROM off. Any access to I/O-2 turns cartridge ROM on.
 * The cart ROM is visible in I/O-1 and I/O-2.
 */

/**
 * Cartridge type: Magic Formel (14)
 *
 * Type     Size    Game    EXROM   LOAD ADDRESS
 * ----------------------------------------------------------------------------
 *          64KiB   0       1       $E000-$FFFF https://vice-emu.sourceforge.io/vice_17.html#SEC409
 *          64KiB   0       0       $E000-$FFFF https://ist.uwaterloo.ca/~schepers/formats/CRT.TXT
 *
 * 8 banks of 8KiB.
 */

/**
 * Cartridge type: C64 Game System, System 3 (15)
 *
 * Type     Size    Game    EXROM   LOAD ADDRESS
 * ----------------------------------------------------------------------------
 *          512KiB  1       0       $8000-$9FFF (all modules)
 *
 * 64 banks of 8KiB each.
 *
 * Known cartridges:
 *  C64GS 4-in-1      (Commodore)  (512 KiB)
 *  Last Ninja Remix  (System 3)   (512 KiB)
 *  Myth              (System 3)   (512 KiB)
 *
 * ROM memory is organized in 8KiB ($2000) banks located at $8000-$9FFF.
 * Bank switching is done by writing to address $DE00+X, where X is the bank number (STA $DE00,X).
 * For instance, to read from bank 3, address $DE03 is written to.
 * Reading from anywhere in the I/O-1 range will disable the cart.
 *
 * The CRT file contains a string of CHIP blocks, each block with a start address of $8000, length $2000
 * and the bank number in the bank field. In the cartridge header, EXROM ($18) is set to 0, GAME ($19) is
 * set to 1 to enable the 8 KiB ROM configuration.
 */

/**
 * Cartridge type: Warp Speed (16)
 *
 * Type     Size    Game    EXROM   LOAD ADDRESS
 * ----------------------------------------------------------------------------
 *          16KiB   0       0       $8000-$BFFF
 *
 * After RESET or POWER ON, 16KiB of cartridge ROM is visible at $8000-$BFFF.
 * Additionally, ROM normally located at $9E00-$9FFF is mirrored into I/O-1 and I/O-2 at $DE00-$DFFF.
 * ROM at $8000-$BFFF is disabled by writing into the I/O-2 area (typically $DF00) and may be re-enabled
 * by writing into I/O-1 ($DE00). However, the $DE00-$DFFF (I/O-1/I/O-2) area itself always remains mapped
 * to cartridge ROM.
 */

/**
 * Cartridge type: Dinamic (17)
 *
 * Type     Size    Game    EXROM   LOAD ADDRESS
 * ----------------------------------------------------------------------------
 *          128KiB  1       0       $8000-$9FFF (All modules)
 *          128KiB (16 banks of 8KiB each)
 *
 * Known DINAMIC cartridges:
 * Narco Police  (128 KiB)
 * Satan         (128 KiB)
 *
 * ROM memory is organized in 8KiB ($2000) banks located at $8000-$9FFF.
 * Bank switching is done by reading from address $DE00+X, where X is the bank number (LDA $DE00,X).
 * For instance, to read from bank 3, address $DE03 is accessed.
 *
 * The CRT file contains a string of CHIP blocks, each block with a start address of $8000, length $2000
 * and the bank number in the bank field. In the cartridge header, EXROM ($18) is set to 0, GAME ($19) is
 * set to 1 to enable the 8 KiB ROM configuration.
 */

/**
 * Cartridge type: Zaxxon, Super Zaxxon (SEGA) (18)
 *
 * Type     Size    Game    EXROM   LOAD ADDRESS
 * ----------------------------------------------------------------------------
 *          20KiB   0       0       $8000-8FFF (Mirrored in $9000-$9FFF, module 0, chip U1)
 *                                  $A000-BFFF (Banked modules 1 and 2, chip U2)
 *
 * The (Super) Zaxxon carts use a 4KiB ($1000) ROM at $8000-$8FFF (mirrored in $9000-$9FFF)
 * along with two 8KiB ($2000) cartridge banks located at $A000-$BFFF.
 * One of the two banks is selected by doing a read access to either the $8000-$8FFF area (bank 0 is selected)
 * or to $9000-$9FFF area (bank 1 is selected). EXROM ($18 = $00) and GAME ($19 = $00) lines are always pulled
 * to GND to select the 16 KiB ROM configuration.
 *
 * The CRT file includes three CHIP blocks:
 * a) bank = 0, load address = $8000, size = $1000
 * b) bank = 0, load address = $A000, size = $2000
 * c) bank = 1, load address = $A000, size = $2000
 */

/**
 * Cartridge type: Magic Desk, Domark, HES Australia (19)
 *
 * Type     Size    Game    EXROM   LOAD ADDRESS
 * ----------------------------------------------------------------------------
 *          32KiB   1       0       $8000-$9FFF (Banks 0-15)
 *          64KiB
 *          128KiB
 *
 * Startup mode 8KiB Game
 * 32KiB, 64KiB or 128KiB sizes (4 to 16 banks of 8KiB each)
 *
 * This cartridge type is very similar to the OCEAN cart type:
 * ROM memory is organized in 8KiB ($2000) banks located at $8000-$9FFF.
 * Bank switching is done by writing the bank number to $DE00.
 * Deviant from the Ocean type, bit 7 is cleared for selecting one of the ROM banks.
 * If bit 7 is set ($DE00 = $80), the GAME/EXROM lines are disabled, turning on RAM at $8000-$9FFF instead of ROM.
 *
 * In the cartridge header, EXROM ($18) is set to 0, GAME ($19) is set to 1 to indicate the RESET/power-up
 * configuration of 8 KiB ROM.
 *
 * Known cartridges:
 * Ghosbusters  (HES Australia) (32 KiB)
 * Magic Desk   (Commodore)     (32 KiB)
 * Badlands     (Domark)        (64 KiB)
 * Vindicators  (Domark)        (64 KiB)
 * Wonderboy    (HES Australia) (64 KiB)
 * Cyberball    (Domark)        (128 KiB)
 */
