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
#include "zx80_cassette.hpp"

#include "logger.hpp"

/* State machines debug */
//#define CAIO_ZX80_CASS_DEBUG

#ifdef CAIO_ZX80_CASS_DEBUG
#define ZX80_CASS_DEBUG(fmt, args...)       do { log.debug(fmt, ## args); } while (0);
#else
#define ZX80_CASS_DEBUG(fmt, args...)
#endif

namespace caio {
namespace sinclair {
namespace zx80 {

ZX80Cassette::ZX80Cassette(const sptr_t<Clock>& clk)
    : _clk{clk}
{
    CAIO_ASSERT(_clk.get() != nullptr);
}

bool ZX80Cassette::read()
{
    uint64_t now = _clk->time();
    int64_t elapsed = now - _rx_time;

    switch (_rx_state) {
    case State::Init:
        /*
         * Read operation is started by the caller (the ROM).
         */
        receive(RxCmd::Rewind);
        _rx_state = State::Sync;
        _rx_time = now;
        ZX80_CASS_DEBUG("ZX80 Cassette: read: Init -> Sync\n");
        return 0;

    case State::Sync:
        if (elapsed < static_cast<int64_t>(SYNC_DURATION)) {
            return 0;
        }

        elapsed = 0;
        _rx_count = 0;
        _rx_bit = 0;
        _rx_state = State::Data;
        _rx_time = now;
        ZX80_CASS_DEBUG("ZX80 Cassette: read: Sync -> Data\n");

        /* PASSTHROUGH */

    case State::Data:
        if (_rx_count == 0) {
            if (elapsed < static_cast<int64_t>(BIT_SEPARATOR_TIME)) {
                return 0;
            }

            if (_rx_bit == 0) {
                /*
                 * Get a new byte to transfer.
                 */
                int rx = receive();
                if (rx == END_OF_FILE) {
                    rx = receive();
                    if (rx == END_OF_TAPE) {
                        ZX80_CASS_DEBUG("ZX80 Cassette: read: End of Tape: Data -> End\n");
                        _rx_state = State::End;
                        return 0;
                    }
                }
                _rx_byte = rx;
                _rx_bit = 128;  /* MSB first */
//                ZX80_CASS_DEBUG("ZX80 Cassette: read: Received byte: ${:02X}\n", _rx_byte);
            }

            /*
             * Bit to transfer.
             */
            bool bit = (_rx_byte & _rx_bit);
            _rx_count = (bit ? BIT_1_PULSE_COUNT : BIT_0_PULSE_COUNT);
            _rx_bit >>= 1;
            _rx_time = now;
            elapsed = 0;
        }

        /*
         * Bit transfer.
         */
        if (elapsed < static_cast<int64_t>(DATA_PULSE_TIME)) {
            return 1;
        }

        if (elapsed < 2 * static_cast<int64_t>(DATA_PULSE_TIME)) {
            return 0;
        }

        _rx_time = now;
        --_rx_count;

        if (_rx_count == 0 && _rx_bit == 0) {
            switch (receive(RxCmd::Peek)) {
            case END_OF_FILE:
                ZX80_CASS_DEBUG("ZX80 Cassette: read: End of File: Data -> Sync\n");
                _rx_state = State::Sync;
                break;
            case END_OF_TAPE:
                ZX80_CASS_DEBUG("ZX80 Cassette: read: End of Tape: Data -> End\n");
                _rx_state = State::End;
                break;
            default:;
            }
        }

        return 0;

    case State::End:
        break;
    }

    return 0;
}

void ZX80Cassette::write(bool pulse)
{
    uint64_t now = _clk->time();
    uint64_t elapsed = now - _tx_time;
    _tx_time = now;

    switch (_tx_state) {
    case State::Init:
        if (!pulse && elapsed > SYNC_PULSE_TIME_MIN && elapsed < SYNC_PULSE_TIME_MAX) {
            /*
             * Sync pulse received.
             */
            ZX80_CASS_DEBUG("ZX80Cassette: write: Init -> Sync\n");
            _tx_state = State::Sync;
        }
        break;

    case State::Sync:
        if (elapsed < SYNC_PULSE_TIME_MIN) {
            /*
             * Protocol violation (probably abort from basic).
             */
            ZX80_CASS_DEBUG("ZX80Cassette: write: Break: Sync -> End\n");
            _tx_state = State::End;
            break;
        }

        if (pulse) {
            /*
             * Sync period terminated.
             */
            _tx_bit = 128;   /* MSB first */
            _tx_count = 0;
            _tx_byte = 0;
            _tx_pulse = 1;
            _tx_state = State::Data;
            ZX80_CASS_DEBUG("ZX80Cassette: write: Sync -> Data\n");
        }
        break;

    case State::Data:
        if (pulse == _tx_pulse) {
            /*
             * No more data to send.
             * (We really should calculate the timing between two consecutive pulses).
             */
            if (_tx_bit == 1) {
                /* Last bit */
                _tx_byte |= (_tx_count == BIT_0_PULSE_COUNT ? 0 : 1);
                transmit(_tx_byte);
            }

            ZX80_CASS_DEBUG("ZX80Cassette: write: End of File: Data -> End\n");
            transmit(END_OF_FILE);
            _tx_state = State::End;
            break;
        }

        _tx_pulse = pulse;

        if (elapsed < BIT_SEPARATOR_TIME) {
            /*
             * Bit encoding ongoing.
             */
            if (!pulse) {
                ++_tx_count;
            }
            break;
        }

        /*
         * Bit encoded.
         */
        if (_tx_count != BIT_0_PULSE_COUNT && _tx_count != BIT_1_PULSE_COUNT) {
            /*
             * Protocol violation (probably abort from basic).
             */
            ZX80_CASS_DEBUG("ZX80Cassette: write: Break: Data -> End\n");
            transmit(END_OF_FILE);
            _tx_state = State::End;
            break;
        }

        _tx_byte |= (_tx_count == BIT_0_PULSE_COUNT ? 0 : _tx_bit);
        _tx_count = 0;
        _tx_bit >>= 1;
        if (_tx_bit != 0) {
            /*
             * Byte encoding ongoing.
             */
            break;
        }

        /*
         * Byte encoded, transmit it.
         */
        ZX80_CASS_DEBUG("ZX80Cassette: write: Transmitting byte: ${:02X}\n", _tx_byte);
        transmit(_tx_byte);
        _tx_bit = 128;
        _tx_byte = 0;
        break;

    case State::End:;
    }
}

void ZX80Cassette::restart()
{
//    ZX80_CASS_DEBUG("ZX80Cassette: restart\n");
    _tx_state = _rx_state = State::Init;
}

bool ZX80Cassette::is_idle() const
{
    return ((_rx_state == State::Init || _rx_state == State::End) &&
        (_tx_state == State::Init || _tx_state == State::End));
}

ZX80CassetteO::ZX80CassetteO(const sptr_t<Clock>& clk, const fs::Path& cassdir)
    : ZX80Cassette{clk},
      _cassdir{fs::fix_home(cassdir)}
{
    if (!fs::is_directory(_cassdir)) {
        throw IOError{"Invalid cassette directory: {}", cassdir.string()};
    }
}

void ZX80CassetteO::restart()
{
    ZX80Cassette::restart();
    _buf.clear();
    _it = _buf.end();
}

inline fs::Path ZX80CassetteO::fname() const
{
    return (_cassdir.string() + "/" + CASSETTE_FNAME + CASSETTE_EXT);
}

void ZX80CassetteO::transmit(int data)
{
    if (data == END_OF_FILE) {
        const auto& fullpath = fname();
        log.debug("ZX80CassetteO: Saving file: {}\n", fullpath.string());
        fs::save(fullpath, _buf);
        _buf.clear();
    } else {
        _buf.push_back(data);
    }
}

int ZX80CassetteO::receive(RxCmd cmd)
{
    if (cmd == RxCmd::Rewind) {
        const auto& fullpath = fname();
        log.debug("ZX80CassetteO: Loading file: {}\n", fullpath.string());
        try {
            _buf = fs::load(fullpath);
        } catch (const std::exception&) {
            _buf = {};
        }
        _it = _buf.begin();
        return 0;
    }

    int data = END_OF_TAPE;

    if (_it != _buf.end()) {
        data = *_it;
        _it += (cmd != RxCmd::Peek);
    }

    return data;
}

ZX80CassetteP::ZX80CassetteP(const sptr_t<Clock>& clk, const fs::Path& cassdir)
    : ZX80CassetteO{clk, cassdir}
{
}

char ZX80CassetteP::zx81_to_ascii(uint8_t ch)
{
    constexpr static uint8_t MASK = 0x3F;
    static char ascii[64] = {
        ' ',    '\xff', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff',     /* 0x00 */
        '\xff', '\xff', '\xff', '\"',   '\xff', '$',    ':',    '?',        /* 0x08 */
        '(',    ')',    '>',    '<',    '=',    '+',    '-',    '*',        /* 0x10 */
        '/',    ';',    ',',    '.',    '0',    '1',    '2',    '3',        /* 0x18 */
        '4',    '5',    '6',    '7',    '8',    '9',    'a',    'b',        /* 0x20 */
        'c',    'd',    'e',    'f',    'g',    'h',    'i',    'j',        /* 0x28 */
        'k',    'l',    'm',    'n',    'o',    'p',    'q',    'r',        /* 0x30 */
        's',    't',    'u',    'v',    'w',    'x',    'y',    'z'         /* 0x38 */
    };

    return ascii[ch & MASK];
}

uint8_t ZX80CassetteP::ascii_to_zx81(char ch)
{
    constexpr static uint8_t MASK = 0x7F;
    static uint8_t zx81[128] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,     /* 0x00 */
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,     /* 0x08 */
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,     /* 0x10 */
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,     /* 0x18 */
        0x00, 0xff, 0x0b, 0xff, 0x0d, 0xff, 0xff, 0xff,     /* 0x20 */
        0x10, 0x11, 0x17, 0x15, 0x1a, 0x16, 0x1c, 0x18,     /* 0x28 */
        0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,     /* 0x30 */
        0x24, 0x25, 0x06, 0x19, 0x13, 0x14, 0x12, 0x0f,     /* 0x38 */
        0xff, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c,     /* 0x40 */
        0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34,     /* 0x48 */
        0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c,     /* 0x50 */
        0x3d, 0x3e, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff,     /* 0x58 */
        0xff, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c,     /* 0x60 */
        0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34,     /* 0x68 */
        0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c,     /* 0x70 */
        0x3d, 0x3e, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff      /* 0x78 */
    };

    return zx81[ch & MASK];
}

inline fs::Path ZX80CassetteP::fname(const fs::Path& basename) const
{
    return (_cassdir.string() + "/" + basename.string() + CASSETTE_EXT);
}

inline std::string ZX80CassetteP::extract_name()
{
    constexpr static size_t MAX_FILENAME_SIZE = 128;
    std::string name{};

    for (size_t i = 0; i < _buf.size() && i < MAX_FILENAME_SIZE; ++i) {
        uint8_t zch = _buf[i];
        char ch = zx81_to_ascii(zch);
        name.push_back(ch);
        if (zch & 0x80) {
            /* Filename ends at character with bit 7 set */
            break;
        }
    }

    if (name.find('\xff') == std::string::npos) {
        /*
         * .p files do not encode the name.
         */
        _buf.erase(_buf.begin(), _buf.begin() + name.size());
    } else {
        /*
         * Name not provided.
         */
        name = CASSETTE_FNAME;
    }

    return name;
}

void ZX80CassetteP::transmit(int data)
{
    if (data < 0) {
        auto name = extract_name();
        auto fullpath = fname(name);
        log.debug("ZX80CassetteP: Saving file: {}\n", fullpath.string());
        fs::save(fullpath, _buf);
        _buf.clear();
    } else {
        _buf.push_back(data);
    }
}

int ZX80CassetteP::receive(RxCmd cmd)
{
    if (cmd == RxCmd::Rewind) {
        log.debug("ZX80CassetteP: Loading filenames from directory: {}\n", _cassdir.string());

        _entries = fs::directory(_cassdir, CASSETTE_PATTERN, fs::MATCH_CASE_INSENSITIVE);
        for (const auto& entry : _entries) {
            log.debug("ZX80CassetteP: Found: {}\n", entry.first.string());
        }

        _dirit = _entries.begin();
        _buf.clear();
        _it = _buf.end();
        return 0;
    }

    int data = END_OF_FILE;

    if (_it == _buf.end()) {
        if (_dirit == _entries.end()) {
            return END_OF_TAPE;
        }

        const auto& entry = *_dirit++;
        const auto& fullpath = entry.first;
        uint64_t filesize = entry.second;

        log.debug("ZX80CassetteP: Loading file: {}, size: {}\n", fullpath.string(), filesize);

        _buf = fs::load(fullpath);

        /*
         * Insert the filename converted to zx81 character format.
         */
        std::string name{fs::basename(fullpath)};
        constexpr size_t extsiz = std::size(CASSETTE_EXT) - 1;
        name.erase(name.size() - extsiz, extsiz);   /* Remove extension (.p) */

        for (auto nit = name.rbegin(); nit != name.rend(); ++nit) {
            uint8_t ch = ascii_to_zx81(*nit);
            _buf.insert(_buf.begin(), ch);
        }

        _buf[name.size() - 1] |= 0x80;
        _it = _buf.begin();

    } else {
        data = *_it;
        if (cmd != RxCmd::Peek) {
            ++_it;
        }
    }

    return data;
}

}
}
}
