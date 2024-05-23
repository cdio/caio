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

int Mos6502::i_LDA_imm(Mos6502& self, addr_t value)
{
    /*
     * Load accumulator
     * LDA #$00         - A9
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
     * Load accumulator from memory
     * LDA ($00, X)     - A1
     * LDA $00          - A5
     * LDA ($00), Y     - B1
     * LDA $00, X       - B5
     * LDA $0000, Y     - B9
     * LDA $0000        - AD
     * LDA $0000, X     - BD
     * Flags: N Z
     */
    self._regs.A = self.read(addr);
    self.set_N(self._regs.A);
    self.set_Z(self._regs.A);
    return 0;
}

int Mos6502::i_LDX_imm(Mos6502& self, addr_t value)
{
    /*
     * Load X
     * LDX #$00         - A2
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
     * Load X from memory
     * LDX $00          - A6
     * LDX $00, Y       - B6
     * LDX $0000        - AE
     * LDX $0000, Y     - BE
     * Flags: N Z
     */
    self._regs.X = self.read(addr);
    self.set_N(self._regs.X);
    self.set_Z(self._regs.X);
    return 0;
}

int Mos6502::i_LDY_imm(Mos6502& self, addr_t value)
{
    /*
     * Load Y
     * LDY #$00         - A0 - Load Y
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
     * Load Y from memory
     * LDY $00          - A4
     * LDY $00, X       - B4
     * LDY $0000        - AC
     * LDY $0000, X     - BC
     * Flags: N Z
     */
    self._regs.Y = self.read(addr);
    self.set_N(self._regs.Y);
    self.set_Z(self._regs.Y);
    return 0;
}

int Mos6502::i_STA(Mos6502& self, addr_t addr)
{
    /*
     * Store Accumulator
     * STA $00          - 85
     * STA $00, X       - 95
     * STA $0000        - 8D
     * STA $0000, X     - 9D
     * STA $0000, Y     - 99
     * STA ($00, X)     - 81
     * STA ($00), Y     - 91
     */
    self.write(addr, self._regs.A);
    return 0;
}

int Mos6502::i_STX(Mos6502& self, addr_t addr)
{
    /*
     * Store register X
     * STX $00          - 86
     * STX $00, Y       - 96
     * STX $0000        - 8E
     */
    self.write(addr, self._regs.X);
    return 0;
}

int Mos6502::i_STY(Mos6502& self, addr_t addr)
{
    /*
     * Store register Y
     * STY $00          - 84
     * STY $00, X       - 94
     * STY $0000        - 8C
     */
    self.write(addr, self._regs.Y);
    return 0;
}

int Mos6502::i_TAX(Mos6502& self, addr_t)
{
    /*
     * Copy A into X
     * TAX              - AA
     * Flags: N Z
     */
    self._regs.X = self._regs.A;
    self.set_Z(self._regs.X);
    self.set_N(self._regs.X);
    return 0;
}

int Mos6502::i_TXA(Mos6502& self, addr_t)
{
    /*
     * Copy X into A
     * TXA              - 8A
     * Flags: N Z
     */
    self._regs.A = self._regs.X;
    self.set_Z(self._regs.A);
    self.set_N(self._regs.A);
    return 0;
}

int Mos6502::i_TAY(Mos6502& self, addr_t)
{
    /*
     * Copy A into Y
     * TAY              - A8
     * Flags: N Z
     */
    self._regs.Y = self._regs.A;
    self.set_N(self._regs.Y);
    self.set_Z(self._regs.Y);
    return 0;
}

int Mos6502::i_TYA(Mos6502& self, addr_t)
{
    /*
     * Copy Y into A
     * TYA              - 98
     * Flags: N Z
     */
    self._regs.A = self._regs.Y;
    self.set_N(self._regs.A);
    self.set_Z(self._regs.A);
    return 0;
}

int Mos6502::i_TSX(Mos6502& self, addr_t)
{
    /*
     * Copy S into X
     * TSX              - BA
     * Flags: N Z
     */
    self._regs.X = self._regs.S;
    self.set_Z(self._regs.X);
    self.set_N(self._regs.X);
    return 0;
}

int Mos6502::i_TXS(Mos6502& self, addr_t)
{
    /*
     * Copy X into S
     * TXS              - 9A
     * Flags: -
     */
    self._regs.S = self._regs.X;
    return 0;
}

int Mos6502::i_PLA(Mos6502& self, addr_t)
{
    /*
     * Pop accumulator
     * PLA              - 68
     * Flags: N Z
     */
    self._regs.A = self.pop();
    self.set_N(self._regs.A);
    self.set_Z(self._regs.A);
    return 0;
}

int Mos6502::i_PHA(Mos6502& self, addr_t)
{
    /*
     * Push accumulator
     * PHA              - 48
     */
    self.push(self._regs.A);
    return 0;
}

int Mos6502::i_PLP(Mos6502& self, addr_t)
{
    /*
     * Pop status flag
     * PLP              - 28
     * P = pop() & ~Flags::B
     */
    self._regs.P = (self.pop() & ~Flags::B) | Flags::_;
    return 0;
}

int Mos6502::i_PHP(Mos6502& self, addr_t)
{
    /*
     * Push status flag
     * PHP              - 08
     * push(P | Flags::B)
     */
    self.push(self._regs.P | Flags::B);
    return 0;
}

}
