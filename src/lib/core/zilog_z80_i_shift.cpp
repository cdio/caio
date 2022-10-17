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

#include "logger.hpp"
#include "types.hpp"
#include "utils.hpp"


namespace caio {

int ZilogZ80::i_RLCA(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * RLCA
     * The contents of the Accumulator (Register A) are rotated left 1 bit position. The sign bit
     * (bit 7) is copied to the Carry flag and also to bit 0. Bit 0 is the least-significant bit.
     * S is not affected.
     * Z is not affected.
     * H is reset.
     * P/V is not affected.
     * N is reset.
     * C is data from bit 7 of Accumulator.
     */
    bool b7 = (self._regs.A & 0x80);
    self._regs.A <<= 1;
    self._regs.A |= (b7 ? 0x01 : 0x00);
    self.flag_C(b7);
    self.flag_H(0);
    self.flag_N(0);
    return 0;
}

int ZilogZ80::i_RRCA(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * RRCA
     * The contents of the Accumulator (Register A) are rotated right 1 bit position.
     * Bit 0 is copied to the Carry flag and also to bit 7. Bit 0 is the least-significant bit.
     * S is not affected.
     * Z is not affected.
     * H is reset.
     * P/V is not affected.
     * N is reset.
     * C is data from bit 0 of Accumulator.
     */
    bool b0 = (self._regs.A & 0x01);
    self._regs.A >>= 1;
    self._regs.A |= (0 ? 0x80 : 0x00);
    self.flag_C(b0);
    self.flag_N(0);
    self.flag_H(0);
    return 0;
}

int ZilogZ80::i_RLA(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * RLA
     * The contents of the Accumulator (Register A) are rotated left 1 bit position
     * through the Carry flag. The previous contents of the Carry flag are copied to bit 0.
     * S is not affected.
     * Z is not affected.
     * H is reset.
     * P/V is not affected.
     * N is reset.
     * C is data from bit 7 of Accumulator.
     */
    bool b7 = (self._regs.A & 0x80);
    self._regs.A <<= 1;
    self._regs.A |= (self.test_C() ? 0x01: 0x00);
    self.flag_C(b7);
    self.flag_H(0);
    self.flag_N(0);
    return 0;
}

int ZilogZ80::i_RRA(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * The contents of the Accumulator (Register A) are rotated right 1 bit position through
     * the Carry flag. The previous contents of the Carry flag are copied to bit 7.
     * S is not affected.
     * Z is not affected.
     * H is reset.
     * P/V is not affected.
     * N is reset.
     * C is data from bit 0 of Accumulator
     */
    bool b0 = (self._regs.A & 0x01);
    self._regs.A >>= 1;
    self._regs.A |= (self.test_C() ? 0x80 : 0x00);
    self.flag_C(b0);
    self.flag_H(0);
    self.flag_N(0);
    return 0;
}

}
