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
#include "monitor.hpp"

#include <algorithm>
#include <sstream>

#include "utils.hpp"

namespace caio {
namespace monitor {

using namespace std::literals::string_literals;

const std::pair<std::string, Expr::ex_t> Expr::operators[] = {
    { "<=", [](MonitoredCPU* cpu, Expr::fn_t& a, Expr::fn_t& b) { return a(*cpu) <= b(*cpu); }},
    { ">=", [](MonitoredCPU* cpu, Expr::fn_t& a, Expr::fn_t& b) { return a(*cpu) >= b(*cpu); }},
    { "==", [](MonitoredCPU* cpu, Expr::fn_t& a, Expr::fn_t& b) { return a(*cpu) == b(*cpu); }},
    { "!=", [](MonitoredCPU* cpu, Expr::fn_t& a, Expr::fn_t& b) { return a(*cpu) != b(*cpu); }},
    { "<",  [](MonitoredCPU* cpu, Expr::fn_t& a, Expr::fn_t& b) { return a(*cpu) < b(*cpu);  }},
    { ">",  [](MonitoredCPU* cpu, Expr::fn_t& a, Expr::fn_t& b) { return a(*cpu) > b(*cpu);  }},
    { "&",  [](MonitoredCPU* cpu, Expr::fn_t& a, Expr::fn_t& b) { return a(*cpu) & b(*cpu);  }},
    { "|",  [](MonitoredCPU* cpu, Expr::fn_t& a, Expr::fn_t& b) { return a(*cpu) | b(*cpu);  }}
};

Expr::fn_t Expr::compile_argument(MonitoredCPU& cpu, const std::string& line)
{
    /*
     * <line> = ["*"]<register_name> | ["*"]["#"]["$"]<number>
     *
     * <register_name> is the name of a cpu register.
     */
    auto pos = line.find_first_not_of(" \n\r\v\t");
    if (pos != std::string::npos) {
        auto last = line.find_last_not_of(" \n\r\v\t");
        if (last == std::string::npos) {
            last = line.size();
        }

        /*
         * Detect if it is a value or a reference to a value stored in memory.
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
             * #xx means decimal.
             */
            base = 10;
            ++pos;
        }

        if (line[pos] == '$') {
            /*
             * $#xx and $xx mean hexadecimal.
             */
            base = 16;
            ++pos;
        }

        std::string svalue = caio::tolow(line.substr(pos, last - pos + 1));

        /*
         * Try to compile a literal value.
         */
        char* e{};
        uint64_t lit = std::strtoull(svalue.c_str(), &e, base);
        if (*e == '\0') {
            /*
             * It is a literal value.
             */
            return [isref, lit](MonitoredCPU& cpu) -> int {
                uint16_t val = static_cast<uint16_t>(lit);
                return (isref ? cpu.peek(val) : val);
            };
        }

        /*
         * Try to compile a register name.
         */
        try {
            cpu.regvalue(svalue);   /* Exception if svalue is not a register name */

            return [isref, svalue](MonitoredCPU& cpu) -> int {
                addr_t val = cpu.regvalue(svalue);
                return (isref ? cpu.peek(val) : val);
            };

        } catch (const std::exception&) {
            /* Not a register name */
            ;
        }
    }

    /*
     * The line does not contain a literal value or a register name.
     */
    std::stringstream os{};
    os << "Invalid argument expression: " << std::quoted(line);
    throw InvalidArgument(os.str());
}

std::function<int()> Expr::compile(MonitoredCPU& cpu, const std::string& line)
{
    /*
     * <expr> = <val1> <op> <val2>
     */
    for (const auto& op : operators) {
        const auto& name = op.first;
        auto pos = line.find(name);
        if (pos != std::string::npos) {
            const auto arg1 = line.substr(0, pos);
            const auto arg2 = line.substr(pos + name.size());
            const auto carg1 = compile_argument(cpu, arg1);
            const auto carg2 = compile_argument(cpu, arg2);
            const auto oper{op.second};
            return std::bind(std::move(oper), &cpu, std::move(carg1), std::move(carg2));
        }
    }

    std::ostringstream os{};
    os << "Invalid expression: " << std::quoted(line);
    throw InvalidArgument{os.str()};
}

Command Monitor::commands[] = {
    { "assemble", "a",  ".|$addr",              "Assemble machine code from $addr",         Monitor::assemble       },
    { "disass",   "d",  ".|$addr [n]",          "Disassemble n instructions from $addr",    Monitor::disassemble    },
    { "dump",     "x",  "$addr [n]",            "Dump n bytes of memory from $addr",        Monitor::dump           },
    { "dump",     "x",  ". [n]",                "Dump n bytes of memory from PC address",   Monitor::dump           },
    { "regs",     "r",  "",                     "Show registers",                           Monitor::registers      },
    { "mmap",     "m",  "",                     "Show memory map",                          Monitor::mmap           },
    { "bpadd",    "b",  "$addr",                "Add a breakpoint at $addr",                Monitor::bp_add         },
    { "bpadd",    "b",  "$addr cond",           "Add a conditional breakpoint at $addr",    Monitor::bp_add         },
    { "bpadd",    "b",  "help|h|?",             "Help about breakpoints",                   Monitor::bp_add         },
    { "bpdel",    "bd", "$addr",                "Delete breakpoint at $addr",               Monitor::bp_del         },
    { "bpclear",  "bc", "",                     "Clear all breakpoints",                    Monitor::bp_clear       },
    { "bplist",   "bl", "",                     "List breakpoints",                         Monitor::bp_list        },
    { "go",       "g",  ".|$addr",              "Run program at $addr",                     Monitor::go             },
    { "si",       "s",  "[.|$addr]",            "Execute single instruction at $addr",      Monitor::step           },
    { "load",     "l",  "fname [$addr]",        "Load a binary file",                       Monitor::load           },
    { "save",     "w",  "fname $start $end",    "Create a binary file",                     Monitor::save           },
    { "loglevel", "lv", "loglevel",             "Set the CPU loglevel",                     Monitor::loglevel       },
    { "fc",       "fc", "",                     "Show command history",                     Monitor::history        },
    { "quit",     "q",  "[code]",               "Terminate the emulator with exit code",    Monitor::quit           },
    { "help",     "h",  "",                     "This help",                                Monitor::help           },
    { "help",     "?",  "",                     "",                                         Monitor::help           }
};

Monitor::Monitor(int ifd, int ofd, MonitoredCPU&& cpu)
    : _rd{ifd, ofd},
      _cpu{cpu}
{
}

bool Monitor::run()
{
    _is_running = true;

    while (_is_running) {
        _rd.write(prompt());

        std::string line = _rd.getline();
        line = (line.empty() ? _prev_line : caio::trim(line));
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::vector<std::string> args{};
        std::string arg{};
        std::istringstream iss{line};
        while (iss >> arg) {
            args.push_back(arg);
        }

        auto begin = &commands[0];
        auto end = begin + std::size(commands);
        auto it = std::find_if(begin, end, [&args](const Command& cmd) {
            return (cmd.command == args[0] || cmd.short_command == args[0]);
        });

        if (it == end) {
            _rd.write("Invalid command: " + args[0] + "\n");
            continue;
        }

        _prev_line = line;
        _prev_fn = it->short_command;

        if (it->fn(*this, args)) {
            /*
             * Exit the monitor and continue the emulation.
             */
            return true;
        }
    }

    /* Exit the monitor and terminate the emulation */
    return false;
}

bool Monitor::is_breakpoint(addr_t addr) const
{
    auto it = _breakpoints.find(addr);
    if (it != _breakpoints.end()) {
        auto& cond = it->second;
        auto& fn = cond.first;
        if (!fn) {
            /*
             * Unconditional breakpoint.
             */
            _rd.write("Breakpoint at $" + caio::to_string(addr) + "\n");
            return true;
        }

        if (fn() != 0) {
            /*
             * Conditional breakpoint.
             */
            _rd.write("Conditional breakpoint at $" + caio::to_string(addr) + " " + cond.second + "\n");
            return true;
        }
    }

    /*
     * No breakpont.
     */
    return false;
}

std::string Monitor::prompt()
{
    std::stringstream os{};

    if (_prev_fn == "s") {
        _cpu.disass(os, _cpu.getpc(), 10, true);
        os << _cpu.regs() << "\n";
    }

    os << PROMPT_PREFIX << "$" << caio::to_string(_cpu.getpc()) << PROMPT_SUFFIX;

    return os.str();
}

inline addr_t Monitor::to_addr(const std::string& str, addr_t defval)
{
    return ((str == ".") ? defval : static_cast<addr_t>(to_count(str)));
}

size_t Monitor::to_count(const std::string& str)
{
    try {
        return caio::to_number<size_t>(str);

    } catch (const InvalidNumber& err) {
        _rd.write("Invalid value: " + str + "\n");
        throw err;
    }
}

bool Monitor::assemble(Monitor& mon, const Command::args_t& args)
{
    /*
     * assemble [<addr>|.]
     * a [<addr>|.]
     */
    addr_t addr = mon._cpu.getpc();
    for (auto it = args.begin() + 1; it != args.end(); ++it) {
        try {
            addr = mon.to_addr(*it, addr);

        } catch (InvalidNumber&) {
            /* Error shown. Continue with next one */
            ;
        }
    }

    mon._rd.write(std::string{"Entering edit mode. To finish write '.' or an empty line\n"});

    auto [ifd, ofd] = mon._rd.fds();
    Readline editor{ifd, ofd};
    while (true) {
        editor.write("$"s + caio::to_string(addr) + ": "s);

        std::string line = editor.getline();
        line = caio::trim(line);

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
                auto u8 = caio::to_number<uint8_t>(str);
                program.push_back(u8);
            } catch (const InvalidNumber&) {
                /*
                 * Show the error and invalidate the whole line.
                 */
                editor.write("Invalid value: " + str + "\n");
                program.clear();
                break;
            }
        }

        /*
         * Write the user edited program line into memory.
         */
        try {
            for (uint8_t u8 : program) {
                mon._cpu.write(addr++, u8);
            }
        } catch (const std::exception& err) {
            /*
             * Error, exit from edit mode.
             */
            std::ostringstream os{};
            os << "Unexpected error: " << err.what() << "\nExiting edit mode.\n";
            editor.write(os.str());
            break;
        }
    }

    return false;
}

bool Monitor::disassemble(Monitor& mon, const Command::args_t& args)
{
    /*
     * disass [<addr> [<count>]]
     * d [<addr> [<count>]]
     */
    addr_t addr = mon._cpu.getpc();
    size_t count = 16;

    try {
        if (args.size() > 2) {
            count = mon.to_count(args[2]);
        }
        if (args.size() > 1) {
            addr = mon.to_addr(args[1], addr);
        }
    } catch (const InvalidNumber&) {
        return false;
    }

    std::ostringstream os{};
    mon._cpu.disass(os, addr, count, true);
    mon._rd.write(os.str());
    return false;
}

bool Monitor::dump(Monitor& mon, const Command::args_t& args)
{
    /*
     * dump [<addr> [<count>]]
     * x [<addr> [<count>]]
     */
    addr_t addr = mon._cpu.getpc();
    size_t count = 16;

    try {
        if (args.size() > 2) {
            count = mon.to_count(args[2]);
        }

        if (args.size() > 1) {
            addr = mon.to_addr(args[1], addr);
        }

    } catch (const InvalidNumber&) {
        return false;
    }

    if (count == 0) {
        count = 0xFFFF - addr + 1;
    }

    addr_t ra = addr;

    std::vector<uint8_t> data(count);

    std::generate(data.begin(), data.end(), [&mon, &ra]() -> uint8_t {
        return mon._cpu.peek(ra++);
    });

    std::ostringstream os{};
    caio::dump(os, data, addr) << "\n";
    mon._rd.write(os.str());
    return false;
}

bool Monitor::registers(Monitor& mon, const Command::args_t& args)
{
    /*
     * registers, r
     */
    std::ostringstream os{};
    os << mon._cpu.regs() << "\n";
    mon._rd.write(os.str());
    return false;
}

bool Monitor::mmap(Monitor& mon, const Command::args_t& args)
{
    /*
     * mmap, m
     */
    std::ostringstream os{};
    mon._cpu.mmap()->dump(os) << "\n";
    mon._rd.write(os.str());
    return false;
}

bool Monitor::bp_add(Monitor& mon, const Command::args_t& args)
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
        mon._rd.write(mon._cpu.bpdoc(args[0]));
        return false;
    }

    /*
     * Compile the condition (if any).
     */
    cond_t cond{};

    if (args.size() > 2) {
        /* Get the condition string */
        std::string line{};
        for (auto it = args.begin() + 2; it != args.end(); ++it) {
            line += *it;
        }

        /* Compile the condition string */
        try {
            auto expr = Expr::compile(mon._cpu, line);
            cond = {expr, line};
        } catch (const std::exception& err) {
            mon._rd.write(std::string{err.what()} + "\n");
            return false;
        }
    }

    /*
     * Add breakpoint.
     */
    try {
        auto addr = static_cast<addr_t>(mon.to_addr(args[1], mon._cpu.getpc()));
        mon.add_breakpoint(addr, cond);

    } catch (const InvalidNumber&) {
        /* Error shown. Continue with the next argument */
        ;
    }

    return false;
}

bool Monitor::bp_del(Monitor& mon, const Command::args_t& args)
{
    /*
     * bpdel <addr>
     * bd <addr>
     */
    for (auto it = args.begin() + 1; it != args.end(); ++it) {
        try {
            addr_t addr = mon.to_addr(*it, mon._cpu.getpc());
            mon.del_breakpoint(addr);
        } catch (const InvalidNumber&) {
            /* Error shown. Continue with the next argument */
            ;
        }
    }

    return false;
}

bool Monitor::bp_clear(Monitor& mon, const Command::args_t& args)
{
    /*
     * bpclear, bc
     */
    mon._breakpoints.clear();
    return false;
}

bool Monitor::bp_list(Monitor& mon, const Command::args_t& args)
{
    /*
     * bplist, bl
     */
    std::ostringstream os{};

    for (const auto& kv : mon._breakpoints) {
        auto& addr = kv.first;
        auto& cond = kv.second;
        auto& cfn  = cond.first;
        auto& cstr = cond.second;

        os << "$" << caio::to_string(addr);

        if (cfn) {
            os << " " << cstr;
        }

        os << ((addr == mon._cpu.getpc() )? " <\n" : "\n");
    }

    mon._rd.write(os.str());
    return false;
}

bool Monitor::go(Monitor& mon, const Command::args_t& args)
{
    /*
     * go [<addr>|.]
     * g [<addr>|.]
     */
    try {
        if (args.size() > 1) {
            addr_t addr = static_cast<addr_t>(mon.to_addr(args[1], mon._cpu.getpc()));
            mon._cpu.setpc(addr);
        }

        mon._prev_line = "g";
        return true;

    } catch (const std::exception&) {
        mon._rd.write(std::string{"Invalid address: "} + args[1] + "\n");
    }

    return false;
}

bool Monitor::step(Monitor& mon, const Command::args_t& args)
{
    /*
     * step [<addr>|.]
     * s [<addr>|.]
     */
    try {
        if (args.size() > 1) {
            addr_t addr = static_cast<addr_t>(mon.to_addr(args[1], mon._cpu.getpc()));
            mon._cpu.setpc(addr);
        }

        mon._cpu.ebreak();
        mon._prev_line = "s";
        return true;

    } catch (const std::exception&) {
        mon._rd.write("Invalid address: "s + args[1] + "\n");
    }

    return false;
}

bool Monitor::load(Monitor& mon, const Command::args_t& args)
{
    /*
     * load <filename> [$addr]
     */
    try {
        if (args.size() > 1) {
            addr_t addr{};

            if (args.size() > 2) {
                addr = caio::to_number<addr_t>(args[2]);
            }

            auto [start, size] = mon._cpu.load(args[1], addr);

            std::ostringstream os{};
            os << "load: " << args[1] << " loaded at $" << caio::to_string(addr)
               << ", size " << size << " ($" << caio::to_string(size) << ")\n";
            mon._rd.write(os.str());
        }
    } catch (const std::exception& e) {
        mon._rd.write(std::string{e.what()} + "\n");
    }

    return false;
}

bool Monitor::save(Monitor& mon, const Command::args_t& args)
{
    /*
     * save <filename> $start $end
     */
    try {
        if (args.size() != 4) {
            throw InvalidArgument{"Invalid number of arguments"};
        }

        const std::string& fname = args[1];
        addr_t start = caio::to_number<addr_t>(args[2]);
        addr_t end = caio::to_number<addr_t>(args[3]);

        if (end < start) {
            throw InvalidArgument{"End address smaller than start address"};
        }

        mon._cpu.save(fname, start, end);

    } catch (const std::exception& e) {
        mon._rd.write(std::string{e.what()} + "\n");
    }

    return false;
}

bool Monitor::loglevel(Monitor& mon, const Command::args_t& args)
{
    /*
     * loglevel <lv>
     * l <lv>
     */
    try {
        if (args.size() != 2) {
            unsigned lv = mon._cpu.loglevel({});
            mon._rd.write(std::to_string(lv) + "\n");
        } else {
            mon._cpu.loglevel(args[1]);
        }

    } catch (const std::exception& e) {
        mon._rd.write(std::string{e.what()} + "\n");
    }

    return false;
}

bool Monitor::history(Monitor& mon, const Command::args_t& args)
{
    /*
     * fc
     */
    mon._rd.write(mon._rd.history());
    return false;
}

bool Monitor::quit(Monitor& mon, const Command::args_t& args)
{
    /*
     * quit, q
     */
    if (args.size() > 1) {
        std::ostringstream os{};
        int eval = std::atoi(args[1].c_str());
        os << "Emulator terminated with exit code: " << eval << "\n";
        mon._rd.write(os.str());
        std::exit(eval);
    }

    mon._is_running = false;
    return false;
}

bool Monitor::help(Monitor& mon, const Command::args_t& args)
{
    /*
     * help, h, ?
     */
    std::ostringstream os{};

    os << "Monitor Commands:\n";

    for (const auto& cmd : commands) {
        std::ostringstream oss{};
        oss << cmd.command << " " << cmd.args;
        os << std::setw(3) << std::right << cmd.short_command << " | "
           << std::setw(24) << std::left << oss.str() << cmd.help << "\n";
    }

    os << "values without a prefix or prefixed by '$' are considered hexadecimal\n"
          "values prefixed only by '#' are considered decimal numbers\n";

    mon._rd.write(os.str());

    return false;
}

}
}
