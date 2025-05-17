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

int Mos6502::i_BPL(Mos6502& self, addr_t rel)
{
    /*
     * Branch (jump) relative if positive
     * BPL $r8          - 10 - 2 cycles, (3 if branched to same page, 4 if branched to another page)
     */
    return (!self.test_N() ? self.take_branch(rel) : 0);
}

int Mos6502::i_BMI(Mos6502& self, addr_t rel)
{
    /*
     * Branch (jump) relative if negative
     * BMI $r8          - 30 - 2 cycles, (3 if branched to same page, 4 if branched to another page)
     */
    return (self.test_N() ? self.take_branch(rel) : 0);
}

int Mos6502::i_BVC(Mos6502& self, addr_t rel)
{
    /*
     * Branch (jump) relative if overflow is not set
     * BVC $r8          - 50 - 2 cycles, (3 if branched to same page, 4 if branched to another page)
     */
    return (!self.test_V() ? self.take_branch(rel) : 0);
}

int Mos6502::i_BVS(Mos6502& self, addr_t rel)
{
    /*
     * Branch (jump) relative if overflow is set
     * BVS $r8          - 70 - 2 cycles, (3 if branched to same page, 4 if branched to another page)
     */
    return (self.test_V() ? self.take_branch(rel) : 0);
}

int Mos6502::i_BCC(Mos6502& self, addr_t rel)
{
    /*
     * Branch (jump) relative if carry is not set
     * BCC $r8          - 90 - 2 cycles, (3 if branched to same page, 4 if branched to another page)
     */
    return (!self.test_C() ? self.take_branch(rel) : 0);
}

int Mos6502::i_BCS(Mos6502& self, addr_t rel)
{
    /*
     * Branch (jump) relative if carry is set
     * BCS $r8          - B0 - 2 cycles, (3 if branched to same page, 4 if branched to another page)
     */
    return (self.test_C() ? self.take_branch(rel) : 0);
}

int Mos6502::i_BNE(Mos6502& self, addr_t rel)
{
    /*
     * Branch (jump) relative if not zero
     * BNE $r8          - D0 - 2 cycles, (3 if branched to same page, 4 if branched to another page)
     */
    return (!self.test_Z() ? self.take_branch(rel) : 0);
}

int Mos6502::i_BEQ(Mos6502& self, addr_t rel)
{
    /*
     * Branch (jump) relative if zero
     * BEQ $r8          - F0 - 2 cycles, (3 if branched to same page, 4 if branched to another page)
     */
    return (self.test_Z() ? self.take_branch(rel) : 0);
}

int Mos6502::i_BRK(Mos6502& self, addr_t)
{
    /*
     * Software interrupt
     * BRK              - 00 - 7 cycles
     * push(PC + 2)
     * push(P | Flags::B)
     * P |= Flags::I
     * PC = *($FFFE)
     */
    self.push_addr(self._regs.PC + 1);
    self.push(self._regs.P | Flags::B);
    self.flag(Flags::I);
    self._regs.PC = self.read_addr(vIRQ);
    return 0;
}

int Mos6502::i_RTI(Mos6502& self, addr_t)
{
    /*
     * Return from interrupt
     * RTI              - 40 - 6 cycles
     * P = pop() & ~Flags::B
     * PC = pop()
     */
    self.read(S_base | self._regs.S);    /* Dummy read from stack */
    self._regs.P = (self.pop() & ~Flags::B) | Flags::_;
    self._regs.PC = self.pop_addr();
    return 0;
}

int Mos6502::i_JSR(Mos6502& self, addr_t addr)
{
    /*
     * Jump to subroutine
     * JSR $0000        - 20 - 6 cycles
     */
    self.read(S_base | self._regs.S);   /* Dummy read from stack */
    self.push_addr(self._regs.PC - 1);  /* The pushed value is the last byte of the JSR instruction */
    self._regs.PC = addr;
    return 0;
}

int Mos6502::i_RTS(Mos6502& self, addr_t)
{
    /*
     * Return from subroutine
     * RTS              - 60 - 6 cycles
     */
    self.read(S_base | self._regs.S);   /* Dummy read from stack */
    const addr_t ra = self.pop_addr();
    self.read(ra);                      /* Dummy read from new PC - 1 */
    self._regs.PC = ra + 1;             /* The popped value is the last byte of the JSR instruction */
    return 0;
}

int Mos6502::i_JMP(Mos6502& self, addr_t addr)
{
    /*
     * Jump to absolute address
     * JMP $0000        - 4C - 3 cycles
     * JMP ($0000)      - 6C - 5 cycles
     */
    self._regs.PC = addr;
    return 0;
}

int Mos6502::i_NOP(Mos6502& self, addr_t)
{
    /*
     * No Operation (jump to next instruction)
     * NOP              - 04 1A 3A 5A 7A DA EA FA - 2 cycles
     * NOP $0000        - 0C                      - 4 cycles
     * NOP $00, X       - 14 34 54 74 D4 F4       - 4 cycles
     * NOP $0000, X     - 1C 3C 5C 7C DC FC       - 4 cycles
     * NOP $00          - 44 64                   - 4 cycles
     * NOP #$00         - 80 82 89 C2 E2          - 4 cycles
     */
    return 0;
}

}
}
