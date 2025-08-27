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
#include "zxsp_tape.hpp"

#include "endian.hpp"
#include "logger.hpp"

//#define CAIO_ZXSPECTRUM_TAPE_DEBUG
#undef CAIO_ZXSPECTRUM_TAPE_DEBUG

#ifdef CAIO_ZXSPECTRUM_TAPE_DEBUG
#define ZXSPECTRUM_TAPE_DEBUG(fmt, args...)         do { log.debug(fmt, ## args); } while (0);
#else
#define ZXSPECTRUM_TAPE_DEBUG(fmt, args...)
#endif

namespace caio {
namespace sinclair {
namespace zxspectrum {
namespace tape {

void PulseBuffer::push_silence(uint64_t time)
{
    _start += time;
    push_back({0, _start});
}

void PulseBuffer::push_pilot(size_t count)
{
    while (count--) {
        _start += PILOT_PULSE_TIME;
        push_back({1, _start});
        _start += PILOT_PULSE_TIME;
        push_back({0, _start});
    }
}

void PulseBuffer::push_sync()
{
    _start += SYNC_HI_PULSE_TIME;
    push_back({1, _start});
    _start += SYNC_LO_PULSE_TIME;
    push_back({0, _start});
}

void PulseBuffer::push_data(std::span<const uint8_t> buf)
{
    for (auto byte : buf) {
        for (uint8_t bit = 128; bit; bit >>= 1) {
            if (byte & bit) {
                _start += BIT_1_PULSE_TIME;
                push_back({1, _start});
                _start += BIT_1_PULSE_TIME;
                push_back({0, _start});
            } else {
                _start += BIT_0_PULSE_TIME;
                push_back({1, _start});
                _start += BIT_0_PULSE_TIME;
                push_back({0, _start});
            }
        }
    }
    uint64_t last = ((*(buf.end() - 1) & 1) ? BIT_1_PULSE_TIME : BIT_0_PULSE_TIME);
    _start += last;
    push_back({1, _start});
}

void PulseBuffer::push_block(TAPFile::Block block)
{
    size_t pilot_count = (block[0] == HeaderBlock::BLOCKTYPE_HEADER ?
        PILOT_PULSE_COUNT_HEADER : PILOT_PULSE_COUNT_DATA);

    push_silence(SILENCE_TIME);
    push_pilot(pilot_count);
    push_sync();
    push_data(block);
}

Tape::Tape(const sptr_t<Clock>& clk, std::string_view itape, std::string_view otape, bool fastload)
    : _clk{clk},
      _fastload{fastload}
{
    CAIO_ASSERT(_clk.get() != nullptr);

    load(itape);
    save(otape);
}

void Tape::save(std::string_view path)
{
    TX tx{};

    tx.path = fs::fix_home(path);
    if (tx.path.empty()) {
        tx.path = ".";
        tx.isdir = true;
    } else {
        tx.isdir = fs::is_directory(tx.path);
    }

    _tx = tx;
    log.debug("Tape: Output tape: \"{}\", is_directory: {}\n", _tx.path, _tx.isdir);
}

void Tape::load(std::string_view path)
{
    std::string spath{path};    /* _rx is destroyed but path could be a reference to _rx's path */
    _rx = {};
    _rx.path = spath;
    _rx.tap.load(spath);
}

inline void Tape::fastload(bool on)
{
    if (_fastload) {
        _clk->fullspeed(on);
    }
}

void Tape::play()
{
    log.debug("Tape: Input tape play: \"{}\"\n", _rx.path);
    _rx.stopped = false;
    fastload(true);
}

void Tape::stop()
{
    log.debug("Tape: Input tape stop: \"{}\". Last block invalidated\n", _rx.path);
    _rx.stopped = true;
    _rx.state = RXState::Init;
    fastload(false);
}

void Tape::eject()
{
    log.debug("Tape: Input tape eject: \"{}\"\n", _rx.path);
    _rx = {};
}

void Tape::rewind()
{
    log.debug("Tape: Input tape rewind: \"{}\"\n", _rx.path);
    load(_rx.path);
}

bool Tape::is_idle() const
{
    return (_rx.stopped && _tx.state == TXState::Init);
}

bool Tape::read()
{
    uint64_t now = _clk->time();

    switch (_rx.state) {
    case RXState::Init:
        if (_rx.stopped) {
            break;
        }

        _rx.state = RXState::Pulse;
        ZXSPECTRUM_TAPE_DEBUG("Tape: read: Init -> Pulse\n");

        /* PASSTHROUGH */

    case RXState::Pulse:
        /*
         * Ask for next pulse stream.
         */
        if (!receive()) {
            stop();
            ZXSPECTRUM_TAPE_DEBUG("Tape: read: Tape ended: Pulse -> Init\n");
            break;
        }

        _rx.state = RXState::Pulsing;
        _rx.pulse = *_rx.pulseit++;
        ZXSPECTRUM_TAPE_DEBUG("Tape: read: New block: Pulse -> Pulsing\n");

        /* PASSTHROUGH */

    case RXState::Pulsing:
        while (now >= std::get<uint64_t>(_rx.pulse)) {
            if (_rx.pulseit == _rx.pulsebuf.end()) {
                _rx.state = RXState::Pulse;
                ZXSPECTRUM_TAPE_DEBUG("Tape: read: Pulsing -> Pulse\n");
                break;
            } else {
                _rx.pulse = *_rx.pulseit++;
            }
        }
        return std::get<bool>(_rx.pulse);
    }

    return 0;
}

void Tape::write(bool pulse)
{
    if (decode_pulse(pulse)) {
        switch (_tx.state) {
        case TXState::Init:
            ZXSPECTRUM_TAPE_DEBUG("Tape: write: Init\n");
            _tx.buffer.clear();
            break;

        case TXState::Pilot:
            ZXSPECTRUM_TAPE_DEBUG("Tape: write: Pilot\n");
            break;

        case TXState::Sync:
            ZXSPECTRUM_TAPE_DEBUG("Tape: write: Sync\n");
            _tx.byte = 0;
            _tx.bit = 128;
            break;

        case TXState::Bit_1:
            _tx.byte |= _tx.bit;

            /* PASSTHROUGH */

        case TXState::Bit_0:
            _tx.bit >>= 1;
            if (_tx.bit == 0) {
                ZXSPECTRUM_TAPE_DEBUG("Tape: write: Transmitting byte: ${:02X}\n", _tx.byte);
                if (!transmit()) {
                    _tx.state = TXState::Init;
                    ZXSPECTRUM_TAPE_DEBUG("Tape: write: Transmission error: Bit_X -> Init\n");
                } else {
                    _tx.byte = 0;
                    _tx.bit = 128;
                }
            }
            break;
        }
    }
}

bool Tape::decode_pulse(bool pulse)
{
    uint64_t now = _clk->time();
    uint64_t elapsed = now - _tx.time;
    _tx.time = now;

    switch (_tx.pulse_st) {
    case TXPulseState::Init:
        if (pulse) {
            _tx.pulse_st = TXPulseState::Level_Hi;
        }
        return false;

    case TXPulseState::Level_Hi:
        if (pulse) {
            _tx.pulse_st = TXPulseState::Init;
        } else {
            _tx.pulse_hi = elapsed;
            _tx.pulse_st = TXPulseState::Level_Lo;
        }
        return false;

    case TXPulseState::Level_Lo:
        if (!pulse) {
            _tx.pulse_st = TXPulseState::Init;
            return false;
        }

        _tx.pulse_lo = elapsed;
        _tx.pulse_st = TXPulseState::Level_Hi;
    }

    TXState newst{TXState::Init};

    if ((_tx.pulse_lo >= PILOT_PULSE_TIME_MIN && _tx.pulse_lo <= PILOT_PULSE_TIME_MAX) &&
        (_tx.pulse_hi >= PILOT_PULSE_TIME_MIN && _tx.pulse_hi <= PILOT_PULSE_TIME_MAX)) {
        newst = TXState::Pilot;

    } else if ((_tx.pulse_lo >= SYNC_LO_PULSE_TIME_MIN && _tx.pulse_lo <= SYNC_LO_PULSE_TIME_MAX) &&
        (_tx.pulse_hi >= SYNC_HI_PULSE_TIME_MIN && _tx.pulse_hi <= SYNC_HI_PULSE_TIME_MAX)) {
        newst = TXState::Sync;

    } else if ((_tx.pulse_lo >= BIT_0_PULSE_TIME_MIN && _tx.pulse_lo <= BIT_0_PULSE_TIME_MAX) &&
        (_tx.pulse_hi >= BIT_0_PULSE_TIME_MIN && _tx.pulse_hi <= BIT_0_PULSE_TIME_MAX)) {
        newst = TXState::Bit_0;

    } else if ((_tx.pulse_lo >= BIT_1_PULSE_TIME_MIN && _tx.pulse_lo <= BIT_1_PULSE_TIME_MAX) &&
        (_tx.pulse_hi >= BIT_1_PULSE_TIME_MIN && _tx.pulse_hi <= BIT_1_PULSE_TIME_MAX)) {
        newst = TXState::Bit_1;
    }

#if 0
    ZXSPECTRUM_TAPE_DEBUG("Tape: decode_pulse: lo={}, hi={}\n", _tx.pulse_lo, _tx.pulse_hi);

    if (newst != _tx.state) {
        ZXSPECTRUM_TAPE_DEBUG("Tape: decode_pulse: {} -> {}\n",
            static_cast<int>(_tx.state), static_cast<int>(newst));
    }
#endif

    _tx.state = newst;
    return true;
}

inline std::string Tape::otape_fullpath(std::string_view basename) const
{
    return (!_tx.isdir ? _tx.path : _tx.path + "/" + std::string{basename} + TAPFile::FILE_EXTENSION);
}

bool Tape::transmit()
{
    _tx.buffer.push_back(_tx.byte);

    const auto* hdr = reinterpret_cast<const HeaderBlock*>(_tx.buffer.data());
    size_t size = _tx.buffer.size();

    if (size < HEADER_BLOCK_SIZE) {
        return true;
    }

    if (hdr->block_type != HeaderBlock::BLOCKTYPE_HEADER) {
        log.error("Tape: Invalid header: ${:02X}. Operation aborted.\n", hdr->block_type);
        return false;
    }

    /*
     * HeaderBlock::datalen is the size of the raw data,
     * the size of the data block is the size of the raw data
     * plus 2 bytes (block_type and chksum).
     */
    size_t datalen = le16toh(hdr->datalen) + DATA_BLOCK_SIZE;

    if (size < HEADER_BLOCK_SIZE + datalen) {
        return true;
    }

    const std::string name{hdr->name, sizeof(hdr->name)};
    const auto fullpath = otape_fullpath(name);
    const uint8_t* data = _tx.buffer.data() + HEADER_BLOCK_SIZE;
    try {
        log.debug("Tape: Saving file: \"{}\"\n", fullpath);
        TAPFile::save(fullpath, {_tx.buffer.data(), HEADER_BLOCK_SIZE}, {data, datalen});
    } catch (const std::exception& err) {
        log.error("Tape: {}\n", err.what());
    }

    _tx.buffer.clear();
    return true;
}

bool Tape::receive()
{
    _rx.pulsebuf.clear();

    const auto& block = _rx.tap.next_block();
    if (block.size() == 0) {
        log.debug("Tape: No more blocks\n");
        return false;
    }

    _rx.pulsebuf.start_time(_clk->time());
    _rx.pulsebuf.push_block(block);
    _rx.pulseit = _rx.pulsebuf.begin();

    log.debug("Tape: New block converted to pulses: block size: {}, pulse buffer size: {}\n",
        block.size(), _rx.pulsebuf.size());

    return true;
}

}
}
}
}
