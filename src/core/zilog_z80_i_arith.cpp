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

namespace caio {
namespace zilog {

uint8_t Z80::add8(uint8_t v1, uint8_t v2, uint8_t carry)
{
    /*
     * return v1 + v2 + carry
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if carry from bit 3; otherwise, it is reset.
     * P/V is set if overflow; otherwise, it is reset.
     * N is reset.
     * C is set if carry from bit 7; otherwise, it is reset.
     *
     * Overflow:
     * For addition, operands with different signs never cause overflow.
     * When adding operands with similar signs and the result contains a
     * different sign, the Overflow Flag is set.
     */
    uint8_t lo = (v1 & 0x0F) + (v2 & 0x0F) + carry;
    bool hc = (lo & 0x10);

    uint8_t hi = (v1 >> 4) + (v2 >> 4) + hc;
    bool cy = (hi & 0x10);

    uint8_t result = (hi << 4) | (lo & 0x0F);

    bool s1 = v1 & 0x80;
    bool s2 = v2 & 0x80;
    bool sr = result & 0x80;

    flag_S(sr);
    flag_Z(result == 0);
    flag_H(hc);
    flag_V((s1 && s2 && !sr) || (!s1 && !s2 && sr));
    flag_N(0);
    flag_C(cy);
    flag_Y(result & Flags::Y);
    flag_X(result & Flags::X);

    return result;
}

uint8_t Z80::sub8(uint8_t v1, uint8_t v2, bool borrow)
{
    /*
     * return v1 - v2 - borrow
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if borrow from bit 4; otherwise, it is reset.
     * P/V is set if overflow; otherwise, it is reset.
     * N is set.
     * C is set if borrow; otherwise, it is reset.
     *
     * Overflow:
     * For subtraction, overflow can occur for operands of unalike signs.
     * Operands of alike signs never cause overflow.
     */
    auto result = add8(v1, ~v2, !borrow);
    flag_N(1);
    flag_C(!test_C());
    flag_H(!test_H());
    return result;
}

int Z80::add_A(uint8_t value, bool carry)
{
    /*
     * A = A + value
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if carry from bit 3; otherwise, it is reset.
     * P/V is set if overflow; otherwise, it is reset.
     * N is reset.
     * C is set if carry from bit 7; otherwise, it is reset.
     */
    _regs.A = add8(_regs.A, value, carry);
    return 0;
}

int Z80::sub_A(uint8_t value, bool borrow)
{
    /*
     * A = A - value - borrow
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if borrow from bit 4; otherwise, it is reset.
     * P/V is set if overflow; otherwise, it is reset.
     * N is set.
     * C is set if borrow; otherwise, it is reset.
     */
    _regs.A = sub8(_regs.A, value, borrow);
    return 0;
}

int Z80::cp_A(uint8_t value)
{
    /*
     * A - value
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if borrow from bit 4; otherwise, it is reset.
     * P/V is set if overflow; otherwise, it is reset.
     * N is set.
     * C is set if borrow; otherwise, it is reset.
     */
    sub8(_regs.A, value, 0);
    flag_Y(value & Flags::Y);
    flag_X(value & Flags::X);
    return 0;
}

int Z80::and_A(uint8_t value)
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
    flag_V(parity(result));
    flag_N(0);
    flag_C(0);
    flag_Y(result & Flags::Y);
    flag_X(result & Flags::X);
    _regs.A = result;
    return 0;
}

int Z80::xor_A(uint8_t value)
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
    flag_Y(_regs.A & Flags::Y);
    flag_X(_regs.A & Flags::X);
    return 0;
}

int Z80::or_A(uint8_t value)
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
    flag_V(parity(_regs.A));
    flag_N(0);
    flag_C(0);
    flag_Y(_regs.A & Flags::Y);
    flag_X(_regs.A & Flags::X);
    return 0;
}

int Z80::i_INC_rr(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * INC {BC,DE,HL,SP}    - 03 13 23 33 - 00ss0011
     */
    auto& reg = self.reg16_from_opcode(op);
    ++reg;
    return 0;
}

int Z80::i_DEC_rr(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * DEC {BC,DE,HL,SP}    - 0B 1B 2B 3B - 00ss1011
     */
    auto& reg = self.reg16_from_opcode(op);
    --reg;
    return 0;
}

int Z80::i_INC_r(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * INC {ABCDEHL}    - 3C 04 0C 14 1C 24 2C - 00rrr100
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if carry from bit 3; otherwise, it is reset.
     * P/V is set if r was 7Fh before operation; otherwise, it is reset.
     * N is reset.
     * C is not affected.
     */
    uint8_t& reg = self.reg8_from_opcode(op);
    bool C = self.test_C();
    reg = self.add8(reg, 1, 0);
    self.flag_C(C);
    return 0;
}

int Z80::i_INC_mHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * INC (HL)     - 34
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if carry from bit 3; otherwise, it is reset.
     * P/V is set if (HL) was 7Fh before operation; otherwise, it is reset.
     * N is reset.
     * C is not affected.
     */
    addr_t addr = self._regs.HL;
    uint8_t data = self.read(addr);
    bool C = self.test_C();
    data = self.add8(data, 1, 0);
    self.flag_C(C);
    self.write(addr, data);
    return 0;
}

int Z80::i_DEC_r(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * DEC {ABCDEHL}    - 3D 05 0D 15 1D 25 2D - 00rrr101
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if borrow from bit 4, otherwise, it is reset.
     * P/V is set if r was 80h before operation; otherwise, it is reset.
     * N is set.
     * C is not affected.
     */
    uint8_t& reg = self.reg8_from_opcode(op);
    bool C = self.test_C();
    reg = self.sub8(reg, 1, 0);
    self.flag_C(C);
    return 0;
}

int Z80::i_DEC_mHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * DEC (HL)     - 35
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if borrow from bit 4, otherwise, it is reset.
     * P/V is set if m was 80h before operation; otherwise, it is reset.
     * N is set.
     * C is not affected.
     */
    addr_t addr = self._regs.HL;
    uint8_t data = self.read(addr);
    bool C = self.test_C();
    data = self.sub8(data, 1, 0);
    self.flag_C(C);
    self.write(addr, data);
    return 0;
}

int Z80::i_ADD_HL_rr(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * ADD HL, {BC,DE,HL,SP}    - 09 19 29 39 - 00ss1001
     *
     * S is not affected.
     * Z is not affected.
     * H is set if carry from bit 11; otherwise, it is reset.
     * P/V is not affected.
     * N is reset.
     * C is set if carry from bit 15; otherwise, it is reset.
     */
    auto& src_reg = self.reg16_from_opcode(op);
    uint16_t result = self._regs.HL;
    self._regs.memptr = result + 1;
    self.add16(result, src_reg, 0);
    self._regs.HL = result;
    return 0;
}

int Z80::i_DAA(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * DAA          - 27
     */
    uint8_t prev_A = self._regs.A;
    uint8_t add{};
    bool C{};

    if (self._regs.A > 0x99 || self.test_C()) {
        add = 0x60;
        C = 1;
    }

    if ((self._regs.A & 0xF) > 0x9 || self.test_H()) {
        add += 0x06;
    }

    self._regs.A += (self.test_N() ? -add : add);
    self.flag_C(C);
    self.flag_H((prev_A & 0x10) ^ (self._regs.A & 0x10));
    self.flag_S(self._regs.A & 0x80);
    self.flag_Z(self._regs.A == 0);
    self.flag_V(self.parity(self._regs.A));
    self.flag_Y(self._regs.A & Flags::Y);
    self.flag_X(self._regs.A & Flags::X);
    return 0;
}

int Z80::i_CPL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * CPL          - 2F
     *
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
    self.flag_Y(self._regs.A & Flags::Y);
    self.flag_X(self._regs.A & Flags::X);
    return 0;
}

int Z80::i_SCF(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * SCF          - 37
     *
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

    /* NEC NMOS */
    self.flag_Y(self._regs.A & Flags::Y);
    self.flag_X(self._regs.A & Flags::X);

    return 0;
}

int Z80::i_CCF(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * CCF          - 3F
     *
     * C = ~C
     * S is not affected.
     * Z is not affected.
     * H previous carry is copied.
     * P/V is not affected.
     * N is reset.
     * C is set if C was 0 before operation; otherwise, it is reset
     */
    self.flag_H(self.test_C());
    self.flag_C(!self.test_C());
    self.flag_N(0);

    /* NEC NMOS */
    self.flag_Y(self._regs.A & Flags::Y);
    self.flag_X(self._regs.A & Flags::X);

    return 0;
}

int Z80::i_ADD_A_r(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * ADD A, {ABCDEHL}  - 8F 80 81 82 83 84 85 - 10000rrr
     *
     * A = A + r
     */
    const uint8_t& src_reg = self.reg8_src_from_opcode(op);
    return self.add_A(src_reg, 0);
}

int Z80::i_ADD_A_n(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * ADD A, n     - C6
     *
     * A = A + n
     */
    return self.add_A(arg, 0);
}

int Z80::i_ADD_A_mHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * ADD A, (HL)  - 86
     *
     * A = A + *HL
     */
    uint8_t data = self.read(self._regs.HL);
    return self.add_A(data, 0);
}

int Z80::i_ADC_A_r(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * ADC A, {ABCDEHL} - 8F 88 89 8A 8B 8C 8D - 10001rrr
     *
     * A = A + r + C
     */
    const uint8_t& src_reg = self.reg8_src_from_opcode(op);
    return self.add_A(src_reg, self.test_C());
}

int Z80::i_ADC_A_n(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * ADC A, n     - CE
     *
     * A = A + n + C
     */
    return self.add_A(arg, self.test_C());
}

int Z80::i_ADC_A_mHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * ADC A, (HL)  - 8E
     *
     * A = A + *HL + C
     */
    uint8_t data = self.read(self._regs.HL);
    return self.add_A(data, self.test_C());
}

int Z80::i_SUB_A_r(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * SUB A, {ABCDEHL} - 97 90 91 92 93 94 95
     */
    const uint8_t& src_reg = self.reg8_src_from_opcode(op);
    return self.sub_A(src_reg, 0);
}

int Z80::i_SUB_A_n(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * SUB A, n     - D6
     */
    return self.sub_A(arg, 0);
}

int Z80::i_SUB_A_mHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * SUB A, (HL)  - 96
     */
    uint8_t data = self.read(self._regs.HL);
    return self.sub_A(data, 0);
}

int Z80::i_SBC_A_r(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * SBC A, {ABCDEHL} - 98 99 9A 9B 9C 9D
     */
    const uint8_t& src_reg = self.reg8_src_from_opcode(op);
    return self.sub_A(src_reg, self.test_C());
}

int Z80::i_SBC_A_n(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * SBC A, n     - DE
     */
    return self.sub_A(arg, self.test_C());
}

int Z80::i_SBC_A_mHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * SBC A, (HL)  - 9E
     */
    uint8_t data = self.read(self._regs.HL);
    return self.sub_A(data, self.test_C());
}

int Z80::i_AND_A_r(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * AND A, {ABCDEHL} - A7 A0 A1 A2 A3 A4 A5
     */
    const uint8_t& src_reg = self.reg8_src_from_opcode(op);
    return self.and_A(src_reg);
}

int Z80::i_AND_A_n(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * AND A, n     - E6
     */
    return self.and_A(arg);
}

int Z80::i_AND_A_mHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * AND A, (HL)  - A6
     */
    uint8_t data = self.read(self._regs.HL);
    self.and_A(data);
    return 0;
}

int Z80::i_XOR_A_r(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * XOR A, {ABCDEHL} - AF A8 A9 AA AB AC AD
     */
    const uint8_t& src_reg = self.reg8_src_from_opcode(op);
    return self.xor_A(src_reg);
}

int Z80::i_XOR_A_n(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * XOR A, n     - EE
     */
    return self.xor_A(arg);
}

int Z80::i_XOR_A_mHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * XOR A, (HL)  - AE
     */
    uint8_t data = self.read(self._regs.HL);
    return self.xor_A(data);
}

int Z80::i_OR_A_r(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * OR A, {ABCDEHL}  - B7 B0 B1 B2 B3 B4 B5
     */
    const uint8_t& src_reg = self.reg8_src_from_opcode(op);
    return self.or_A(src_reg);
}

int Z80::i_OR_A_n(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * OR A, n      - F6
     */
    return self.or_A(arg);
}

int Z80::i_OR_A_mHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * OR A, (HL)   - B6
     */
    uint8_t data = self.read(self._regs.HL);
    return self.or_A(data);
}

int Z80::i_CP_A_r(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * CP A, {ABCDEFHL} - BF B8 B9 BA BB BC BD
     */
    uint8_t& reg = self.reg8_src_from_opcode(op);
    return self.cp_A(reg);
}

int Z80::i_CP_A_n(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * CP A, n      - FE
     */
    return self.cp_A(arg & 0xFF);
}

int Z80::i_CP_A_mHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * CP A, (HL)   - BE
     */
    uint8_t data = self.read(self._regs.HL);
    return self.cp_A(data);
}

}
}
