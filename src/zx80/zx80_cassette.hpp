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

#include <functional>
#include <string>
#include <vector>

#include "clock.hpp"
#include "fs.hpp"
#include "types.hpp"

namespace caio {
namespace sinclair {
namespace zx80 {

/**
 * ZX80 Cassette interface.
 * The cassette interface process cassette pulses.
 * When a "save" operation is performed from basic, this class converts
 * pulses to a stream of bytes.
 * When a "load" operation is performed from basic, this class converts
 * a stream of byts to pulses.
 *
 * This class must be derived by file handlers.
 */
class ZX80Cassette {
public:
    constexpr static const uint64_t SYNC_PULSE_TIME_MIN = 1000;
    constexpr static const uint64_t SYNC_PULSE_TIME_MAX = 1800;
    constexpr static const uint64_t SYNC_DURATION       = 1'000'000;    /* Duration of sync state       */
    constexpr static const uint64_t DATA_PULSE_TIME     = 150;          /* 150us (+/- 10us)             */
    constexpr static const uint64_t DATA_PULSE_TIME_MAX = 400;          /* 300us (+/- 100us)            */
    constexpr static const size_t BIT_0_PULSE_COUNT     = 4;            /* Bit 0: 4 high pulses         */
    constexpr static const size_t BIT_1_PULSE_COUNT     = 9;            /* Bit 1: 9 high pulses         */
    constexpr static const size_t BIT_SEPARATOR_TIME    = 1300;         /* Bit separation time 1300us   */
    constexpr static const int END_OF_FILE              = -1;
    constexpr static const int END_OF_TAPE              = -2;

    virtual ~ZX80Cassette();

    /**
     * Read a pulse from this cassette.
     * @return The pulse value.
     * @see receive(bool rewind, bool peek)
     */
    bool read();

    /**
     * Generate a cassette pulse.
     * @see transmit(int)
     */
    void write(bool pulse);

    /**
     * Restart this cassette interface.
     * Reset the reception and transmission state machines.
     */
    virtual void restart();

    /**
     * Get the cassette activity status.
     * @return true if the cassette is idle; false if the cassette is loading or saving data.
     */
    bool is_idle() const;

protected:
    enum class RxCmd {
        Read,
        Peek,
        Rewind
    };

    /**
     * Initialise this cassette interface.
     * @param clk System clock.
     * @warning The clock parameter must be valid otherwise the process is terminated.
     */
    ZX80Cassette(const sptr_t<Clock>& clk);

    /**
     * Transmit a byte to this cassette.
     * @param data Byte to transmit (END_OF_FILE to terminate the transmission).
     */
    virtual void transmit(int data) = 0;

    /**
     * Receive a byte from this cassette.
     * The command parameter specifies how the value must be retrieved from the tape:
     * - RxCmd::Read:   Read the value and advance the tape (normal play, this is the default);
     * - RxCmd::Peek:   Read the value but do not advance the tape;
     * - RxCmd::Rewind: Rewind the tape.
     * @return The received byte or END_OF_FILE or END_OF_TAPE in case the current file or
     * the tape is ended.
     * @see RxCmd
     */
    virtual int receive(RxCmd cmd = RxCmd::Read) = 0;

private:
    enum class State {
        Init,
        Sync,
        Data,
        End
    };

    sptr_t<Clock>   _clk;                       /* System clock                 */

    State           _tx_state{State::Init};     /* Transmission machine state   */
    bool            _tx_pulse{};                /* Current transmitted pulse    */
    uint64_t        _tx_time{};                 /* Transmitted pulse duration   */
    unsigned        _tx_count{};                /* Transmitted pulse count      */
    uint8_t         _tx_bit{};                  /* Transmitted bit              */
    uint8_t         _tx_byte{};                 /* Transmitted byte             */

    State           _rx_state{State::Init};     /* Reception machine state      */
    uint8_t         _rx_byte{};                 /* Received byte                */
    uint8_t         _rx_bit{};                  /* Received bit                 */
    uint64_t        _rx_time{};                 /* Received pulse duration      */
    uint8_t         _rx_count{};                /* Received pulse count         */
};

/**
 * ZX80 Cassette interface for the 4K ROM (.o files).
 * The 4K ROM does not specify file names for load or save operations.
 * This implementation uses an arbitrarily chosen name as a file name
 * and places it in the host filesystem under a user specified directory
 * using the .o extension.
 *
 * @see ZX80CassetteO::CASSETTE_FNAME
 * @see ZX80Cassette
 */
class ZX80CassetteO : public ZX80Cassette {
public:
    constexpr static const char* CASSETTE_FNAME = "basic";
    constexpr static const char* CASSETTE_EXT   = ".o";

    using buffer_t = std::vector<uint8_t>;
    using buffer_it_t = buffer_t::iterator;

    /**
     * Initialise this cassette interface.
     * @param clk     System clock;
     * @param cassdir Cassette directory.
     * @exception IOError if the specified cassette directory does not exist or it is not a directory.
     * @warning The clock parameter must be valid otherwise the process is terminated.
     * @see ZX80Cassette(const sptr_t<Clock>&)
     */
    ZX80CassetteO(const sptr_t<Clock>& clk, const std::string& cassdir);

    virtual ~ZX80CassetteO();

    /**
     * Restart this cassette interface.
     * Reset the reception and transmission state machines
     * and clear internal data buffers.
     * @see ZX80Cassette::restart()
     */
    void restart() override;

protected:
    /**
     * Generate a file name.
     * The 4K ROM saves and loads files to/from the cassette without specifying a name,
     * this emulator uses an arbitrarily chosen name: CASSETTE_FNAME.
     * @return The full pathname.
     * @see CASSETTE_FNAME
     * @see ZX80CassetteO(const sptr_t<Clock>&, const std::string&)
     */
    std::string fname() const;

    /**
     * Transmit a byte to this cassette.
     * @param data Byte to transmit (END_OF_FILE to terminate the transmission).
     * When the transmission is terminated a file is generated in
     * the host filesystem using the file name specified by fname().
     * If the file already exists it is overwritten.
     * @see fname()
     */
    void transmit(int data) override;

    /**
     * Receive a byte from this cassette.
     * The command parameter specifies how the value must be retrieved from the tape:
     * - RxCmd::Read:   Read the value and advance the tape (normal play, this is the default);
     * - RxCmd::Peek:   Read the value but do not advance the tape;
     * - RxCmd::Rewind: Rewind the tape. A file specified by fname() is read from
     *                  the host filesystem and its data is used to feed this method.
     * @return The received byte or END_OF_FILE or END_OF_TAPE in case the current file or
     * the tape is ended.
     * @see fname()
     * @see ZX80Cassette::receive(RxCmd)
     */
    int receive(RxCmd cmd = RxCmd::Read) override;

    std::string _cassdir;   /* Cassette directory       */

    buffer_t    _buf{};     /* Data buffer              */
    buffer_it_t _it{};      /* Input buffer iterator    */
};

/**
 * ZX80 Cassette interface for the 8K ROM (.p files).
 * The 8K ROM specifies file names for load and save operations.
 * This implementation uses those names to write to/read from
 * files that are placed in the host filesystem under a user
 * specified directory, using the .p extension.
 *
 * @see ZX80Cassette
 */
class ZX80CassetteP : public ZX80CassetteO {
public:
    constexpr static const char CASSETTE_EXT[]      = { ".p" };
    constexpr static const char* CASSETTE_PATTERN   = "*.p";

    /**
     * Initialise this cassette interface.
     * @param clk     System clock;
     * @param cassdir Cassette directory.
     * @exception IOError if the specified cassette directory does not exist or it is not a directory.
     * @warning The clock parameter must be valid otherwise the process is terminated.
     * @see ZX80CassetteO(const sptr_t<Clock>&, const std::string&)
     */
    ZX80CassetteP(const sptr_t<Clock>& clk, const std::string& cassdir);

    virtual ~ZX80CassetteP();

    /**
     * Convert a 8K ROM character to ascii.
     * @param ch Character code.
     * @return The ASCII value or 255 if it cannot be converted.
     */
    static char zx81_to_ascii(uint8_t ch);

    /**
     * Convert from ascii to 8K ROM character.
     * @param ch ASCII code.
     * @return The 8K ROM character code or 255 if it cannot be converted.
     */
    static uint8_t ascii_to_zx81(char ch);

private:
    /**
     * Generate a full path mame.
     * @param basename File name.
     * @return The full pathname.
     */
    std::string fname(const std::string& basename) const;

    /**
     * Extract a filename from the data buffer received from the ZX80.
     * If the data buffer does not contain a valid name, the default is used.
     * @retuen The filename converted to ascii.
     * @see CASSETTE_FNAME
     */
    std::string extract_name();

    /**
     * Transmit a byte to this cassette.
     * @param data Byte to transmit (END_OF_FILE to terminate the transmission).
     * When the transmission is terminated a file is generated in the
     * host filesystem using the file name specified in the received stream.
     * If the file already exists it is overwritten.
     */
    void transmit(int data) override;

    /**
     * Receive a byte from this cassette.
     * The command parameter specifies how the value must be retrieved from the tape:
     * - RxCmd::Read:   Read the value and advance the tape (normal play, this is the default);
     * - RxCmd::Peek:   Read the value but do not advance the tape;
     * - RxCmd::Rewind: Rewind the tape. The cassette directory is opened and traversed
     *                  looking for .p files, these files are sequentially read during
     *                  normal operation.
     * @return The received byte or END_OF_FILE or END_OF_TAPE in case the current file or
     * the tape is ended.
     * @see fname()
     * @see ZX80Cassette::receive(RxCmd)
     */
    int receive(RxCmd cmd = RxCmd::Read) override;

    fs::dir_t           _entries{};     /* .p filenames under the cassette directory    */
    fs::dir_t::iterator _dirit{};       /* Entries iterator                             */
};

}
}
}
