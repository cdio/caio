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
#pragma once

#include <cstdint>
#include <functional>
#include <istream>
#include <limits>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "types.hpp"


namespace cemu {

/**
 * The monitor is used to debug an emulated a MOS6510 cpu when it is running.
 * The user communicates with the monitor through specified input/output streams.
 */
class Mos6510Monitor {
public:
    /**
     * Conditional Breakpoint expression compiler.
     * A conditional expression is a string formatted as "value1 operator value2"
     * where operator is one of: "<", ">", "<=", ">=", "==", "!=", "&", "|"
     * value1 and value2 are simple expressions containing one of:
     * - Literal value:      "$D020", "D020", "#53280", "#$A7"
     * - Memory reference:   "*D020", "*$D020", "*#53280"
     * - Register reference: "ra", "rx", "ry", "rs", "rp", "rp.n" "rp.v", "rp.b", "rp.d", "rp.i", "rp.z", "rp.c"
     *   Register references can be used along with "*" to get the content of a zero-page address (for exmaple: "*ra").
     *
     * Examples of a breakpoint using the monitor command line:
     *  b $8011 ra >= $20
     *  b $8011 *d020 >= #15
     */
    class Expr {
    public:
        using fn_t = std::function<int(const class Mos6510 &)>;
        using ex_t = std::function<int(const class Mos6510 &, const fn_t &, const fn_t &)>;

        Expr() = delete;

        ~Expr() = delete;

        /**
         * Compile the user defined condition.
         * @param line String containing the expression to compile.
         * @return On success the first element contains the compiled expression; on error the second element
         * contains the exception. To detect an error condition the bool method of the first element must be called.
         */
        static std::pair<fn_t, std::exception> compile(const std::string &line);

    private:
        /**
         * Compile an expression argument string.
         * The argument must be a literal value, a memory reference or a register reference.
         * @param line The expression argument.
         * @return The compiled expression argument.
         */
        static fn_t compile_argument(const std::string &line);

        static const std::vector<std::pair<std::string, ex_t>> operators;
        static const std::vector<std::pair<std::string, fn_t>> registers;
    };


    using cond_t = std::pair<Expr::fn_t, std::string>;


    /**
     * Initialise this MOS6510 monitor.
     * @param is  Input stream used to read commands from the user;
     * @param os  Output stream used to send responses to the user;
     * @param cpu The monitored CPU.
     */
    Mos6510Monitor(std::istream &is, std::ostream &os, class Mos6510 &cpu)
        : _is{is},
          _os{os},
          _cpu{cpu} {
    }

    virtual ~Mos6510Monitor() {
    }

    /**
     * Start this MOS6510 monitor.
     * @return Clockable::HALT if the system must be terminated; otherwise another different value.
     */
    size_t run();

    /**
     * Add a breakpoint.
     * @param addr Breakpoint address;
     * @param cond Condition that triggers the breakpoint (emtpy for unconditional breakpoints).
     */
    void add_breakpoint(addr_t addr, cond_t cond = {}) {
        _breakpoints[addr] = cond;
    }

    /**
     * Remove a breakpoint.
     * @param addr Addres to remove from the breakpoint list.
     */
    void del_breakpoint(addr_t addr) {
        _breakpoints.erase(addr);
    }

    /**
     * Detect whether there is a (conditional or unconditional) breakpoint on a specified address.
     * @param addr Address to detect;
     * @return true if there is breakpoint; false otherwise.
     */
    bool is_breakpoint(addr_t addr) const;

private:
    constexpr static const char *PROMPT_PREFIX = "";
    constexpr static const char *PROMPT_SUFFIX = "> ";

    using fn_t = bool (*)(Mos6510Monitor &, const std::vector<std::string> &args);


    struct Command {
        const std::string command;
        const std::string short_command;
        const std::string args;
        const std::string help;
        fn_t              fn;
    };


    /**
     * @return The next prompt string.
     */
    std::string prompt();

    /**
     * Convert a string formatted as "[$]xxxx" or "." to an address.
     * @param str    The string to convert;
     * @param devfal Default value (set in case the string is ".").
     * @return The address.
     * @exception InvalidNumber
     * @see to_number()
     */
    //FIXME: move to utils
    addr_t to_addr(const std::string &str, addr_t defval);

    /**
     * Convert a string formatted as "[$#]xxx" to an unsigned number.
     * @param str The string to convert.
     * @return The converted unsigned integner.
     * @exception InvalidNumber
     * @see utils::to_number()
     */
    //FIXME: move to utils
    size_t to_count(const std::string &str);

    std::istream  &_is;
    std::ostream  &_os;
    class Mos6510 &_cpu;

    size_t                   _count{};
    bool                     _is_running{};
    std::string              _prev_line{};
    fn_t                     _prev_fn{};
    std::map<addr_t, cond_t> _breakpoints{};


    static bool assemble    (Mos6510Monitor &self, const std::vector<std::string> &args);
    static bool disassemble (Mos6510Monitor &self, const std::vector<std::string> &args);
    static bool dump        (Mos6510Monitor &self, const std::vector<std::string> &args);
    static bool registers   (Mos6510Monitor &self, const std::vector<std::string> &args);

    static bool bp_add      (Mos6510Monitor &self, const std::vector<std::string> &args);
    static bool bp_del      (Mos6510Monitor &self, const std::vector<std::string> &args);
    static bool bp_clear    (Mos6510Monitor &self, const std::vector<std::string> &args);
    static bool bp_list     (Mos6510Monitor &self, const std::vector<std::string> &args);

    static bool go          (Mos6510Monitor &self, const std::vector<std::string> &args);
    static bool step        (Mos6510Monitor &self, const std::vector<std::string> &args);

    static bool load        (Mos6510Monitor &self, const std::vector<std::string> &args);
    static bool save        (Mos6510Monitor &self, const std::vector<std::string> &args);

    static bool loglevel    (Mos6510Monitor &self, const std::vector<std::string> &args);

    static bool quit        (Mos6510Monitor &self, const std::vector<std::string> &args);
    static bool help        (Mos6510Monitor &self, const std::vector<std::string> &args);

    static const std::vector<Command> commands;
};

}
