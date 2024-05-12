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
 * LDA (load accumulator)
 ********************************************************************************/
int Mos6502::i_LDA_imm(Mos6502& self, addr_t value)
{
    /*
     * LDA #$00
     *
     * Flags: N Z
     */
    self._regs.A = static_cast<uint8_t>(value);
    self.set_N(self._regs.A);
    self.set_Z(self._regs.A);
    return 0;
}

int Mos6502::i_LDA(Mos6502& self, addr_t addr)
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
    return 0;
}

/********************************************************************************
 * LDX (load register X)
 ********************************************************************************/
int Mos6502::i_LDX_imm(Mos6502& self, addr_t value)
{
    /*
     * LDX #$00
     *
     * Flags: N Z
     */
    self._regs.X = static_cast<uint8_t>(value);
    self.set_N(self._regs.X);
    self.set_Z(self._regs.X);
    return 0;
}

int Mos6502::i_LDX(Mos6502& self, addr_t addr)
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
    return 0;
}

/********************************************************************************
 * LDY (load register Y)
 ********************************************************************************/
int Mos6502::i_LDY_imm(Mos6502& self, addr_t value)
{
    /*
     * LDY #$00
     *
     * Flags: N Z
     */
    self._regs.Y = static_cast<uint8_t>(value);
    self.set_N(self._regs.Y);
    self.set_Z(self._regs.Y);
    return 0;
}

int Mos6502::i_LDY(Mos6502& self, addr_t addr)
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
    return 0;
}

/********************************************************************************
 * STA (store accumulator)
 ********************************************************************************/
int Mos6502::i_STA(Mos6502& self, addr_t addr)
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
    return 0;
}

/********************************************************************************
 * STX (store register X)
 ********************************************************************************/
int Mos6502::i_STX(Mos6502& self, addr_t addr)
{
    /*
     * STX $00
     * STX $00, Y
     * STX $0000
     */
    self.write(addr, self._regs.X);
    return 0;
}

/********************************************************************************
 * STY (store register Y)
 ********************************************************************************/
int Mos6502::i_STY(Mos6502& self, addr_t addr)
{
    /*
     * STY $00
     * STY $00, X
     * STY $0000
     */
    self.write(addr, self._regs.Y);
    return 0;
}

/********************************************************************************
 * TAX (X = A)
 ********************************************************************************/
int Mos6502::i_TAX(Mos6502& self, addr_t)
{
    /*
     * TAX
     *
     * Flags: N Z
     */
    self._regs.X = self._regs.A;
    self.set_Z(self._regs.X);
    self.set_N(self._regs.X);
    return 0;
}

/********************************************************************************
 * TXA (A = X)
 ********************************************************************************/
int Mos6502::i_TXA(Mos6502& self, addr_t)
{
    /*
     * TXA
     *
     * Flags: N Z
     */
    self._regs.A = self._regs.X;
    self.set_Z(self._regs.A);
    self.set_N(self._regs.A);
    return 0;
}

/********************************************************************************
 * TAY (Y = A)
 ********************************************************************************/
int Mos6502::i_TAY(Mos6502& self, addr_t)
{
    /*
     * TAY
     *
     * Flags: N Z
     */
    self._regs.Y = self._regs.A;
    self.set_N(self._regs.Y);
    self.set_Z(self._regs.Y);
    return 0;
}

/********************************************************************************
 * TYA (A = Y)
 ********************************************************************************/
int Mos6502::i_TYA(Mos6502& self, addr_t)
{
    /*
     * TYA
     *
     * Flags: N Z
     */
    self._regs.A = self._regs.Y;
    self.set_N(self._regs.A);
    self.set_Z(self._regs.A);
    return 0;
}

/********************************************************************************
 * TSX (X = S)
 ********************************************************************************/
int Mos6502::i_TSX(Mos6502& self, addr_t)
{
    /*
     * TSX
     *
     * Flags: N Z
     */
    self._regs.X = self._regs.S;
    self.set_Z(self._regs.X);
    self.set_N(self._regs.X);
    return 0;
}

/********************************************************************************
 * TXS (S = X)
 ********************************************************************************/
int Mos6502::i_TXS(Mos6502& self, addr_t)
{
    /*
     * TXS
     *
     * Flags: -
     */
    self._regs.S = self._regs.X;
    return 0;
}

/********************************************************************************
 * PLA (Pop accumulator: A = pop())
 ********************************************************************************/
int Mos6502::i_PLA(Mos6502& self, addr_t)
{
    /*
     * PLA
     *
     * Flags: N Z
     */
    self._regs.A = self.pop();
    self.set_N(self._regs.A);
    self.set_Z(self._regs.A);
    return 0;
}

/********************************************************************************
 * PHA (Push accumulator: push(A))
 ********************************************************************************/
int Mos6502::i_PHA(Mos6502& self, addr_t)
{
    /*
     * PHA
     */
    self.push(self._regs.A);
    return 0;
}

/********************************************************************************
 * PLP (Pop processor status: P = pop())
 ********************************************************************************/
int Mos6502::i_PLP(Mos6502& self, addr_t)
{
    /*
     * PLP
     */
    self.pop_P();
    return 0;
}

/********************************************************************************
 * PHP (Push processor status: push(P))
 ********************************************************************************/
int Mos6502::i_PHP(Mos6502& self, addr_t)
{
    /*
     * PHP
     */
    self.push_P();
    return 0;
}

}
