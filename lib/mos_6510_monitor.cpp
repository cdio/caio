/*
 * Copyright (C) 2020 Claudio Castiglia
 *
 * This file is part of CEMU.
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
#include "mos_6510_monitor.hpp"

#include <endian.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "mos_6510.hpp"
#include "prg.hpp"
#include "utils.hpp"


namespace cemu {

const std::vector<Mos6510Monitor::Command> Mos6510Monitor::commands{
    { "assemble", "a",  ".|$addr",            "Assemble machine code from $addr",       Mos6510Monitor::assemble    },
    { "disass",   "d",  ".|$addr [$n]",       "Disassemble $n instructions from $addr", Mos6510Monitor::disassemble },
    { "dump",     "x",  ". [$n]",             "Dump $n bytes of memory from $addr",     Mos6510Monitor::dump        },
    { "regs",     "r",  "",                   "Show registers",                         Mos6510Monitor::registers   },
    { "bpadd",    "b",  "$addr",              "Add a breakpoint at $addr",              Mos6510Monitor::bp_add      },
    { "bpadd",    "b",  "$addr <cond>",       "Add a conditional breakpoint at $addr",  Mos6510Monitor::bp_add      },
    { "bpadd",    "b",  "help|h|?",           "Help about breakpoints",                 Mos6510Monitor::bp_add      },
    { "bpdel",    "bd", "$addr",              "Delete breakpoint at $addr",             Mos6510Monitor::bp_del      },
    { "bpclear",  "bc", "",                   "Clear all breakpoints",                  Mos6510Monitor::bp_clear    },
    { "bplist",   "bl", "",                   "List breakpoints",                       Mos6510Monitor::bp_list     },
    { "go",       "g",  ".|$addr",            "Run program at $addr",                   Mos6510Monitor::go          },
    { "si",       "s",  "[.|$addr]",          "Execute single instruction",             Mos6510Monitor::step        },
    { "load",     "l",  "<prog> [$addr]",     "Load a binary or PRG file",              Mos6510Monitor::load        },
    { "save",     "s",  "<prog> $start $end", "Save a PRG file",                        Mos6510Monitor::save        },
    { "loglevel", "lv", "<lv>",               "Set the CPU loglevel",                   Mos6510Monitor::loglevel    },
    { "quit",     "q",  "",                   "Halt the CPU",                           Mos6510Monitor::quit        },
    { "quit",     "q",  "<e>",                "Terminate the emulator with exit code",  Mos6510Monitor::quit        },
    { "help",     "h",  "",                   "This help",                              Mos6510Monitor::help        },
    { "help",     "?",  "",                   "",                                       Mos6510Monitor::help        }
};


const std::vector<std::pair<std::string, Mos6510Monitor::Expr::ex_t>> Mos6510Monitor::Expr::operators{
    { "<=", [](const Mos6510 &cpu, const Mos6510Monitor::Expr::fn_t &a, const Mos6510Monitor::Expr::fn_t &b) {
        return a(cpu) <= b(cpu);
    }},
    { ">=", [](const Mos6510 &cpu, const Mos6510Monitor::Expr::fn_t &a, const Mos6510Monitor::Expr::fn_t &b) {
        return a(cpu) >= b(cpu);
    }},
    { "==", [](const Mos6510 &cpu, const Mos6510Monitor::Expr::fn_t &a, const Mos6510Monitor::Expr::fn_t &b) {
        return a(cpu) == b(cpu);
    }},
    { "!=", [](const Mos6510 &cpu, const Mos6510Monitor::Expr::fn_t &a, const Mos6510Monitor::Expr::fn_t &b) {
        return a(cpu) != b(cpu);
    }},
    { "<", [](const Mos6510 &cpu, const Mos6510Monitor::Expr::fn_t &a, const Mos6510Monitor::Expr::fn_t &b) {
        return a(cpu) < b(cpu);
    }},
    { ">", [](const Mos6510 &cpu, const Mos6510Monitor::Expr::fn_t &a, const Mos6510Monitor::Expr::fn_t &b) {
        return a(cpu) > b(cpu);
    }},
    { "&", [](const Mos6510 &cpu, const Mos6510Monitor::Expr::fn_t &a, const Mos6510Monitor::Expr::fn_t &b) {
        return a(cpu) & b(cpu);
    }},
    { "|", [](const Mos6510 &cpu, const Mos6510Monitor::Expr::fn_t &a, const Mos6510Monitor::Expr::fn_t &b) {
        return a(cpu) | b(cpu);
    }}
};


const std::vector<std::pair<std::string, Mos6510Monitor::Expr::fn_t>> Mos6510Monitor::Expr::registers{
    { "ra",      [](const Mos6510 &cpu) { return cpu._regs.A;    }   },
    { "rx",      [](const Mos6510 &cpu) { return cpu._regs.X;    }   },
    { "ry",      [](const Mos6510 &cpu) { return cpu._regs.Y;    }   },
    { "rs",      [](const Mos6510 &cpu) { return cpu._regs.S;    }   },
    { "rp",      [](const Mos6510 &cpu) { return cpu._regs.P;    }   },
    { "rp.n",    [](const Mos6510 &cpu) { return cpu.test_N();   }   },
    { "rp.v",    [](const Mos6510 &cpu) { return cpu.test_V();   }   },
    { "rp.b",    [](const Mos6510 &cpu) { return cpu.test_B();   }   },
    { "rp.d",    [](const Mos6510 &cpu) { return cpu.test_D();   }   },
    { "rp.i",    [](const Mos6510 &cpu) { return cpu.test_I();   }   },
    { "rp.z",    [](const Mos6510 &cpu) { return cpu.test_Z();   }   },
    { "rp.c",    [](const Mos6510 &cpu) { return cpu.test_C();   }   }
};


Mos6510Monitor::Expr::fn_t Mos6510Monitor::Expr::compile_argument(const std::string &line)
{
    /*
     * <line> = "ra" | "rx" | "ry" | "rs" | "rp" | "rp.[nvbdizc]" | ["*"]["#"]["$"]<number>
     */
    auto pos = line.find_first_not_of(" \n\r\v\t");
    if (pos != std::string::npos) {
        auto last = line.find_last_not_of(" \n\r\v\t");
        if (last == std::string::npos) {
            last = line.size();
        }

        /*
         * Detect if it is a direct value or a reference to a value stored in memory.
         */
        bool isref{};
        if (line[pos] == '*') {
            isref = true;
            ++pos;
        }

        /*
         * Detect numeric base (default is hexadecimal).
         */
        int base = 16;
        if (line[pos] == '#') {
            /*
             * #xx indicates decimal.
             */
            base = 10;
            ++pos;
        }

        if (line[pos] == '$') {
            /*
             * $#xx or $xx indicates hexadecimal.
             */
            base = 16;
            ++pos;
        }

        std::string svalue = utils::tolow(line.substr(pos, last - pos + 1));

        /*
         * Try to compile a literal value.
         */
        char *e{};
        uint64_t lit = std::strtoull(svalue.c_str(), &e, base);
        if (*e == '\0') {
            /*
             * It is a literal value.
             */
            return [isref, lit](const Mos6510 &cpu) -> int {
                uint16_t val = static_cast<uint16_t>(lit);
                return (isref ? cpu._mmap->read(val) : val);
            };
        }

        /*
         * Try to compile a register value.
         */
        for (const auto &elem : registers) {
            if (elem.first == svalue) {
                /*
                 * It is a register value.
                 */
                const auto &fn = elem.second;
                return [isref, fn](const Mos6510 &cpu) -> int {
                    uint16_t val = fn(cpu);
                    return (isref ? cpu._mmap->read(val) : val);
                };
            }
        }
    }

    /*
     * The line does not contain a literal value nor a register name.
     */
    std::stringstream os{};
    os << "Invalid expression: " << std::quoted(line);
    throw InvalidArgument(os.str());
}

std::pair<Mos6510Monitor::Expr::fn_t, std::exception> Mos6510Monitor::Expr::compile(const std::string &line)
{
    /*
     * <expr> = <val1> <op> <val2>
     */
    try {
        for (const auto &op : operators) {
            const auto &name = op.first;
            auto pos = line.find(name);
            if (pos != std::string::npos) {
                const auto arg1 = line.substr(0, pos);
                const auto arg2 = line.substr(pos + name.size());
                const auto carg1 = compile_argument(arg1);
                const auto carg2 = compile_argument(arg2);
                const auto oper{op.second};
                return {std::bind(std::move(oper), std::placeholders::_1, std::move(carg1), std::move(carg2)), {}};
            }
        }

        std::ostringstream os{};
        os << "Invalid exmpression: " << std::quoted(line);
        return {{}, InvalidArgument{os.str()}};

    } catch (const std::exception &err) {
        return {{}, std::move(err)};
    }
}

size_t Mos6510Monitor::run()
{
    _is_running = true;

    while (_is_running) {
        _os << prompt();

        std::string line{};
        if (!std::getline(_is, line, '\n')) {
            /* Most probably EOF */
            break;
        }

        if (line.empty()) {
//FIXME            line = _prev_line;
//breaks monitor test scripts
        }

        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::vector<std::string> args{};
        std::string arg{};
        std::istringstream iss{line};
        while (iss >> arg) {
            args.push_back(arg);
        }

        auto it = std::find_if(commands.begin(), commands.end(), [&args](const Command &cmd) {
            return (cmd.command == args[0] || cmd.short_command == args[0]);
        });

        if (it == commands.end()) {
            _os << "Invalid command: " << args[0] << std::endl;
            continue;
        }

        _prev_line = line;
        _prev_fn = it->fn;

        if (it->fn(*this, args)) {
            /*
             * The command wants to exit the monitor letting
             * the system to continue with the CPU emulation.
             */
            return (!Clockable::HALT);
        }
    }

    /* The user wants to terminate the session or some other error occurred */
    return Clockable::HALT;
}


bool Mos6510Monitor::is_breakpoint(addr_t addr) const
{
    const auto it = _breakpoints.find(addr);
    if (it != _breakpoints.end()) {
        const cond_t &cond = it->second;
        auto &fn = cond.first;
        if (!fn) {
            /*
             * Unconditional breakpoint.
             */
            _os << "Breakpoint at $" << utils::to_string(addr) << std::endl;
            return true;
        }

        if (fn(_cpu) != 0) {
            /*
             * Conditional breakpoint.
             */
            _os << "Conditional breakpoint at $" << utils::to_string(addr) << " " << cond.second << std::endl;
            return true;
        }
    }

    /*
     * No breakpont.
     */
    return false;
}

std::string Mos6510Monitor::prompt()
{
    std::ostringstream os{};

    if (_prev_fn == Mos6510Monitor::step) {
        _cpu.disass(os, _cpu._regs.PC, 10, true);
        os << _cpu._regs.to_string() << std::endl;
    }

    os << PROMPT_PREFIX << "$" << utils::to_string(_cpu._regs.PC) << PROMPT_SUFFIX;

    return os.str();
}

addr_t Mos6510Monitor::to_addr(const std::string &str, addr_t defval)
{
    return ((str == ".") ? defval : static_cast<addr_t>(to_count(str)));
}

size_t Mos6510Monitor::to_count(const std::string &str)
{
    try {
        return utils::to_number<size_t>(str);
    } catch (const InvalidNumber &err) {
        _os << "Invalid value: " << str << std::endl;
        throw err;
    }
}

bool Mos6510Monitor::assemble(Mos6510Monitor &self, const std::vector<std::string> &args)
{
    /*
     * assemble [<addr>|.]
     * a [<addr>|.]
     */
    addr_t addr = self._cpu._regs.PC;
    for (auto it = args.begin() + 1; it != args.end(); ++it) {
        try {
            addr = self.to_addr(*it, addr);
        } catch (InvalidNumber &) {
            /* Error shown. Continue with next one */
            ;
        }
    }

    self._os << "Entering edit mode. To finish write '.' or an empty line" << std::endl;

    auto mmap = self._cpu._mmap;

    while (true) {
        self._os << "$" << utils::to_string(addr) << ": " << std::flush;

        std::string line{};
        if (!std::getline(self._is, line, '\n')) {
            /* Most probably EOF */
            break;
        }

        if (line.empty() || line == ".") {
            /*
             * Leave the edit mode.
             */
            break;
        }

        /*
         * Parse line.
         */
        std::vector<uint8_t> program{};
        std::istringstream iss{line};
        std::string str{};
        while (iss >> str) {
            try {
                auto u8 = utils::to_number<uint8_t>(str);
                program.push_back(u8);
            } catch (const InvalidNumber &_) {
                /*
                 * Show the error and invalidate the whole line.
                 */
                self._os << "Invalid value: " << str << std::endl;
                program.clear();
                break;
            }
        }

        /*
         * Copy the user edited program line into memory.
         */
        try {
            for (uint8_t u8 : program) {
                mmap->write(addr++, u8);
            }
        } catch (const std::exception &err) {
            /*
             * Error, exit from edit mode.
             */
            self._os << "Unexpected: " << err.what() << std::endl;
            break;
        }
    }

    return false;
}

bool Mos6510Monitor::disassemble(Mos6510Monitor &self, const std::vector<std::string> &args)
{
    /*
     * disass [<addr> [<count>]]
     * d [<addr> [<count>]]
     */
    addr_t addr = self._cpu._regs.PC;
    size_t count = 16;

    try {
        if (args.size() > 2) {
            count = self.to_count(args[2]);
        }
        if (args.size() > 1) {
            addr = self.to_addr(args[1], addr);
        }
    } catch (const InvalidNumber &) {
        return false;
    }

    self._cpu.disass(self._os, addr, count, true);
    return false;
}

bool Mos6510Monitor::dump(Mos6510Monitor &self, const std::vector<std::string> &args)
{
    /*
     * dump [<addr> [<count>]]
     * x [<addr> [<count>]]
     */
    addr_t addr = self._cpu._regs.PC;
    size_t count = 16;

    try {
        if (args.size() > 2) {
            count = self.to_count(args[2]);
        }

        if (args.size() > 1) {
            addr = self.to_addr(args[1], addr);
        }

    } catch (const InvalidNumber &) {
        return false;
    }

    if (count == 0) {
        count = 0xFFFF - addr + 1;
    }

    addr_t ra = addr;

    std::vector<uint8_t> data(count);

    std::generate(data.begin(), data.end(), [&self, &ra]() -> uint8_t {
        return self._cpu._mmap->read(ra++);
    });

    utils::dump(self._os, data, addr) << std::endl;

    return false;
}

bool Mos6510Monitor::registers(Mos6510Monitor &self, const std::vector<std::string> &args)
{
    /*
     * registers, r
     */
    self._os << self._cpu._regs.to_string() << std::endl;
    return false;
}

bool Mos6510Monitor::bp_add(Mos6510Monitor &self, const std::vector<std::string> &args)
{
    /*
     * Add a breakpoint at a specified address:
     *  b  help | h | ?
     *  b  <addr> [<cond>]
     *
     * Ex:
     *      *$fd02 == 3
     *      P.N == 0
     *      A < #10
     *
     * <cond> = <val> <op> <val>
     * <val>  = [$]<number> | *<number> | ra | rx | ry | rs | rp | rp.[nvbdizc]
     * <op>   = "<=" | ">=" | "<" | ">" | "==" | "!=" | "&"  | "|"
     */
    if (args.size() == 1) {
        /*
         * Nothing to do.
         */
        return false;
    }

    /*
     * Help.
     */
    if (args[1] == "h" || args[1] == "?" || args[1] == "help") {
        self._os << args[0] << " help | h | ?" << std::endl
            << args[0] << " <addr> [<cond>]" << std::endl
            << std::endl
            << "<cond> = <val> <op> <val>" << std::endl
            << "<val>  = [*]{[#][$]<u16>| ra | rx | ry | rs | rp | rp.n | rp.v | rp.b | rp.i | rp.z | rp.c}" << std::endl
            << "<op>   = '<' | '>' | '<=' | '>=' | '==' | '!=' | '&' | '|'" << std::endl
            << std::endl
            << "examples:" << std::endl
            << "  b $8009 *$fd20 >= #$f0" << std::endl
            << "  b $8010 rx >= 80" << std::endl
            << "  b $4100 rp.n == 1" << std::endl;
        return false;
    }

    /*
     * Conditional Breakpoint.
     */
    cond_t cond{};
    if (args.size() > 2) {
        /*
         * Re-build the condition string and compile it.
         */
        std::string line{};
        for (auto it = args.begin() + 2; it != args.end(); ++it) {
            line += *it;
        }

        auto [expr, err] = Expr::compile(line);
        if (!expr) {
            self._os << err.what() << std::endl;
            return false;
        }

        cond = {expr, line};
    }

    /*
     * Breakpoint.
     */
    try {
        auto addr = static_cast<addr_t>(self.to_addr(args[1], self._cpu._regs.PC));
        self.add_breakpoint(addr, cond);

    } catch (const InvalidNumber &) {
        /* Error shown. Continue with the next argument */
        ;
    }

    return false;
}

bool Mos6510Monitor::bp_del(Mos6510Monitor &self, const std::vector<std::string> &args)
{
    /*
     * bpdel <addr>
     * bd <addr>
     */
    for (auto it = args.begin() + 1; it != args.end(); ++it) {
        try {
            addr_t addr = self.to_addr(*it, self._cpu._regs.PC);
            self.del_breakpoint(addr);
        } catch (const InvalidNumber &) {
            /* Error shown. Continue with the next argument */
            ;
        }
    }

    return false;
}

bool Mos6510Monitor::bp_clear(Mos6510Monitor &self, const std::vector<std::string> &args)
{
    /*
     * bpclear, bc
     */
    self._breakpoints.clear();
    return false;
}

bool Mos6510Monitor::bp_list(Mos6510Monitor &self, const std::vector<std::string> &args)
{
    /*
     * bplist, bl
     */
    for (const auto &kv : self._breakpoints) {
        auto &addr = kv.first;
        auto &cond = kv.second;
        auto &cfn  = cond.first;
        auto &cstr = cond.second;

        self._os << "$" << utils::to_string(addr);

        if (cfn) {
            self._os << " " << cstr;
        }

        self._os << ((addr == self._cpu._regs.PC) ? " <" : "") << std::endl;
    }

    return false;
}

bool Mos6510Monitor::go(Mos6510Monitor &self, const std::vector<std::string> &args)
{
    /*
     * go [<addr>|.]
     * g [<addr>|.]
     */
    try {
        if (args.size() > 1) {
            addr_t addr = static_cast<addr_t>(self.to_addr(args[1], self._cpu._regs.PC));
            self._cpu._regs.PC = addr;
        }

        self._prev_line = "";
        return true;

    } catch (const InvalidNumber &) {
        self._os << "Invalid address: " << args[1] << std::endl;
    }

    return false;
}

bool Mos6510Monitor::step(Mos6510Monitor &self, const std::vector<std::string> &args)
{
    /*
     * step [<addr>|.]
     * s [<addr>|.]
     */
    try {
        if (args.size() > 1) {
            addr_t addr = static_cast<addr_t>(self.to_addr(args[1], self._cpu._regs.PC));
            self._cpu._regs.PC = addr;
        }

        self._cpu.ebreak();
        return true;

    } catch (const InvalidNumber &) {
        self._os << "Invalid address: " << args[1] << std::endl;
    }

    return false;
}

bool Mos6510Monitor::load(Mos6510Monitor &self, const std::vector<std::string> &args)
{
    /*
     * load <filename> [$addr]
     */
    try {
        if (args.size() > 1) {
            addr_t addr{};
            if (args.size() > 2) {
                addr = utils::to_number<addr_t>(args[2]);
            }

            Prg prog{args[1], addr};
            addr = prog.addr();
            for (auto c : prog) {
                self._cpu._mmap->write(addr++, c);
            }

            addr_t size = prog.size();

            self._os << "load: " << args[1] << " loaded at $" << utils::to_string(prog.addr())
                << ", size " << size << " ($" << utils::to_string(size) << ")" << std::endl;
        }
    } catch (const std::exception &e) {
        self._os << e.what() << std::endl;
    }

    return false;
}

bool Mos6510Monitor::save(Mos6510Monitor &self, const std::vector<std::string> &args)
{
    /*
     * save <filename> $start $end
     */
    try {
        if (args.size() != 4) {
            throw InvalidArgument{"Invalid number of arguments"};
        }

        const std::string &fname = args[1];
        addr_t start = utils::to_number<addr_t>(args[2]);
        addr_t end = utils::to_number<addr_t>(args[3]);

        if (end < start) {
            throw InvalidArgument{"End address smaller than start address"};
        }

        Prg prog{};
        for (auto addr = start; addr <= end; ++addr) {
            uint8_t c = self._cpu._mmap->read(addr);
            prog.push_back(c);
        }

        prog.save(fname, start);

    } catch (const std::exception &e) {
        self._os << e.what() << std::endl;
    }

    return false;
}

bool Mos6510Monitor::loglevel(Mos6510Monitor &self, const std::vector<std::string> &args)
{
    /*
     * loglevel <lv>
     * l <lv>
     */
    try {
        if (args.size() != 2) {
            unsigned lv = self._cpu.loglevel();
            self._os << lv << std::endl;
        } else {
            self._cpu.loglevel(args[1]);
        }

    } catch (const std::exception &e) {
        self._os << e.what() << std::endl;
    }

    return false;
}

bool Mos6510Monitor::quit(Mos6510Monitor &self, const std::vector<std::string> &args)
{
    /*
     * quit, q
     */
    if (args.size() > 1) {
        int eval = std::atoi(args[1].c_str());
        self._os << "Emulator terminated with exit code: " << eval << std::endl;
        std::exit(eval);
    }

    self._is_running = false;
    return false;
}

bool Mos6510Monitor::help(Mos6510Monitor &self, const std::vector<std::string> &args)
{
    /*
     * help, h, ?
     */
    self._os << "Monitor Commands: " << std::endl;

    for (const auto &cmd : self.commands) {
        std::ostringstream oss{};
        oss << cmd.command << " " << cmd.args;
        self._os << std::setw(3) << std::right << cmd.short_command << " | "
                 << std::setw(24) << std::left << oss.str() << cmd.help << std::endl;
    }

    self._os << "values without a prefix or prefixed by '$' are considered hexadecimal" << std::endl
             << "values prefixed only by '#' are considered decimal numbers" << std::endl;

    return false;
}

}
