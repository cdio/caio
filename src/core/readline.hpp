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
#pragma once

#include <array>
#include <format>
#include <span>
#include <string>

#include "fs.hpp"
#include "types.hpp"

namespace caio {
namespace readline {

/**
 * Readline History.
 */
class History {
public:
    constexpr static const size_t HISTSIZ = 100;

    /**
     * Iitialise this instance and optionally fill it with history data from a file.
     * @param fname The name of the history file to read.
     * @see load()
     * @exception IOError if an error occurs while trying to load the specified file.
     */
    History(const fs::Path& fname = {});

    /**
     * Release this instance.
     * If a file name was specified, the history is saved before this instance is destroyed.
     * @see save()
     */
    virtual ~History();

    History& operator=(History&& other);

    /**
     * Get a reference to the current string.
     * The current string is the string being edited by the user.
     * @return A reference to the current string.
     */
    std::string& current() {
        return _history[_current];
    }

    /**
     * Add the current string to the history.
     * The current string is the string being edited by the user
     * when this string is added it becomes part of the history.
     * @return The added string.
     */
    std::string add_current();

    /**
     * Set the previous history string as current.
     */
    void back();

    /**
     * Set the next history string as current.
     */
    void forward();

    /**
     * Return a string containing all the history values (each entry on its own line).
     * @return The entire history as a string.
     */
    std::string to_string() const;

private:
    /**
     * Load the history file.
     * @exception IOError
     * @see _histfname
     */
    void load();

    /**
     * Load the history file.
     * @exception IOError
     * @see _histfname
     */
    void save();

    fs::Path                         _histfname;
    size_t                           _cursor{};
    size_t                           _current{};
    std::array<std::string, HISTSIZ> _history{};
};

/**
 * Readline
 */
class Readline {
public:
    constexpr static const char ESCAPE           = '\x1B';
    constexpr static const char BACKSPACE        = '\x7F';
    constexpr static const char NEWLINE          = '\n';
    constexpr static const char CURSOR_CONTROL   = '[';
    constexpr static const char CURSOR_UP        = 'A';
    constexpr static const char CURSOR_DOWN      = 'B';
    constexpr static const char CURSOR_RIGHT     = 'C';
    constexpr static const char CURSOR_LEFT      = 'D';

    constexpr static const char* ERASE_LINE      = "\x1B[1K\r";
    constexpr static const char* ERASE_PREV_CHAR = "\x08 \x08";

    /**
     * Initialise this instance.
     * @param ifd User input file descriptor;
     * @param ofd User output file descriptor (console);
     * @param histfname History file name (or empty).
     * @exception IOError
     * @note The input/output file descriptors are duplicated
     * so they can be closed after this method returns.
     */
    //TODO: replace with iostream and native_handle() (C++26)
    Readline(int ifd = -1, int ofd = -1, const fs::Path& histfname = {});

    virtual ~Readline() {
        close();
    }

    Readline& operator=(Readline&& other);

    /**
     * Retrieve an input line from the user.
     * @return An input line from the user.
     */
    std::string getline();

    /**
     * Retrieve the complete history as a string.
     * @return The entire history as a string.
     * @see History::to_string()
     */
    std::string history() const {
        return _history.to_string();
    }

    /**
     * Send a character to the user.
     * @param ch Character to send.
     * @exception IOError
     */
    void write(char ch) const {
        write(std::span{&ch, 1});
    }

    /**
     * Send a message to the user.
     * @param msg Message to send.
     * @exception IOError
     */
    void write(const std::string& msg) const {
        write(std::span{msg.c_str(), msg.size()});
    }

    /**
     * Send a message to the user.
     * @param msg Message to send.
     * @exception IOError
     */
    void write(const char* msg) const {
        write(std::string{msg});
    }

    /**
     * Send a message to the user.
     * @param fmt  Format string;
     * @param args Arguments.
     * @exception IOError
     */
    template<typename... Args>
    void write(std::format_string<Args...> fmt, Args&&... args) const {
        write(std::vformat(fmt.get(), std::make_format_args(args...)));
    }

    /**
     * Send a character buffer to the user.
     * @param data Buffer to send.
     * @exception IOError
     */
    void write(std::span<const char> data) const;

    /**
     * Get input and output file descriptors.
     * @return The input and output file descriptors respectively.
     */
    std::pair<int, int> fds() const {
        return {_ifd, _ofd};
    }

    /**
     * Reset input and output file descriptors.
     * If only one of them must be changed specify the other as -1.
     * @param fd Pair with the new file descriptors (input and output).
     */
    void fds(const std::pair<int, int>& fd);

    /**
     * Read a character from the user.
     * This method blocks until the user enters a character
     * and it does not affect the history.
     * @return The user input.
     */
    char getc();

private:
    /**
     * Properly configure the input/output file descriptors.
     */
    void term_init();

    /**
     * Close the (duplicated) input/output file descriptors.
     * @see Readline(int ifd, int, const fs::Path&)
     */
    void close();

    /**
     * Process ANSI escape sequences.
     */
    bool process_escape();

    /**
     * Process arrow keys (history navigation).
     */
    bool process_cursor();

    /**
     * Remove characters entered by the user.
     * @param size The number of characters to remove.
     */
    void erase_chars(size_t size);

    int     _ifd{-1};
    int     _ofd{-1};
    History _history{};
};

}

using Readline = readline::Readline;

}
