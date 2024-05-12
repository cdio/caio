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

int Z80::i_HALT(Z80& self, uint8_t, addr_t)
{
    /*
     * HALT         - 76
     */
    if (self._halt_pin) {
        /*
         * The CPU is already halted but the PC points to the
         * next instruction: Make it point to the HALT opcode.
         */
        --self._regs.PC;
    } else {
        self.halt();
    }
    return 0;
}

int Z80::i_NOP(Z80&, uint8_t, addr_t)
{
    /*
     * NOP          - 00
     */
    return 0;
}

int Z80::i_DJNZ(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * DJNZ $rel    - 10
     *
     * B = B - 1
     * If B != 0: PC = PC + arg
     */
    --self._regs.B;
    if (self._regs.B != 0) {
        self.take_branch(arg);
        return 0;
    }
    return 0x00020008;  /* 2 M cycles, 8 T states */
}

int Z80::i_JR(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * JR $rel      - 18
     */
    self.take_branch(arg);
    return 0;
}

int Z80::i_JR_NZ(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * JR NZ, $rel  - 20
     */
    if (!self.test_Z()) {
        self.take_branch(arg);
        return 0;
    }
    return 0x00020007;
}

int Z80::i_JR_Z(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * JR Z, $rel   - 28
     */
    if (self.test_Z()) {
        self.take_branch(arg);
        return 0;
    }
    return 0x00020007;
}

int Z80::i_JR_NC(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * JR NC, $rel  - 30
     */
    if (!self.test_C()) {
        self.take_branch(arg);
        return 0;
    }
    return 0x00020007;
}

int Z80::i_JR_C(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * JR C, $rel   - 38
     */
    if (self.test_C()) {
        self.take_branch(arg);
        return 0;
    }
    return 0x00020007;
}

bool Z80::test_cond_from_opcode(uint8_t op)
{
    /*
     * XXcccXXX
     *   |||
     *   000 = NZ  (Non Zero)
     *   001 =  Z  (Zero)
     *   010 = NC  (No Carry)
     *   011 =  C  (Carry)
     *   100 = PO  (Parity Odd)
     *   101 = PE  (Parity Even)
     *   110 = P   (sign positive)
     *   111 = M   (sign negative)
     */
    constexpr static uint8_t COND_MASK = 0x38;
    switch (op & COND_MASK) {
    case 0x00:  /* NZ */
        return !test_Z();

    case 0x08:  /* Z */
        return test_Z();

    case 0x10:  /* NC */
        return !test_C();

    case 0x18:  /* C */
        return test_C();

    case 0x20:  /* Odd Parity */
        return !test_V();

    case 0x28:  /* Even Parity */
        return test_V();

    case 0x30:  /* Positive */
        return !test_S();

    case 0x38:  /* Negative */
    default:
        return test_S();
    }
}

int Z80::i_RET(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * RET          - C9
     */
    self._regs.PC = self.pop_addr();
    self._regs.memptr = self._regs.PC;
    return 0;
}

int Z80::i_RET_cc(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * RET NZ
     * RET Z
     * RET NC
     * RET C
     * RET PO
     * RET PE
     * RET P
     * RET M
     *
     * 11ccc000
     *   |||
     *   000 = NZ  (Non Zero)
     *   001 =  Z  (Zero)
     *   010 = NC  (No Carry)
     *   011 =  C  (Carry)
     *   100 = PO  (Parity Odd)
     *   101 = PE  (Parity Even)
     *   110 = P   (sign positive)
     *   111 = M   (sign negative)
     */
    bool cond = self.test_cond_from_opcode(op);
    if (cond) {
        self._regs.PC = self.pop_addr();
        self._regs.memptr = self._regs.PC;
        return 0;
    }

    return 0x00010005;
}

int Z80::i_JP_nn(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * JP nn        - C3
     */
    self._regs.memptr = arg;
    self._regs.PC = arg;
    return 0;
}

int Z80::i_JP_cc_nn(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * JP NZ nn
     * JP Z  nn
     * JP NC nn
     * JP C  nn
     * JP PO nn
     * JP PE nn
     * JP P  nn
     * JP M  nn
     *
     * 11ccc010 LLLLLLLL HHHHHHHH
     *   |||
     *   000 = NZ  (Non Zero)
     *   001 =  Z  (Zero)
     *   010 = NC  (No Carry)
     *   011 =  C  (Carry)
     *   100 = PO  (Parity Odd)
     *   101 = PE  (Parity Even)
     *   110 = P   (sign positive)
     *   111 = M   (sign negative)
     */
    self._regs.memptr = arg;
    bool cond = self.test_cond_from_opcode(op);
    if (cond) {
        self._regs.PC = arg;
    }

    return 0;
}

int Z80::i_JP_HL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * JP HL        - E9
     */
    self._regs.PC = self._regs.HL;
    return 0;
}

int Z80::i_CALL_nn(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * CALL nn          - CD
     */
    self._regs.memptr = arg;
    return self.call(arg);
}

int Z80::i_CALL_cc_nn(Z80& self, uint8_t op, addr_t addr)
{
    /*
     * CALL NZ nn
     * CALL Z  nn
     * CALL NC nn
     * CALL C  nn
     * CALL PO nn
     * CALL PE nn
     * CALL P  nn
     * CALL M  nn
     *
     * 11ccc100 LLLLLLLL HHHHHHHH
     *   |||
     *   000 = NZ  (Non Zero)
     *   001 =  Z  (Zero)
     *   010 = NC  (No Carry)
     *   011 =  C  (Carry)
     *   100 = PO  (Parity Odd)
     *   101 = PE  (Parity Even)
     *   110 = P   (sign positive)
     *   111 = M   (sign negative)
     */
    self._regs.memptr = addr;
    bool cond = self.test_cond_from_opcode(op);
    if (cond) {
        return self.call(addr);
    }

    /* No branch: 3M 10T */
    return 0x00030010;
}

int Z80::i_RST_p(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * RST $00      - C7
     * RST $08      - CF
     * RST $10      - D7
     * RST $18      - DF
     * RST $20      - E7
     * RST $28      - EF
     * RST $30      - F7
     * RST $38      - FF
     *
     * RST $n       - 11nnn111
     */
    constexpr static uint8_t RST_ADDR_MASK = 0x38;
    addr_t addr = op & RST_ADDR_MASK;
    return i_CALL_nn(self, op, addr);
}

}
}
