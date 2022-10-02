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
#include "monitor.hpp"

#include <algorithm>
#include <sstream>

#include "utils.hpp"


namespace caio {

const std::vector<std::pair<std::string, Monitor::Expr::ex_t>> Monitor::Expr::operators{
    { "<=", [](MonitoredCpu &cpu, Monitor::Expr::fn_t &a, Monitor::Expr::fn_t &b) {
        return a(cpu) <= b(cpu);
    }},
    { ">=", [](MonitoredCpu &cpu, Monitor::Expr::fn_t &a, Monitor::Expr::fn_t &b) {
        return a(cpu) >= b(cpu);
    }},
    { "==", [](MonitoredCpu &cpu, Monitor::Expr::fn_t &a, Monitor::Expr::fn_t &b) {
        return a(cpu) == b(cpu);
    }},
    { "!=", [](MonitoredCpu &cpu, Monitor::Expr::fn_t &a, Monitor::Expr::fn_t &b) {
        return a(cpu) != b(cpu);
    }},
    { "<", [](MonitoredCpu &cpu, Monitor::Expr::fn_t &a, Monitor::Expr::fn_t &b) {
        return a(cpu) < b(cpu);
    }},
    { ">", [](MonitoredCpu &cpu, Monitor::Expr::fn_t &a, Monitor::Expr::fn_t &b) {
        return a(cpu) > b(cpu);
    }},
    { "&", [](MonitoredCpu &cpu, Monitor::Expr::fn_t &a, Monitor::Expr::fn_t &b) {
        return a(cpu) & b(cpu);
    }},
    { "|", [](MonitoredCpu &cpu, Monitor::Expr::fn_t &a, Monitor::Expr::fn_t &b) {
        return a(cpu) | b(cpu);
    }}
};


Monitor::Expr::fn_t Monitor::Expr::compile_argument(MonitoredCpu &cpu, const std::string &line)
{
    /*
     * <line> = ["*"]<register_name> | ["*"]["#"]["$"]<number>
     *
     * <register_name> is the name of a monitored's cpu register.
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
            return [isref, lit](MonitoredCpu &cpu) -> int {
                uint16_t val = static_cast<uint16_t>(lit);
                return (isref ? cpu.read(val) : val);
            };
        }

        /*
         * Try to compile a register name.
         */
        try {
            cpu.regvalue(svalue);     /* Exception if svalue is not a register name */

            return [isref, svalue](MonitoredCpu &cpu) -> int {
                addr_t val = cpu.regvalue(svalue);
                return (isref ? cpu.read(val) : val);
            };

        } catch (const InvalidArgument &) {
            /* Not a register name */
            ;
        }
    }

    /*
     * The line does not contain a literal value or a register name.
     */
    std::stringstream os{};
    os << "Invalid expression: " << std::quoted(line);
    throw InvalidArgument(os.str());
}

std::pair<std::function<int()>, std::exception> Monitor::Expr::compile(MonitoredCpu &cpu, const std::string &line)
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
                const auto carg1 = compile_argument(cpu, arg1);
                const auto carg2 = compile_argument(cpu, arg2);
                const auto oper{op.second};
                return {std::bind(std::move(oper), cpu, std::move(carg1), std::move(carg2)), {}};
            }
        }

        std::ostringstream os{};
        os << "Invalid expression: " << std::quoted(line);
        return {{}, InvalidArgument{os.str()}};

    } catch (const std::exception &err) {
        return {{}, std::move(err)};
    }
}

Monitor::Monitor(std::istream &is, std::ostream &os, MonitoredCpu &&cpu)
    : _is{is},
      _os{os},
      _cpu{std::move(cpu)},
      _commands{
          { "assemble", "a", ".|$addr", "Assemble machine code from $addr",
              [this](const cmdargs_t &args) { return this->assemble(args); }

          }, { "disass", "d", ".|$addr [$n]", "Disassemble $n instructions from $addr",
              [this](const cmdargs_t &args) { return this->disassemble(args); }},

          { "dump", "x", ". [$n]", "Dump $n bytes of memory from $addr",
              [this](const cmdargs_t &args) { return this->dump(args); }},

          { "regs", "r", "", "Show registers",
              [this](const cmdargs_t &args) { return this->registers(args); }},

          { "mmap", "m", "", "Show memory map",
              [this](const cmdargs_t &args) { return this->mmap(args); }},

          { "bpadd", "b", "$addr", "Add a breakpoint at $addr",
              [this](const cmdargs_t &args) { return this->bp_add(args); }},

          { "bpadd", "b", "$addr <cond>", "Add a conditional breakpoint at $addr",
              {}},

          { "bpadd", "b", "help|h|?", "Help about breakpoints",
              {}},

          { "bpdel", "bd", "$addr", "Delete breakpoint at $addr",
              [this](const cmdargs_t &args) { return this->bp_del(args); }},

          { "bpclear", "bc", "", "Clear all breakpoints",
              [this](const cmdargs_t &args) { return this->bp_clear(args); }},

          { "bplist", "bl", "", "List breakpoints",
              [this](const cmdargs_t &args) { return this->bp_list(args); }},

          { "go", "g", ".|$addr", "Run program at $addr",
              [this](const cmdargs_t &args) { return this->go(args); }},

          { "si", "s", "[.|$addr]", "Execute single instruction",
              [this](const cmdargs_t &args) { return this->step(args); }},

          { "load", "l", "<prog> [$addr]", "Load a binary or PRG file",
              [this](const cmdargs_t &args) { return this->load(args); }},

          { "save", "w", "<prog> $start $end", "Save a PRG file",
              [this](const cmdargs_t &args) { return this->save(args); }},

          { "loglevel", "lv", "<lv>", "Set the CPU loglevel",
              [this](const cmdargs_t &args) { return this->loglevel(args); }},

          { "quit", "q", "", "Halt the CPU",
              [this](const cmdargs_t &args) { return this->quit(args); }},

          { "quit", "q", "<e>", "Terminate the emulator with exit code",
              {}},

          { "help", "h", "", "This help",
              [this](const cmdargs_t &args) { return this->help(args); }},

          { "help", "?", "", "",
              [this](const cmdargs_t &args) { return this->help(args); }}
      }
{
}

Monitor::~Monitor()
{
}

bool Monitor::run()
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

        auto it = std::find_if(_commands.begin(), _commands.end(), [&args](const Command &cmd) {
            return (cmd.command == args[0] || cmd.short_command == args[0]);
        });

        if (it == _commands.end()) {
            _os << "Invalid command: " << args[0] << std::endl;
            continue;
        }

        _prev_line = line;
        _prev_fn = it->short_command;

        if (it->fn(args)) {
            /*
             * The command wants to exit the monitor letting
             * the system to continue with the CPU emulation.
             */
            return true;
        }
    }

    /* The user wants to terminate the session or some other error occurred */
    return false;
}

void Monitor::add_breakpoint(addr_t addr, const cond_t &cond)
{
    _breakpoints[addr] = cond;
}

void Monitor::del_breakpoint(addr_t addr)
{
    _breakpoints.erase(addr);
}

bool Monitor::is_breakpoint(addr_t addr) const
{
    auto it = _breakpoints.find(addr);
    if (it != _breakpoints.end()) {
        auto &cond = it->second;
        auto &fn = cond.first;
        if (!fn) {
            /*
             * Unconditional breakpoint.
             */
            _os << "Breakpoint at $" << utils::to_string(addr) << std::endl;
            return true;
        }

        if (fn() != 0) {
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

std::string Monitor::prompt()
{
    std::stringstream os{};

    if (_prev_fn == "s") {
        _cpu.disass(_os, _cpu.pc(), 10, true);
        _cpu.regs(_os);
        _os << std::endl;
    }

    _os << PROMPT_PREFIX << "$" << utils::to_string(_cpu.pc()) << PROMPT_SUFFIX;

    return os.str();
}

addr_t Monitor::to_addr(const std::string &str, addr_t defval)
{
    return ((str == ".") ? defval : static_cast<addr_t>(to_count(str)));
}

size_t Monitor::to_count(const std::string &str)
{
    try {
        return utils::to_number<size_t>(str);

    } catch (const InvalidNumber &err) {
        _os << "Invalid value: " << str << std::endl;
        throw err;
    }
}

bool Monitor::assemble(const cmdargs_t &args)
{
    /*
     * assemble [<addr>|.]
     * a [<addr>|.]
     */
    addr_t addr = _cpu.pc();
    for (auto it = args.begin() + 1; it != args.end(); ++it) {
        try {
            addr = to_addr(*it, addr);

        } catch (InvalidNumber &) {
            /* Error shown. Continue with next one */
            ;
        }
    }

    _os << "Entering edit mode. To finish write '.' or an empty line" << std::endl;

    while (true) {
        _os << "$" << utils::to_string(addr) << ": " << std::flush;

        std::string line{};
        if (!std::getline(_is, line, '\n')) {
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
                _os << "Invalid value: " << str << std::endl;
                program.clear();
                break;
            }
        }

        /*
         * Write the user edited program line into memory.
         */
        try {
            for (uint8_t u8 : program) {
                _cpu.write(addr++, u8);
            }
        } catch (const std::exception &err) {
            /*
             * Error, exit from edit mode.
             */
            _os << "Unexpected: " << err.what() << std::endl;
            break;
        }
    }

    return false;
}

bool Monitor::disassemble(const cmdargs_t &args)
{
    /*
     * disass [<addr> [<count>]]
     * d [<addr> [<count>]]
     */
    addr_t addr = _cpu.pc();
    size_t count = 16;

    try {
        if (args.size() > 2) {
            count = to_count(args[2]);
        }
        if (args.size() > 1) {
            addr = to_addr(args[1], addr);
        }
    } catch (const InvalidNumber &) {
        return false;
    }

    _cpu.disass(_os, addr, count, true);
    return false;
}

bool Monitor::dump(const cmdargs_t &args)
{
    /*
     * dump [<addr> [<count>]]
     * x [<addr> [<count>]]
     */
    addr_t addr = _cpu.pc();
    size_t count = 16;

    try {
        if (args.size() > 2) {
            count = to_count(args[2]);
        }

        if (args.size() > 1) {
            addr = to_addr(args[1], addr);
        }

    } catch (const InvalidNumber &) {
        return false;
    }

    if (count == 0) {
        count = 0xFFFF - addr + 1;
    }

    addr_t ra = addr;

    std::vector<uint8_t> data(count);

    std::generate(data.begin(), data.end(), [this, &ra]() -> uint8_t {
        return _cpu.read(ra++);
    });

    utils::dump(_os, data, addr) << std::endl;

    return false;
}

bool Monitor::registers(const cmdargs_t &args)
{
    /*
     * registers, r
     */
    _cpu.regs(_os);
    _os << std::endl;
    return false;
}

bool Monitor::mmap(const cmdargs_t &args)
{
    /*
     * mmap, m
     */
    _cpu.mmap()->dump(_os) << std::endl;
    return false;
}

bool Monitor::bp_add(const cmdargs_t &args)
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
        _os << args[0] << " help | h | ?" << std::endl
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

        auto [expr, err] = Expr::compile(_cpu, line);
        if (!expr) {
            _os << err.what() << std::endl;
            return false;
        }

        cond = {expr, line};
    }

    /*
     * Breakpoint.
     */
    try {
        auto addr = static_cast<addr_t>(to_addr(args[1], _cpu.pc()));
        add_breakpoint(addr, cond);

    } catch (const InvalidNumber &) {
        /* Error shown. Continue with the next argument */
        ;
    }

    return false;
}

bool Monitor::bp_del(const cmdargs_t &args)
{
    /*
     * bpdel <addr>
     * bd <addr>
     */
    for (auto it = args.begin() + 1; it != args.end(); ++it) {
        try {
            addr_t addr = to_addr(*it, _cpu.pc());
            del_breakpoint(addr);
        } catch (const InvalidNumber &) {
            /* Error shown. Continue with the next argument */
            ;
        }
    }

    return false;
}

bool Monitor::bp_clear(const cmdargs_t &args)
{
    /*
     * bpclear, bc
     */
    _breakpoints.clear();
    return false;
}

bool Monitor::bp_list(const cmdargs_t &args)
{
    /*
     * bplist, bl
     */
    for (const auto &kv : _breakpoints) {
        auto &addr = kv.first;
        auto &cond = kv.second;
        auto &cfn  = cond.first;
        auto &cstr = cond.second;

        _os << "$" << utils::to_string(addr);

        if (cfn) {
            _os << " " << cstr;
        }

        _os << ((addr == _cpu.pc() )? " <" : "") << std::endl;
    }

    return false;
}

bool Monitor::go(const cmdargs_t &args)
{
    /*
     * go [<addr>|.]
     * g [<addr>|.]
     */
    try {
        if (args.size() > 1) {
            addr_t addr = static_cast<addr_t>(to_addr(args[1], _cpu.pc()));
            _cpu.pc() = addr;
        }

        _prev_line = "";
        return true;

    } catch (const InvalidNumber &) {
        _os << "Invalid address: " << args[1] << std::endl;
    }

    return false;
}

bool Monitor::step(const cmdargs_t &args)
{
    /*
     * step [<addr>|.]
     * s [<addr>|.]
     */
    try {
        if (args.size() > 1) {
            addr_t addr = static_cast<addr_t>(to_addr(args[1], _cpu.pc()));
            _cpu.pc() = addr;
        }

        _cpu.ebreak();
        return true;

    } catch (const InvalidNumber &) {
        _os << "Invalid address: " << args[1] << std::endl;
    }

    return false;
}

bool Monitor::load(const cmdargs_t &args)
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

            auto size = _cpu.load(args[1], addr);

            _os << "load: " << args[1] << " loaded at $" << utils::to_string(addr)
                << ", size " << size << " ($" << utils::to_string(size) << ")" << std::endl;
        }
    } catch (const std::exception &e) {
        _os << e.what() << std::endl;
    }

    return false;
}

bool Monitor::save(const cmdargs_t &args)
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

        _cpu.save(fname, start, end);

    } catch (const std::exception &e) {
        _os << e.what() << std::endl;
    }

    return false;
}

bool Monitor::loglevel(const cmdargs_t &args)
{
    /*
     * loglevel <lv>
     * l <lv>
     */
    try {
        if (args.size() != 2) {
            unsigned lv = _cpu.loglevel({});
            _os << lv << std::endl;
        } else {
            _cpu.loglevel(args[1]);
        }

    } catch (const std::exception &e) {
        _os << e.what() << std::endl;
    }

    return false;
}

bool Monitor::quit(const cmdargs_t &args)
{
    /*
     * quit, q
     */
    if (args.size() > 1) {
        int eval = std::atoi(args[1].c_str());
        _os << "Emulator terminated with exit code: " << eval << std::endl;
        std::exit(eval);
    }

    _is_running = false;
    return false;
}

bool Monitor::help(const cmdargs_t &args)
{
    /*
     * help, h, ?
     */
    _os << "Monitor Commands: " << std::endl;

    for (const auto &cmd : _commands) {
        std::ostringstream oss{};
        oss << cmd.command << " " << cmd.args;
        _os << std::setw(3) << std::right << cmd.short_command << " | "
            << std::setw(24) << std::left << oss.str() << cmd.help << std::endl;
    }

    _os << "values without a prefix or prefixed by '$' are considered hexadecimal" << std::endl
        << "values prefixed only by '#' are considered decimal numbers" << std::endl;

    return false;
}

}
