/*
 * Copyright (C) 2020-2022 Claudio Castiglia
 *
 * This file is part of CAIO.
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
 * LDA (load accumulator)
 ********************************************************************************/
void Mos6502::i_LDA_imm(Mos6502 &self, addr_t value)
{
    /*
     * LDA #$00
     *
     * Flags: N Z
     */
    self._regs.A = static_cast<uint8_t>(value);
    self.set_N(self._regs.A);
    self.set_Z(self._regs.A);
}

void Mos6502::i_LDA(Mos6502 &self, addr_t addr)
{
    /*
     * LDA $00
     * LDA $00, X
     * LDA $0000
     * LDA $0000, X
     * LDA $0000, Y
     * LDA ($00, X)
     * LDA ($00), Y
     *
     * Flags: N Z
     */
    self._regs.A = self.read(addr);
    self.set_N(self._regs.A);
    self.set_Z(self._regs.A);
}


/********************************************************************************
 * LDX (load register X)
 ********************************************************************************/
void Mos6502::i_LDX_imm(Mos6502 &self, addr_t value)
{
    /*
     * LDX #$00
     *
     * Flags: N Z
     */
    self._regs.X = static_cast<uint8_t>(value);
    self.set_N(self._regs.X);
    self.set_Z(self._regs.X);
}

void Mos6502::i_LDX(Mos6502 &self, addr_t addr)
{
    /*
     * LDX $00
     * LDX $00, Y
     * LDX $0000
     * LDX $0000, Y
     *
     * Flags: N Z
     */
    self._regs.X = self.read(addr);
    self.set_N(self._regs.X);
    self.set_Z(self._regs.X);
}


/********************************************************************************
 * LDY (load register Y)
 ********************************************************************************/
void Mos6502::i_LDY_imm(Mos6502 &self, addr_t value)
{
    /*
     * LDY #$00
     *
     * Flags: N Z
     */
    self._regs.Y = static_cast<uint8_t>(value);
    self.set_N(self._regs.Y);
    self.set_Z(self._regs.Y);
}

void Mos6502::i_LDY(Mos6502 &self, addr_t addr)
{
    /*
     * LDY $00
     * LDY $00, X
     * LDY $0000
     * LDY $0000, X
     *
     * Flags: N Z
     */
    self._regs.Y = self.read(addr);
    self.set_N(self._regs.Y);
    self.set_Z(self._regs.Y);
}


/********************************************************************************
 * STA (store accumulator)
 ********************************************************************************/
void Mos6502::i_STA(Mos6502 &self, addr_t addr)
{
    /*
     * STA $00
     * STA $00, X
     * STA $0000
     * STA $0000, X
     * STA $0000, Y
     * STA ($00, X)
     * STA ($00), Y
     */
    self.write(addr, self._regs.A);
}


/********************************************************************************
 * STX (store register X)
 ********************************************************************************/
void Mos6502::i_STX(Mos6502 &self, addr_t addr)
{
    /*
     * STX $00
     * STX $00, Y
     * STX $0000
     */
    self.write(addr, self._regs.X);
}


/********************************************************************************
 * STY (store register Y)
 ********************************************************************************/
void Mos6502::i_STY(Mos6502 &self, addr_t addr)
{
    /*
     * STY $00
     * STY $00, X
     * STY $0000
     */
    self.write(addr, self._regs.Y);
}


/********************************************************************************
 * TAX (X = A)
 ********************************************************************************/
void Mos6502::i_TAX(Mos6502 &self, addr_t _)
{
    /*
     * TAX
     *
     * Flags: N Z
     */
    self._regs.X = self._regs.A;
    self.set_Z(self._regs.X);
    self.set_N(self._regs.X);
}


/********************************************************************************
 * TXA (A = X)
 ********************************************************************************/
void Mos6502::i_TXA(Mos6502 &self, addr_t _)
{
    /*
     * TXA
     *
     * Flags: N Z
     */
    self._regs.A = self._regs.X;
    self.set_Z(self._regs.A);
    self.set_N(self._regs.A);
}


/********************************************************************************
 * TAY (Y = A)
 ********************************************************************************/
void Mos6502::i_TAY(Mos6502 &self, addr_t _)
{
    /*
     * TAY
     *
     * Flags: N Z
     */
    self._regs.Y = self._regs.A;
    self.set_N(self._regs.Y);
    self.set_Z(self._regs.Y);
}


/********************************************************************************
 * TYA (A = Y)
 ********************************************************************************/
void Mos6502::i_TYA(Mos6502 &self, addr_t _)
{
    /*
     * TYA
     *
     * Flags: N Z
     */
    self._regs.A = self._regs.Y;
    self.set_N(self._regs.A);
    self.set_Z(self._regs.A);
}


/********************************************************************************
 * TSX (X = S)
 ********************************************************************************/
void Mos6502::i_TSX(Mos6502 &self, addr_t _)
{
    /*
     * TSX
     *
     * Flags: N Z
     */
    self._regs.X = self._regs.S;
    self.set_Z(self._regs.X);
    self.set_N(self._regs.X);
}


/********************************************************************************
 * TXS (S = X)
 ********************************************************************************/
void Mos6502::i_TXS(Mos6502 &self, addr_t _)
{
    /*
     * TXS
     *
     * Flags: -
     */
    self._regs.S = self._regs.X;
}


/********************************************************************************
 * PLA (Pop accumulator: A = pop())
 ********************************************************************************/
void Mos6502::i_PLA(Mos6502 &self, addr_t)
{
    /*
     * PLA
     *
     * Flags: N Z
     */
    self._regs.A = self.pop();
    self.set_N(self._regs.A);
    self.set_Z(self._regs.A);
}


/********************************************************************************
 * PHA (Push accumulator: push(A))
 ********************************************************************************/
void Mos6502::i_PHA(Mos6502 &self, addr_t)
{
    /*
     * PHA
     */
    self.push(self._regs.A);
}


/********************************************************************************
 * PLP (Pop processor status: P = pop())
 ********************************************************************************/
void Mos6502::i_PLP(Mos6502 &self, addr_t _)
{
    /*
     * PLP
     */
    self.pop_P();
}


/********************************************************************************
 * PHP (Push processor status: push(P))
 ********************************************************************************/
void Mos6502::i_PHP(Mos6502 &self, addr_t _)
{
    /*
     * PHP
     */
    self.push_P();
}

}
