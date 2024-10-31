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
#include "tapfile.hpp"

#include <fstream>

#include "endian.hpp"
#include "logger.hpp"

namespace caio {
namespace sinclair {
namespace zxspectrum {

TAPFile::TAPFile(const fs::Path& path)
{
    load(path);
}

TAPFile::~TAPFile()
{
}

void TAPFile::reset()
{
    _path = "";
    _entries = {};
    _dirit = _entries.end();
    _buf = {};
    _bufpos = 0;
}

void TAPFile::load(const fs::Path& path)
{
    reset();

    if (!path.empty()) {
        _path = fs::fix_home(path);
        if (!fs::exists(_path)) {
            throw IOError{"{}: {}", _path.string(), Error::to_string(ENOENT)};
        }

        auto isdir = fs::is_directory(_path);

        log.debug("TAPFile: Loading: \"{}\", is_directory: {}\n", _path.string(), isdir);

        if (isdir) {
            _entries = fs::directory(_path, FILE_PATTERN, fs::MATCH_CASE_INSENSITIVE);

        } else {
            /*
             * Assume the specifeed file is a TAP file.
             */
            auto fsize = fs::file_size(_path);
            _entries.push_back({_path, fsize});
        }

        /*
         * Remove files exceeding FILE_SIZE_LIMIT.
         */
        fs::Dir::iterator it{};
        do {
            for (it = _entries.begin(); it != _entries.end(); ++it) {
                if (it->second > FILE_SIZE_LIMIT || it->second < 3) {
                    log.warn("TAPFile: Found: \"{}\", size: {}, min size: 3, max size: {}. Ignored\n",
                        it->first.string(), it->second, FILE_SIZE_LIMIT);
                    _entries.erase(it);
                    break;
                }
            }
        } while (it != _entries.end());

        if (_entries.size()) {
            _dirit = _entries.begin();
        }

        for (const auto& entry : _entries) {
            log.debug("TAPFile: Found: \"{}\", size: {}\n", entry.first.string(), entry.second);
        }
    }
}

bool TAPFile::more_data()
{
    if (_bufpos < _buf.size()) {
        return true;
    }

    if (_dirit == _entries.end()) {
        return false;
    }

    const auto& fname = _dirit->first;

    log.debug("TAPFile: Feeding file: \"{}\"\n", fname.string());

    _buf = fs::load(fname);
    _bufpos = 0;
    ++_dirit;
    return true;
}

TAPFile::Block TAPFile::next_block()
{
    if (!more_data()) {
        log.debug("TAPFile: No more blocks\n");
        return {};
    }

    /*
     * The first two bytes in a TAP block encode the block size.
     * The block size is the size of the data excluding the first 2 bytes.
     */
    const uint8_t* data = _buf.data() + _bufpos;
    uint16_t block_size = le16toh(*reinterpret_cast<const uint16_t*>(data));

    _bufpos += 2 + block_size;
    if (_bufpos > _buf.size()) {
        log.error("TAPFile: {}: Invalid block size: {}. TAP aborted.\n", (_dirit - 1)->first.string(), block_size);
        reset();
        return {};
    }

    data += 2;
    log.debug("TAPFile: New block: type: ${:02X}, size: {}\n", *data, block_size);
    return {data, block_size};
}

void TAPFile::save(const fs::Path& fname, Block header, Block data)
{
    const auto fullpath = fs::fix_home(fname);

    std::ofstream os{fullpath, std::ios_base::out | std::ios_base::app};
    if (!os) {
        throw IOError{"Can't open TAP file: {}: {}", fullpath.string(), Error::to_string()};
    }

    uint8_t size_lo = header.size() & 255;
    uint8_t size_hi = header.size() >> 8;

    if (!os.write(reinterpret_cast<const char*>(&size_lo), 1) ||
        !os.write(reinterpret_cast<const char*>(&size_hi), 1) ||
        !os.write(reinterpret_cast<const char*>(header.data()), header.size())) {
        throw IOError{"Can't save TAP header block: {}: {}", fullpath.string(), Error::to_string()};
    }

    size_lo = data.size() & 255;
    size_hi = data.size() >> 8;

    if (!os.write(reinterpret_cast<const char*>(&size_lo), 1) ||
        !os.write(reinterpret_cast<const char*>(&size_hi), 1) ||
        !os.write(reinterpret_cast<const char*>(data.data()), data.size())) {
        throw IOError{"Can't save TAP data block: {}: {}", fullpath.string(), Error::to_string()};
    }
}

}
}
}
