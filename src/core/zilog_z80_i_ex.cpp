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
#include "zilog_z80.hpp"

#include <algorithm>


namespace caio {
namespace zilog {

int Z80::i_EX_AF_sAF(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * EX AF, AF'   - 08
     * Swap AF, AF'
     */
    std::swap(self._regs.aAF, self._regs.AF);
    return 0;
}

int Z80::i_EX_DE_HL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * EX DE, HL    - EB
     * Swap DE, HL
     */
    std::swap(self._regs.HL, self._regs.DE);
    return 0;
}

int Z80::i_EXX(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * EXX          - D9
     * Swap BC, BC'
     * Swap DE, DE'
     * Swap HL, HL'
     */
    std::swap(self._regs.aBC, self._regs.BC);
    std::swap(self._regs.aDE, self._regs.DE);
    std::swap(self._regs.aHL, self._regs.HL);
    return 0;
}

int Z80::i_EX_mSP_HL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * EX (SP), HL  - E3
     * Swap *SP, HL
     */
    uint16_t val = self.read_addr(self._regs.SP);
    std::swap(val, self._regs.HL);
    self.write_addr(self._regs.SP, val);
    self._regs.memptr = self._regs.HL;
    return 0;
}

}
}
