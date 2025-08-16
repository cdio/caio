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

#include <atomic>
#include <functional>
#include <string_view>
#include <tuple>
#include <vector>

#include "clock.hpp"
#include "fs.hpp"
#include "tapfile.hpp"
#include "types.hpp"

namespace caio {
namespace sinclair {
namespace zxspectrum {
namespace tape {

constexpr static const uint64_t SILENCE_TIME            = 1'000'000;
constexpr static const uint64_t PILOT_PULSE_TIME        = 619;
constexpr static const uint64_t PILOT_PULSE_TIME_MIN    = PILOT_PULSE_TIME - 20;
constexpr static const uint64_t PILOT_PULSE_TIME_MAX    = PILOT_PULSE_TIME + 20;
constexpr static const uint64_t SYNC_LO_PULSE_TIME      = 210;
constexpr static const uint64_t SYNC_LO_PULSE_TIME_MIN  = SYNC_LO_PULSE_TIME - 10;
constexpr static const uint64_t SYNC_LO_PULSE_TIME_MAX  = SYNC_LO_PULSE_TIME + 10;
constexpr static const uint64_t SYNC_HI_PULSE_TIME      = 190;
constexpr static const uint64_t SYNC_HI_PULSE_TIME_MIN  = SYNC_HI_PULSE_TIME - 10;
constexpr static const uint64_t SYNC_HI_PULSE_TIME_MAX  = SYNC_HI_PULSE_TIME + 10;
constexpr static const uint64_t BIT_0_PULSE_TIME        = 244;
constexpr static const uint64_t BIT_0_PULSE_TIME_MIN    = BIT_0_PULSE_TIME - 20;
constexpr static const uint64_t BIT_0_PULSE_TIME_MAX    = BIT_0_PULSE_TIME + 20;
constexpr static const uint64_t BIT_1_PULSE_TIME        = 488;
constexpr static const uint64_t BIT_1_PULSE_TIME_MIN    = BIT_1_PULSE_TIME - 20;
constexpr static const uint64_t BIT_1_PULSE_TIME_MAX    = BIT_1_PULSE_TIME + 20;
constexpr static const size_t PILOT_PULSE_COUNT_HEADER  = 4032;
constexpr static const size_t PILOT_PULSE_COUNT_DATA    = 1612;

/**
 * Pulse type.
 * A pulse type contains two values:
 * - Boolean value indicating the pulse state (0 or 1).
 * - Time value indicating how much time (in microseconds) the state must be maintained.
 */
using pulse_t = std::tuple<bool, uint64_t>;

/**
 * Pulse buffer.
 * Pulse buffer is a vector of pulses and provides
 * methods to convert from bytes to pulses.
 */
class PulseBuffer : public std::vector<pulse_t> {
public:
    PulseBuffer(uint64_t start = 0)
        : _start{start} {
    }

    virtual ~PulseBuffer() {
    }

    void start_time(uint64_t start) {
        _start = start;
    }

    void push_silence(uint64_t time);
    void push_pilot(size_t count);
    void push_sync();
    void push_data(std::span<const uint8_t> buf);
    void push_block(TAPFile::Block block);

private:
    uint64_t _start{};
};

/**
 * Standard Header block.
 */
struct HeaderBlock {
    constexpr static const uint8_t BLOCKTYPE_HEADER     = 0x00;
    constexpr static const uint8_t BLOCKTYPE_DATA       = 0xFF;
    constexpr static const uint8_t TYPE_BASIC_PRG       = 0x00;
    constexpr static const uint8_t TYPE_BASIC_NUMARR    = 0x01;
    constexpr static const uint8_t TYPE_BASIC_CHRARR    = 0x02;
    constexpr static const uint8_t TYPE_BINARY_CODE     = 0x03;

    uint8_t block_type;

    struct {
        uint8_t  type;
        char     name[10];
        uint16_t datalen;

        union {
            uint32_t params;
            struct {                    /* Type BASIC_PRG */
                uint16_t line;
                uint16_t prog;
            };
            struct {                    /* Type BINARY_CODE */
                uint16_t addr;
                uint16_t unused;
            };
        };
    } __attribute__((packed));

    uint8_t parity;

} __attribute__((packed));

constexpr static const uint16_t HEADER_BLOCK_SIZE = sizeof(HeaderBlock);
constexpr static const uint16_t DATA_BLOCK_SIZE   = 2;

/**
 * ZX-Spectrum Tape interface.
 * The tape interface process tape pulses.
 * - SAVE operation: A stream of pulses is received and converted to
 *   a stream of bytes; when the transfer is ended a TAP file is
 *   created in the host file system.
 * - LOAD operation: The content of a single TAP file or several TAP
 *   files found inside a directory in the host filesystem are
 *   concatenated and a stream of pulses is generated.
 *
 * @see https://problemkaputt.de/zxdocs.htm#spectrumcassette
 * @see https://sinclair.wiki.zxnet.co.uk/wiki/Spectrum_tape_interface
 */
class Tape {
public:
    /**
     * Initialise this tape interface.
     * @param clk      System clock;
     * @param itape    Input tape file (TAP) or directory;
     * @param otape    Output tape file (TAP) or directory;
     * @param fastload true to deactivate emulation of cassette speed; false otherwise.
     * @exception IOError
     * @see load(std::string_view)
     * @see save(std::string_view)
     */
    Tape(const sptr_t<Clock>& clk, std::string_view itape, std::string_view otape, bool fastload);

    virtual ~Tape();

    /**
     * Set the output tape.
     * This method affects SAVE operations, it does not affect LOAD operations
     * unless the specified path is the same for both.
     * If the specified path exists and it is a regular file it is assumed to
     * be a TAP file and any SAVE operation appends data to it.
     * If the specified path is a directory, a new TAP file is created inside
     * it whenever a SAVE operation is executed, in this case the name of the
     * new file is extracted from the standard header.
     * @param path Tape file or directory name.
     * @exception IOError
     * @see load(std::string_view)
     * @see HeaderBlock
     */
    void save(std::string_view path);

    /**
     * Set the input tape and rewind it.
     * Any previous input tape is ejected and the new tape is inserted.
     * This method affects LOAD operations, it does not affect SAVE operations
     * unless the specified path is the same for both.
     * If the specified path is empty, the current tape is ejected.
     * If the specified path is a file name, it is considered to be a TAP file.
     * If the specified path is a directory, a number of its TAP files are
     * concatenated into a single big TAP (see TAPFile::load(std::string_view))
     * In the last two cases a rewind is done afterwards.
     * @param path Tape file or directory name.
     * @exception IOError
     * @see TAPFile::load(std::string_view)
     */
    void load(std::string_view path);

    /**
     * Start playing the input tape.
     * This method affects LOAD operations, it does not affect SAVE operations.
     * @see load(std::string_view)
     * @see stop()
     */
    void play();

    /**
     * Stop playing the input tape.
     * This method affects LOAD operations, it does not affect SAVE operations.
     * @see play()
     */
    void stop();

    /**
     * Eject the input tape.
     * This method affects LOAD operations, it does not affect SAVE operations.
     * @see load(std::string_view)
     */
    void eject();

    /**
     * Rewind the input tape.
     * This method affects LOAD operations, it does not affect SAVE operations.
     * @see load(std::string_view)
     */
    void rewind();

    /**
     * Get the tape activity status.
     * @return true if the tape is idle (stopped); false if the tape is running.
     */
    bool is_idle() const;

    /**
     * Read a pulse from the input tape.
     * @return The pulse value.
     * @see receive()
     * @see RX
     */
    bool read();

    /**
     * Write a pulse to the output tape.
     * A series of successive pulses are decoded and when the last
     * pulse is detected a TAP file is generated in the output tape.
     * @param pulse Pulse to transmit.
     * @see decode_pulse(bool)
     * @see save(std::string_view)
     * @see transmit()
     * @see TX
     */
    void write(bool pulse);

private:
    /**
     * RX state (data from tape to CPU).
     */
    enum class RXState {
        Init,
        Pulse,
        Pulsing
    };

    /**
     * TX state (data from CPU to tape).
     */
    enum class TXState {
        Init,
        Pilot,
        Sync,
        Bit_0,
        Bit_1
    };

    /**
     * TX pulse decoding state.
     * Used to convert pulses to bytes.
     */
    enum class TXPulseState {
        Init,
        Level_Lo,
        Level_Hi
    };

    /**
     * Transmission parameters.
     */
    struct TX {
        std::string           path{};                       /* Output TAP file or directory     */
        bool                  isdir{};                      /* Output is file or directory      */
        Buffer                buffer{};                     /* Transmission buffer              */

        TXState               state{TXState::Init};         /* FSM state                        */
        TXPulseState          pulse_st{TXPulseState::Init}; /* FSM pulse decoding state         */
        uint64_t              time{};                       /* FSM current time                 */
        uint64_t              pulse_lo{};                   /* FSM duraton of a low pulse       */
        uint64_t              pulse_hi{};                   /* FSM duration of a high pulse     */
        uint8_t               bit{128};                     /* FSM bit being decoded            */
        uint8_t               byte{};                       /* FSM byte being decoded           */
    };

    /**
     * Reception parameters.
     */
    struct RX {
        std::string           path{};                       /* Input TAP file or directory      */
        bool                  stopped{true};                /* Input tape stopped or playing    */
        TAPFile               tap{};                        /* Input TAP file                   */

        RXState               state{RXState::Init};         /* FSM state                        */
        pulse_t               pulse{};                      /* FSM current pulse being received */
        PulseBuffer           pulsebuf{};                   /* TAP block converted to pulses    */
        PulseBuffer::iterator pulseit{};                    /* Pulse buffer interator           */
    };

    /**
     * Decode a series of pulses (received from the CPU).
     * When a series of pulses is decoded the state of the transmit machine is changed.
     * @param pulse Pulse value.
     * @return true if the pulse is decoded; false otherwise.
     * @see TX
     * @see TXPulseState
     */
    bool decode_pulse(bool pulse);

    /**
     * Get the full pathname for an output file.
     * @param basename File name (extracted from the header).
     * @return The full pathname.
     * @see HeaderBlock
     */
    std::string otape_fullpath(std::string_view basename) const;

    /**
     * Send a byte to the output tape.
     * The specified byte is stored inside a transmit buffer,
     * when the last byte is received a TAP file is generated
     * and saved in the output tape directory or appended to
     * the output tape file.
     * @return true on success; false otherwise.
     * @see otape_fullpath(std::string_view)
     * @see save(std::string_view)
     * @see TX
     */
    bool transmit();

    /**
     * Receive data from the input tape.
     * A block is read from the input tape and it is transformed
     * into a succession of pulses.
     * @return true if data is available; false otherwise.
     * @see PulseBuffer
     * @see RX
     */
    bool receive();

    /**
     * Start/Stop fastload mode.
     * If the _fastload variable is false this method does nothing.
     */
    void fastload(bool on);

    sptr_t<Clock> _clk;         /* System clock                     */
    bool          _fastload;    /* Do/don't emulate cassette speed  */

    TX            _tx{};        /* Transmission machine             */
    RX            _rx{};        /* Reception machine                */
};

}

using ZXSpectrumTape = tape::Tape;

}
}
}
