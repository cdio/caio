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

int Z80::i_LD_rr_nn(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD rp, nn
     * LD BC, nn    - 01
     * LD DE, nn    - 11
     * LD HL, nn    - 21
     * LD SP, nn    - 31
     *
     * 00dd0001
     */
    auto& dst_reg = self.reg16_from_opcode(op);
    dst_reg = arg;
    return 0;
}

int Z80::i_LD_r_n(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD A, n      - 3E
     * LD B, n      - 06
     * LD C, n      - 0E
     * LD D, n      - 16
     * LD E, n      - 1E
     * LD H, n      - 26
     * LD L, n      - 2E
     *
     * 00rrr110
     *   |||
     *   000 = B
     *   001 = C
     *   010 = D
     *   011 = E
     *   100 = H
     *   101 = L
     *   110 = -    (ignored)
     *   111 = A
     */
    uint8_t& reg = self.reg8_from_opcode(op);
    reg = arg;
    return 0;
}

int Z80::i_LD_r_r(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD {ABCDEHL}, {ABCDEHL}  - 7F 78 79 7A 7B 7C 7D
     * LD d, s                  - 01dddsss
     */
    uint8_t& dst_reg = self.reg8_from_opcode(op);
    uint8_t& src_reg = self.reg8_src_from_opcode(op);
    dst_reg = src_reg;
    return 0;
}

int Z80::i_LD_r_mHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD {ABCDEHL}, (HL)
     */
    uint8_t& dst_reg = self.reg8_from_opcode(op);
    uint8_t data = self.read(self._regs.HL);
    dst_reg = data;
    return 0;
}

int Z80::i_LD_mHL_r(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD (HL), {ABCDEHL}       - 77 70 71 72 73 74 75
     * LD (HL), r               - 01110rrr
     */
    uint8_t& src_reg = self.reg8_src_from_opcode(op);
    self.write(self._regs.HL, src_reg);
    return 0;
}

int Z80::i_LD_A_mdd(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD A, (BC)   - 0A
     * LD A, (DE)   - 1A
     * LD A, (nn)   - 3A
     */
    constexpr static const uint8_t LD_A_BC = 0x0A;
    constexpr static const uint8_t LD_A_DE = 0x1A;
    addr_t addr = (op == LD_A_BC ? self._regs.BC : (op == LD_A_DE ? self._regs.DE : arg));
    uint8_t data = self.read(addr);
    self._regs.A = data;
    self._regs.memptr = addr + 1;
    return 0;
}

int Z80::i_LD_mdd_A(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD (BC), A   - 02
     * LD (DE), A   - 12
     * LD (nn), A   - 32
     */
    constexpr static const uint8_t LD_BC_A = 0x02;
    constexpr static const uint8_t LD_DE_A = 0x12;
    addr_t addr = (op == LD_BC_A ? self._regs.BC : (op == LD_DE_A ? self._regs.DE : arg));
    self.write(addr, self._regs.A);
    self._regs.memptr = (static_cast<uint16_t>(self._regs.A) << 8) | ((addr + 1) & 255);
    return 0;
}

int Z80::i_LD_mnn_HL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD (nn), HL  - 22
     */
    self.write(arg, self._regs.L);
    self.write(arg + 1, self._regs.H);
    self._regs.memptr = arg + 1;
    return 0;
}

int Z80::i_LD_HL_mnn(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD HL, (nn)  - 2A
     */
    self._regs.L = self.read(arg);
    self._regs.H = self.read(arg + 1);
    self._regs.memptr = arg + 1;
    return 0;
}

int Z80::i_LD_mHL_n(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD (HL), n   - 36
     */
    self.write(self._regs.HL, arg);
    return 0;
}

int Z80::i_LD_SP_HL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD SP, HL    - F9
     */
    self._regs.SP = self._regs.HL;
    return 0;
}

int Z80::i_POP_rr(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * POP {BC,DE,HL,AF}    - C1 D1 E1 F1
     *
     * 11qq0001
     *   ||
     *   00 = BC
     *   01 = DE
     *   10 = HL
     *   11 = AF
     */
    auto& dst_reg = self.reg16_from_opcode(op, true /* no SP */);
    dst_reg = self.pop_addr();
    return 0;
}

int Z80::i_PUSH_rr(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * PUSH {BC,DE,HL,AF}   - C5 D5 E5 F5
     *
     * 11qq0101
     *   ||
     *   00 = BC
     *   01 = DE
     *   10 = HL
     *   11 = AF
     */
    auto& src_reg = self.reg16_from_opcode(op, true /* no SP */);
    self.push_addr(src_reg);
    return 0;
}

}
}
