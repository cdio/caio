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
#include "mos_6510.hpp"


namespace cemu {

void Mos6510::i_CLC(Mos6510 &self, addr_t _)
{
    /*
     * CLC  - Clear Carry
     *
     * 2 cycles
     */
    self.flag_C(false);
}

void Mos6510::i_SEC(Mos6510 &self, addr_t _)
{
    /*
     * SEC  - Set Carry
     *
     * 2 cycles
     */
    self.flag_C(true);
}

void Mos6510::i_CLI(Mos6510 &self, addr_t _)
{
    /*
     * CLI  - Clear Interrupt flag
     *
     * 2 cycles.
     */
    self.flag_I(false);
}

void Mos6510::i_SEI(Mos6510 &self, addr_t _)
{
    /*
     * SEI  - Set Interrupt flag
     *
     * 2 cycles
     */
    self.flag_I(true);
}

void Mos6510::i_CLV(Mos6510 &self, addr_t _)
{
    /*
     * CLV  - Clear Overflow flag
     *
     * 2 cycles
     */
    self.flag_V(false);
}

void Mos6510::i_CLD(Mos6510 &self, addr_t)
{
    /*
     * CLD  - Clear Decimal flag
     *
     * 2 cycles
     */
    self.flag_D(false);
}

void Mos6510::i_SED(Mos6510 &self, addr_t)
{
    /*
     * SED  - Set Decimal flag
     *
     * 2 cycles
     */
    self.flag_D(true);
}

}
