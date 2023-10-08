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

#include <algorithm>
#include <array>
#include <atomic>
#include <fstream>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "fs.hpp"
#include "c1541.hpp"


namespace caio {
namespace c1541 {

/**
 * Commodore 1541 Filesystem.
 * This C1541 implementation gives access to a native directory.
 * @see C1541
 */
class C1541Fs : public C1541 {
public:
    constexpr static const char* TYPE = "C1541FS";

    /**
     * Initialise this 1541 Filesystem drive.
     * @param unit Unit number (usually 8, 9, 10 or 11);
     * @param bus  Bus to connect to.
     * @exception InvalidArgument
     * @see C1541(uint8_t, const sptr_t<cbm_bus::Bus>&)
     */
    C1541Fs(uint8_t unit, const sptr_t<cbm_bus::Bus>& bus)
        : C1541{unit, bus} {
        type(TYPE);
    }

    virtual ~C1541Fs() {
    }

    /**
     * @see C1541::attach()
     */
    void attach(const std::string& path) override;

private:
    /**
     * Channel data.
     */
    struct Channel {
        std::string       fname{};
        FileType          type{};
        OpenMode          mode{};
        std::fstream      fs{};
        std::stringstream ss{};
        size_t            pos{};
        size_t            size{};
        uint64_t          elapsed{};    /* debug/statistics */

        Channel() {
        }

        Channel(Channel&& other) {
            operator=(std::move(other));
        }

        Channel& operator=(Channel&& other) {
            fname    = std::move(other.fname);
            type     = other.type;
            mode     = other.mode;
            fs       = std::move(other.fs);
            ss       = std::move(other.ss);
            pos      = other.pos;
            size     = other.size;
            elapsed  = other.elapsed;
            return *this;
        }

        bool is_open() const {
            return (mode == OpenMode::DIR || fs.is_open());
        }

        bool is_write() const {
            return (mode != OpenMode::READ && mode != OpenMode::DIR);
        }
    };

    using channel_array_t = std::array<Channel, MAX_CHANNELS - 1>;

    /**
     * @see C1541::channel_open()
     */
    Status channel_open(uint8_t ch, const std::string& petfname, FileType type, OpenMode mode) override;

    /**
     * @see C1541::channel_close()
     */
    Status channel_close(uint8_t ch) override;

    /**
     * @see C1541::channel_read()
     */
    std::pair<ReadByte, Status> channel_read(uint8_t ch) override;

    /**
     * @see C1541::channel_push_back()
     */
    void channel_push_back(uint8_t ch) override;

    /**
     * @see C1541::channel_write()
     */
    Status channel_write(uint8_t ch, const buf_t& buf) override;

    /**
     * @see C1541::command(DOSCommand, const std::string&)
     */
    Status command(DOSCommand cmd, const std::string& param) override;

    std::string to_basic(addr_t& addr, const std::filesystem::path& fspath, size_t fsize);

    Status open_dir(uint8_t ch, Channel& channel, const std::string& fname, FileType type, OpenMode mode);

    Status open_file(uint8_t ch, Channel& channel, const std::string& fname, FileType type, OpenMode mode);

    Status copy(const std::string& param);

    Status rename(const std::string& param);

    Status scratch(const std::string& param);

    Status initialize();

    channel_array_t _channels{};
    std::string     _last_file{};
};

}
}
