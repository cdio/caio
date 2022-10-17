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

#include <chrono>
#include <iomanip>


namespace caio {

int ZilogZ80::i_INC_rr(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * INC BC
     * INC DE
     * INC HL
     * INC SP
     */
    uint16_t &reg = self.reg16_from_opcode(op);
    ++reg;
    return 0;
}

int ZilogZ80::i_DEC_rr(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * DEC BC
     * DEC DE
     * DEC HL
     * DEC SP
     */
    uint16_t &reg = self.reg16_from_opcode(op);
    --reg;
    return 0;
}

int ZilogZ80::i_INC_r(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * INC {ABCDEHL}
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if carry from bit 3; otherwise, it is reset.
     * P/V is set if r was 7Fh before operation; otherwise, it is reset.
     * N is reset.
     * C is not affected.
     */
    uint8_t &reg = self.reg8_from_opcode(op);
    uint8_t result = reg + 1;
    self.flag_S(result & 0x80);
    self.flag_Z(result == 0);
    self.flag_H((reg & 0x10) ^ (result & 0x10));
    self.flag_V(reg == 0x7F);
    self.flag_N(0);
    reg = result;
    return 0;
}

int ZilogZ80::i_DEC_r(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * DEC {ABCDEHL}
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if borrow from bit 4, otherwise, it is reset.
     * P/V is set if r was 80h before operation; otherwise, it is reset.
     * N is set.
     * C is not affected.
     */
    uint8_t &reg = self.reg8_from_opcode(op);
    uint8_t result = reg - 1;
    self.flag_S(result & 0x80);
    self.flag_Z(result == 0);
    self.flag_H(!((reg & 0x10) ^ (result & 0x10)));
    self.flag_V(reg == 0x80);
    self.flag_N(1);
    reg = result;
    return 0;
}

int ZilogZ80::i_INC_mHL(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * INC (HL)
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if carry from bit 3; otherwise, it is reset.
     * P/V is set if (HL) was 7Fh before operation; otherwise, it is reset.
     * N is reset.
     * C is not affected.
     */
    addr_t addr = self._regs.HL;
    uint8_t data = self.read(addr);
    int result = static_cast<int>(data) + 1;
    self.write(addr, static_cast<uint8_t>(result));
    self.flag_S(result & 0x80);
    self.flag_Z(result == 0);
    self.flag_H((data & 0x10) ^ (result & 0x10));
    self.flag_V(data == 0x7F);
    self.flag_N(0);
    return 0;
}

int ZilogZ80::i_DEC_mHL(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * DEC (HL)
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if borrow from bit 4, otherwise, it is reset.
     * P/V is set if m was 80h before operation; otherwise, it is reset.
     * N is set.
     * C is not affected.
     */
    addr_t addr = self._regs.HL;
    uint8_t data = self.read(addr);
    int result = static_cast<int>(data) - 1;
    self.write(addr, static_cast<uint8_t>(result));
    self.flag_S(result & 0x80);
    self.flag_Z(result == 0);
    self.flag_H(!((data & 0x10) ^ (result & 0x10)));
    self.flag_V(data == 0x80);
    self.flag_N(1);
    return 0;
}

int ZilogZ80::i_ADD_HL_rr(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * ADD HL, BC
     * ADD HL, DE
     * ADD HL, HL
     * ADD HL, SP
     * S is not affected.
     * Z is not affected.
     * H is set if carry from bit 11; otherwise, it is reset.
     * P/V is not affected.
     * N is reset.
     * C is set if carry from bit 15; otherwise, it is reset.
     */
    uint16_t reg = self.reg16_from_opcode(op);
    int result = static_cast<int>(self._regs.HL) + static_cast<int>(reg);
    self.flag_H((self._regs.HL & 0x0010) ^ (result & 0x0010));
    self.flag_N(0);
    self.flag_C(result & 0x10000);
    self._regs.HL = static_cast<uint16_t>(result & 0xFFFF);
    return 0;
}

int ZilogZ80::i_DAA(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    uint8_t prev_A = self._regs.A;
    uint8_t add{};
    bool C{};

    if (self._regs.A > 0x99 || self.test_C()) {
        add = 0x60;
        C = 0;
    }

    if ((self._regs.A & 0xF) > 0x9 || self.test_H()) {
        add = 0x06;
    }

    self._regs.A += (self.test_N() ? -add : add);
    self.flag_C(C);
    self.flag_H((prev_A & 0x10) ^ (self._regs.A & 0x10));
    self.flag_S(self._regs.A & 0x80);
    self.flag_Z(self._regs.A == 0);
    self.flag_V(self.parity(self._regs.A));
    return 0;
}

int ZilogZ80::i_CPL(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * A is inverted.
     *
     * S is not affected.
     * Z is not affected.
     * H is set.
     * P/V is not affected.
     * N is set.
     * C is not affected.
     */
    self._regs.A = ~self._regs.A;
    self.flag_H(1);
    self.flag_N(1);
    return 0;
}

int ZilogZ80::i_SCF(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * C = 1
     * S is not affected.
     * Z is not affected.
     * H is reset.
     * P/V is not affected.
     * N is reset.
     * C is set.
     */
    self.flag_C(1);
    self.flag_H(0);
    self.flag_N(0);
    return 0;
}

int ZilogZ80::i_CCF(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * C = ~C
     * S is not affected.
     * Z is not affected.
     * H, previous carry is copied.
     * P/V is not affected.
     * N is reset.
     * C is set if CY was 0 before operation; otherwise, it is reset
     */
    self.flag_H(self.test_C());
    self.flag_N(0);
    self.flag_C(self.test_C() ^ 1);
    return 0;
}

int ZilogZ80::add_A(int value)
{
    /*
     * A = A + value
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if carry from bit 3; otherwise, it is reset.
     * P/V is set if overflow; otherwise, it is reset.
     * N is reset.
     * C is set if carry from bit 7; otherwise, it is reset.
     */
    int result = static_cast<int>(_regs.A) + value;
    flag_S(result & 0x80);
    flag_Z(result == 0);
    flag_H((_regs.A & 0x10) ^ (result & 0x10));
    flag_V(test_C() ^ (result & 0x100));
    flag_N(0);
    flag_C(result & 0x100);
    return 0;
}

int ZilogZ80::i_ADD_A_r(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * ADD A, {BCDEHL}
     * A = A + r
     */
    const uint8_t &src_reg = self.reg8_src_from_opcode(op);
    return self.add_A(src_reg);
}

int ZilogZ80::i_ADD_A_n(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * ADD A, n
     * A = A + n
     */
    return self.add_A(arg);
}

int ZilogZ80::i_ADD_A_mHL(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * ADD A, (HL)
     * A = A + *HL
     */
    uint8_t data = self.read(self._regs.HL);
    return self.add_A(data);
}

int ZilogZ80::i_ADC_A_r(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * ADC A, {BCDEHL}
     * A = A + r + C
     */
    const uint8_t &src_reg = self.reg8_src_from_opcode(op);
    int value = static_cast<int>(src_reg) + static_cast<int>(self._regs.C);
    return self.add_A(value);
}

int ZilogZ80::i_ADC_A_n(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * ADC A, n
     * A = A + n + C
     */
    int value = static_cast<int>(arg) + static_cast<int>(self._regs.C);
    return self.add_A(value);
}

int ZilogZ80::i_ADC_A_mHL(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * ADC A, (HL)
     * A = A + *HL + C
     */
    uint8_t data = self.read(self._regs.HL);
    int value = static_cast<int>(data) + static_cast<int>(self._regs.C);
    return self.add_A(value);
}

int ZilogZ80::sub_A(int value)
{
    /*
     * A = A - value
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if borrow from bit 4; otherwise, it is reset.
     * P/V is set if overflow; otherwise, it is reset.
     * N is set.
     * C is set if borrow; otherwise, it is reset.
     */
    add_A(-value);
    flag_N(1);
    flag_H(!test_H());
    flag_C(!test_C());
    return 0;
}

int ZilogZ80::i_SUB_A_r(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * SUB A, {ABCDEHL}
     */
    const uint8_t &src_reg = self.reg8_src_from_opcode(op);
    return self.sub_A(src_reg);
}

int ZilogZ80::i_SUB_A_n(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * SUB A, n
     */
    return self.sub_A(arg);
}

int ZilogZ80::i_SUB_A_mHL(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * SUB A, (HL)
     */
    uint8_t data = self.read(self._regs.HL);
    return self.sub_A(data);
}

int ZilogZ80::i_SBC_A_r(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * SBC A, {ABCDEHL}
     */
    const uint8_t &src_reg = self.reg8_src_from_opcode(op);
    int value = static_cast<int>(src_reg) + (!self.test_C());
    return self.sub_A(value);
}

int ZilogZ80::i_SBC_A_n(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * SBC A, n
     */
    int value = static_cast<int>(arg) + (!self.test_C());
    return self.sub_A(value);
}

int ZilogZ80::i_SBC_A_mHL(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * SBC A, (HL)
     */
    uint8_t data = self.read(self._regs.HL);
    int value = static_cast<int>(data) + (!self.test_C());
    return self.sub_A(value);
}

int ZilogZ80::and_A(uint8_t value)
{
    /*
     * A = A & value
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set.
     * P/V is reset if overflow; otherwise, it is reset.
     * N is reset.
     * C is reset
     */
    uint8_t result = _regs.A & value;
    flag_S(result & 0x80);
    flag_Z(result == 0);
    flag_H(1);
    flag_V(parity(result)); //XXX check
    flag_N(0);
    flag_C(0);
    _regs.A = result;
    return 0;
}

int ZilogZ80::i_AND_A_r(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * AND A, {ABCDEHL}
     */
    const uint8_t &src_reg = self.reg8_src_from_opcode(op);
    return self.and_A(src_reg);
}

int ZilogZ80::i_AND_A_n(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * AND A, n
     */
    return self.and_A(arg);
}

int ZilogZ80::i_AND_A_mHL(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * AND A, (HL)
     */
    uint8_t data = self.read(self._regs.HL);
    self.and_A(data);
    return 0;
}

int ZilogZ80::xor_A(uint8_t value)
{
    /*
     * A = A ^ value
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is reset.
     * P/V is set if parity even; otherwise, it is reset.
     * N is reset.
     * C is reset.
     */
    _regs.A ^= value;
    flag_S(_regs.A & 0x80);
    flag_Z(_regs.A == 0);
    flag_H(0);
    flag_V(parity(_regs.A));
    flag_N(0);
    flag_C(0);
    return 0;
}

int ZilogZ80::i_XOR_A_r(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * XOR A, {ABCDEHL}
     */
    const uint8_t &src_reg = self.reg8_src_from_opcode(op);
    return self.xor_A(src_reg);
}

int ZilogZ80::i_XOR_A_n(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * XOR A, m
     */
    return self.xor_A(arg);
}

int ZilogZ80::i_XOR_A_mHL(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * XOR A, (HL)
     */
    uint8_t data = self.read(self._regs.HL);
    return self.xor_A(data);
}

int ZilogZ80::or_A(uint8_t value)
{
    /*
     * A = A | value
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is reset.
     * P/V is set if overflow; otherwise, it is reset.
     * N is reset.
     * C is reset.
     */
    _regs.A |= value;
    flag_S(_regs.A & 0x80);
    flag_Z(_regs.A == 0);
    flag_H(0);
    flag_V(parity(_regs.A));  //Check if V or P XXX
    flag_N(0);
    flag_C(0);
    return 0;
}

int ZilogZ80::i_OR_A_r(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * OR A, {ABCDEHL}
     */
    const uint8_t &src_reg = self.reg8_src_from_opcode(op);
    return self.or_A(src_reg);
}

int ZilogZ80::i_OR_A_n(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * OR A, n
     */
    return self.or_A(arg);
}

int ZilogZ80::i_OR_A_mHL(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * OR A, (HL)
     */
    uint8_t data = self.read(self._regs.HL);
    return self.or_A(data);
}

int ZilogZ80::cp_A(int value)
{
    /*
     * A - value
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if borrow from bit 4; otherwise, it is reset.
     * P/V is set if overflow; otherwise, it is reset.
     * N is set.
     * C is set if borrow; otherwise, it is reset.
     */
    int result = static_cast<int>(_regs.A) - value;
    flag_S(result & 0x80);
    flag_Z(result == 0);
    flag_H(!((_regs.A & 0x10) ^ (result & 0x10)));
    flag_V(test_C() ^ (result & 0x100));
    flag_N(1);
    flag_C(!(result & 0x100));
    return 0;
}

int ZilogZ80::i_CP_A_r(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * CP A, {ABCDEFHL}
     */
    const uint8_t &src_reg = self.reg8_src_from_opcode(op);
    return self.cp_A(src_reg);
}

int ZilogZ80::i_CP_A_n(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * CP A, n
     */
    return self.cp_A(arg);
}

int ZilogZ80::i_CP_A_mHL(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * CP A, (HL)
     */
    uint8_t data = self.read(self._regs.HL);
    return self.cp_A(data);
}

}
