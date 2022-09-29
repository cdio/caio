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
#include "mos_6502.hpp"


namespace caio {

/********************************************************************************
 * BPL (Branch relative if positive)
 ********************************************************************************/
void Mos6502::i_BPL(Mos6502 &self, addr_t rel)
{
    /*
     * BPL $r8
     *
     * 2 cycles, (3 if branched to same page, 4 if branched to another page)
     */
    if (!self.test_N()) {
        self.take_branch(rel);
    }
}


/********************************************************************************
 * BMI (Branch relative if negative)
 ********************************************************************************/
void Mos6502::i_BMI(Mos6502 &self, addr_t rel)
{
    /*
     * BMI $r8
     *
     * 2 cycles, (3 if branched to same page, 4 if branched to another page)
     */
    if (self.test_N()) {
        self.take_branch(rel);
    }
}


/********************************************************************************
 * BVC (Branch relative if not overflow)
 ********************************************************************************/
void Mos6502::i_BVC(Mos6502 &self, addr_t rel)
{
    /*
     * BVC $r8
     *
     * 2 cycles, (3 if branched to same page, 4 if branched to another page)
     */
    if (!self.test_V()) {
        self.take_branch(rel);
    }
}


/********************************************************************************
 * BVS (Branch relative if overflow)
 ********************************************************************************/
void Mos6502::i_BVS(Mos6502 &self, addr_t rel)
{
    /*
     * BVS $r8
     *
     * 2 cycles, (3 if branched to same page, 4 if branched to another page)
     */
    if (self.test_V()) {
        self.take_branch(rel);
    }
}


/********************************************************************************
 * BCC (Branch relative if carry is not set)
 ********************************************************************************/
void Mos6502::i_BCC(Mos6502 &self, addr_t rel)
{
    /*
     * BCC $r8
     *
     * 2 cycles, (3 if branched to same page, 4 if branched to another page)
     */
    if (!self.test_C()) {
        self.take_branch(rel);
    }
}


/********************************************************************************
 * BCS (Branch relative if carry is set)
 ********************************************************************************/
void Mos6502::i_BCS(Mos6502 &self, addr_t rel)
{
    /*
     * BCS $r8
     *
     * 2 cycles, (3 if branched to same page, 4 if branched to another page)
     */
    if (self.test_C()) {
        self.take_branch(rel);
    }
}


/********************************************************************************
 * BNE (Branch relative if not zero)
 ********************************************************************************/
void Mos6502::i_BNE(Mos6502 &self, addr_t rel)
{
    /*
     * BNE $r8
     *
     * 2 cycles, (3 if branched to same page, 4 if branched to another page)
     */
    if (!self.test_Z()) {
        self.take_branch(rel);
    }
}


/********************************************************************************
 * BEQ (Branch relative if zero)
 ********************************************************************************/
void Mos6502::i_BEQ(Mos6502 &self, addr_t rel)
{
    /*
     * BEQ $r8
     *
     * 2 cycles, (3 if branched to same page, 4 if branched to another page)
     */
    if (self.test_Z()) {
        self.take_branch(rel);
    }
}


/********************************************************************************
 * BRK (Software interrupt)
 ********************************************************************************/
void Mos6502::i_BRK(Mos6502 &self, addr_t _)
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
}


/********************************************************************************
 * RTI (Return from Interrupt)
 ********************************************************************************/
void Mos6502::i_RTI(Mos6502 &self, addr_t _)
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
}


/********************************************************************************
 * JSR (Jump to Subroutine)
 ********************************************************************************/
void Mos6502::i_JSR(Mos6502 &self, addr_t addr)
{
    /*
     * JSR $0000
     *
     * 6 cycles
     */
    addr = self.read_addr(self._regs.PC - 2);
    self.push_addr(self._regs.PC - 1); /* The pushed value is the last byte of the JSR instruction */
    self._regs.PC = addr;
}


/********************************************************************************
 * RTS (Return from Subroutine)
 ********************************************************************************/
void Mos6502::i_RTS(Mos6502 &self, addr_t _)
{
    /*
     * RTS
     *
     * 6 cycles
     */
    addr_t ra = self.pop_addr() + 1;   /* The popped value is the last byte of the JSR instruction */
    self._regs.PC = ra;
}


/********************************************************************************
 * JMP (Jump to absolute address)
 ********************************************************************************/
void Mos6502::i_JMP(Mos6502 &self, addr_t addr)
{
    /*
     * JMP $0000
     * 3 cycles
     *
     * JMP ($0000)
     * 5 cycles
     */
    self._regs.PC = addr;
}


/********************************************************************************
 * NOP (No Operation: Jump to next instruction)
 ********************************************************************************/
void Mos6502::i_NOP(Mos6502 &self, addr_t _)
{
    /*
     * NOP
     *
     * 2 cycles
     */
}

}
