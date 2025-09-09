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
#include "readline.hpp"

#include "logger.hpp"

#include <sys/types.h>
#include <sys/uio.h>
#include <termios.h>
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <sstream>

namespace caio {
namespace readline {

History::History(const fs::Path& fname)
    : _histfname{fname}
{
    load();
}

History::~History()
{
    try {
        save();
    } catch (...) {
    }
}

History& History::operator=(History&& other)
{
    _histfname = std::move(other._histfname);
    _cursor = other._cursor;
    _current = other._current;
    _history = std::move(other._history);

    other._cursor = {};
    other._current = {};
    other._history = {};

    return *this;
}

std::string History::add_current()
{
    const auto curr = _history[_current];
    if (!curr.empty()) {
        if (_current == 0 || curr != _history[_current - 1]) {
            if (_current == HISTSIZ) {
                std::copy(_history.begin() + 1, _history.end(), _history.begin());
            } else {
                ++_current;
            }
        }

        _cursor = _current;
        _history[_current] = {};
    }

    return curr;
}

void History::back()
{
    if (_cursor > 0) {
        --_cursor;
    }

    if (_current != _cursor) {
        _history[_current] = _history[_cursor];
    }
}

void History::forward()
{
    if (_cursor < _current) {
        ++_cursor;
    }

    if (_current != _cursor) {
        _history[_current] = _history[_cursor];
    }
}

std::string History::to_string() const
{
    std::ostringstream os{};

    for (size_t p = 0; p < _current; ++p) {
        os << _history[p] << (p == _cursor ? " <\n" : "\n");
    }

    return os.str();
}

void History::load()
{
    //TODO test
    if (!_histfname.empty()) {
        std::ifstream is{_histfname, std::ios::binary | std::ios::in};
        if (!is) {
            if (errno != ENOENT) {
                throw IOError{"Can't open: {}: {}", _histfname.string(), Error::to_string()};
            }
            log.warn("Unable to load history file: {}: {}\n", _histfname.string(), Error::to_string(errno));
        }

        std::string line{};
        while (std::getline(is, line)) {
            if (line.empty()) {
                continue;
            }

            current() = line;
            add_current();
        }

        is.close();
    }
}

void History::save()
{
    //TODO test
    if (!_histfname.empty()) {
        std::ofstream os{_histfname, std::ios::binary | std::ios::out | std::ios::trunc};
        if (!os) {
            if (errno != ENOENT) {
                throw IOError{"Can't open: {}: {}", _histfname.string(), Error::to_string()};
            }
            log.warn("Unable to save history file: {}: {}\n", _histfname.string(), Error::to_string(errno));
        }

        for (size_t pos = 0; pos < _current; ++pos) {
            os << _history[pos] << "\n";
        }

        os.close();
    }
}

Readline::Readline(int ifd, int ofd, const fs::Path& histfname)
    : _ifd{-1},
      _ofd{-1},
      _history{histfname}
{
    fds({ifd, ofd});
}

Readline& Readline::operator=(Readline&& other)
{
    close();
    _ifd = other._ifd;
    _ofd = other._ofd;
    _history = std::move(other._history);

    other._ifd = -1;
    other._ofd = -1;

    return *this;
}

void Readline::fds(const std::pair<int, int>& fd)
{
    const auto setfd = [](int& dstfd, int fd, std::string_view iostr) {
        if (fd >= 0) {
            auto nfd = ::dup(fd);
            if (nfd < 0) {
                throw IOError{"Can't duplicate {} file descriptor: {}", iostr, Error::to_string(errno)};
            }
            if (dstfd >= 0) {
                ::close(dstfd);
            }
            dstfd = nfd;
        }
    };

    setfd(_ifd, fd.first, "input");
    setfd(_ofd, fd.second, "output");
    term_init();
}

void Readline::close()
{
    if (_ifd >= 0) {
        ::close(_ifd);
    }

    if (_ofd >= 0) {
        ::close(_ofd);
    }
}

void Readline::term_init()
{
    if (::isatty(_ifd)) {
        struct ::termios attr{};

        if (::tcgetattr(_ifd, &attr) < 0) {
            throw IOError{"Can't get input terminal attributes: {}", Error::to_string(errno)};
        }

        attr.c_lflag &= ~(ICANON | ECHO);
        if (::tcsetattr(_ifd, TCSANOW, &attr) < 0) {
            throw IOError{"Can't set input terminal attributes: {}", Error::to_string(errno)};
        }
    }

    /* TODO: output */
}

char Readline::getc()
{
    char ch{};

    while (::read(_ifd, &ch, 1) <= 0) {
        if (errno != EINTR) {
            throw IOError{"Can't read character: {}", Error::to_string(errno)};
        }
    }

    return ch;
}

void Readline::write(std::span<const char> data) const
{
    if (data.size() != 0) {
        size_t wr = ::write(_ofd, data.data(), data.size());
        if (wr != data.size()) {
            throw IOError{"Can't write: {}", Error::to_string(errno)};
        }
    }
}

bool Readline::process_escape()
{
    const char ch = getc();

    switch (ch) {
    case CURSOR_CONTROL:
        return process_cursor();

    default:;
    }

    return false;
}

bool Readline::process_cursor()
{
    const char ch = getc();

    switch (ch) {
    case CURSOR_UP:
        _history.back();
        return true;

    case CURSOR_DOWN:
        _history.forward();
        return true;

    case CURSOR_RIGHT:
    case CURSOR_LEFT:
        return true;

    default:;
    }

    return false;
}

void Readline::erase_chars(size_t size)
{
    while (size--) {
        write(ERASE_PREV_CHAR);
    }
}

std::string Readline::getline()
{
    for (;;) {
        std::string& line = _history.current();
        const char ch = getc();
        switch (ch) {
        case NEWLINE:
            write(ch);
            return _history.add_current();

        case BACKSPACE:
            if (!line.empty()) {
                write(ERASE_PREV_CHAR);
                line.pop_back();
            }
            break;

        case ESCAPE: {
            size_t size = line.size();
            if (process_escape()) {
                erase_chars(size);
                write(line);
            }
            break;
        }

        default:
            write(ch);
            line.push_back(ch);
        }
    }
}

}
}
