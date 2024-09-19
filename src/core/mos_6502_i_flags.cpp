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
#include "mos_6502.hpp"

namespace caio {
namespace mos {

int Mos6502::i_CLC(Mos6502& self, addr_t)
{
    /*
     * Clear Carry
     * CLC              - 18 - 2 cycles
     */
    self.flag_C(false);
    return 0;
}

int Mos6502::i_SEC(Mos6502& self, addr_t)
{
    /*
     * Set Carry
     * SEC              - 38 - 2 cycles
     */
    self.flag_C(true);
    return 0;
}

int Mos6502::i_CLI(Mos6502& self, addr_t)
{
    /*
     * Clear Interrupt flag
     * CLI              - 58 - 2 cycles.
     */
    self.flag_I(false);
    return 0;
}

int Mos6502::i_SEI(Mos6502& self, addr_t)
{
    /*
     * Set Interrupt flag
     * SEI              - 78 - 2 cycles
     */
    self.flag_I(true);
    return 0;
}

int Mos6502::i_CLV(Mos6502& self, addr_t)
{
    /*
     * Clear Overflow flag
     * CLV              - B8 - 2 cycles
     */
    self.flag_V(false);
    return 0;
}

int Mos6502::i_CLD(Mos6502& self, addr_t)
{
    /*
     * Clear Decimal flag
     * CLD              - D8 - 2 cycles
     */
    self.flag_D(false);
    return 0;
}

int Mos6502::i_SED(Mos6502& self, addr_t)
{
    /*
     * Set Decimal flag
     * SED              - F8 - 2 cycles
     */
    self.flag_D(true);
    return 0;
}

}
}
