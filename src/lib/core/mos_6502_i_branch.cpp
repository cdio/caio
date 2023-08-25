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

/********************************************************************************
 * BPL (Branch relative if positive)
 ********************************************************************************/
int Mos6502::i_BPL(Mos6502& self, addr_t rel)
{
    /*
     * BPL $r8
     *
     * 2 cycles, (3 if branched to same page, 4 if branched to another page)
     */
    if (!self.test_N()) {
        self.take_branch(rel);
    }

    return 0;
}


/********************************************************************************
 * BMI (Branch relative if negative)
 ********************************************************************************/
int Mos6502::i_BMI(Mos6502& self, addr_t rel)
{
    /*
     * BMI $r8
     *
     * 2 cycles, (3 if branched to same page, 4 if branched to another page)
     */
    if (self.test_N()) {
        self.take_branch(rel);
    }

    return 0;
}


/********************************************************************************
 * BVC (Branch relative if not overflow)
 ********************************************************************************/
int Mos6502::i_BVC(Mos6502& self, addr_t rel)
{
    /*
     * BVC $r8
     *
     * 2 cycles, (3 if branched to same page, 4 if branched to another page)
     */
    if (!self.test_V()) {
        self.take_branch(rel);
    }

    return 0;
}


/********************************************************************************
 * BVS (Branch relative if overflow)
 ********************************************************************************/
int Mos6502::i_BVS(Mos6502& self, addr_t rel)
{
    /*
     * BVS $r8
     *
     * 2 cycles, (3 if branched to same page, 4 if branched to another page)
     */
    if (self.test_V()) {
        self.take_branch(rel);
    }

    return 0;
}


/********************************************************************************
 * BCC (Branch relative if carry is not set)
 ********************************************************************************/
int Mos6502::i_BCC(Mos6502& self, addr_t rel)
{
    /*
     * BCC $r8
     *
     * 2 cycles, (3 if branched to same page, 4 if branched to another page)
     */
    if (!self.test_C()) {
        self.take_branch(rel);
    }

    return 0;
}


/********************************************************************************
 * BCS (Branch relative if carry is set)
 ********************************************************************************/
int Mos6502::i_BCS(Mos6502& self, addr_t rel)
{
    /*
     * BCS $r8
     *
     * 2 cycles, (3 if branched to same page, 4 if branched to another page)
     */
    if (self.test_C()) {
        self.take_branch(rel);
    }

    return 0;
}


/********************************************************************************
 * BNE (Branch relative if not zero)
 ********************************************************************************/
int Mos6502::i_BNE(Mos6502& self, addr_t rel)
{
    /*
     * BNE $r8
     *
     * 2 cycles, (3 if branched to same page, 4 if branched to another page)
     */
    if (!self.test_Z()) {
        self.take_branch(rel);
    }

    return 0;
}


/********************************************************************************
 * BEQ (Branch relative if zero)
 ********************************************************************************/
int Mos6502::i_BEQ(Mos6502& self, addr_t rel)
{
    /*
     * BEQ $r8
     *
     * 2 cycles, (3 if branched to same page, 4 if branched to another page)
     */
    if (self.test_Z()) {
        self.take_branch(rel);
    }

    return 0;
}


/********************************************************************************
 * BRK (Software interrupt)
 ********************************************************************************/
int Mos6502::i_BRK(Mos6502& self, addr_t)
{
    /*
     * BRK
     *
     *   push PC + 2
     *   push (P | Flag::B)
     *   P |= flag::I
     *   PC = *($FFFE)
     *
     * 7 cycles
     */
    self.push_addr(self._regs.PC + 1);

    self.flag(Flags::B);
    self.push_P();
    self.flag(Flags::I);

    addr_t addr = self.read_addr(vIRQ);
    self._regs.PC = addr;

    return 0;
}


/********************************************************************************
 * RTI (Return from Interrupt)
 ********************************************************************************/
int Mos6502::i_RTI(Mos6502& self, addr_t)
{
    /*
     * RTI
     *
     *  pop P
     *  pop PC
     *
     * 6 cycles
     */
    self.pop_P();
    self._regs.PC = self.pop_addr();
    return 0;
}


/********************************************************************************
 * JSR (Jump to Subroutine)
 ********************************************************************************/
int Mos6502::i_JSR(Mos6502& self, addr_t addr)
{
    /*
     * JSR $0000
     *
     * 6 cycles
     */
    addr = self.read_addr(self._regs.PC - 2);
    self.push_addr(self._regs.PC - 1); /* The pushed value is the last byte of the JSR instruction */
    self._regs.PC = addr;
    return 0;
}


/********************************************************************************
 * RTS (Return from Subroutine)
 ********************************************************************************/
int Mos6502::i_RTS(Mos6502& self, addr_t)
{
    /*
     * RTS
     *
     * 6 cycles
     */
    addr_t ra = self.pop_addr() + 1;   /* The popped value is the last byte of the JSR instruction */
    self._regs.PC = ra;
    return 0;
}


/********************************************************************************
 * JMP (Jump to absolute address)
 ********************************************************************************/
int Mos6502::i_JMP(Mos6502& self, addr_t addr)
{
    /*
     * JMP $0000
     * 3 cycles
     *
     * JMP ($0000)
     * 5 cycles
     */
    self._regs.PC = addr;
    return 0;
}


/********************************************************************************
 * NOP (No Operation: Jump to next instruction)
 ********************************************************************************/
int Mos6502::i_NOP(Mos6502& self, addr_t)
{
    /*
     * NOP
     *
     * 2 cycles
     */
    return 0;
}

}
