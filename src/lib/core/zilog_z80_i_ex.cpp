/*
 * Copyright (C) 2020-2022 Claudio Castiglia
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


namespace caio {

int ZilogZ80::i_EX_AF_sAF(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    uint16_t aAF = self._regs.aAF;
    self._regs.aAF = self._regs.AF;
    self._regs.AF = aAF;
    return 0;
}

int ZilogZ80::i_EX_DE_HL(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    uint16_t HL = self._regs.HL;
    self._regs.HL = self._regs.DE;
    self._regs.DE = HL;
    return 0;
}

int ZilogZ80::i_EXX(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    uint16_t tmp = self._regs.aBC;
    self._regs.aBC = self._regs.BC;
    self._regs.BC = tmp;

    tmp = self._regs.aDE;
    self._regs.aDE = self._regs.DE;
    self._regs.DE = tmp;

    tmp = self._regs.aHL;
    self._regs.aHL = self._regs.HL;
    self._regs.HL = tmp;

    return 0;
}

int ZilogZ80::i_EX_mSP_HL(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    uint16_t HL = self._regs.HL;
    self._regs.HL = self.pop_addr();
    self.push_addr(HL);
    return 0;
}

}
