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

#include "fs.hpp"
#include "types.hpp"

namespace caio {
namespace sinclair {
namespace zxspectrum {

/**
 * TAP file.
 * Read/Write TAP files.
 * @see https://worldofspectrum.org/faq/reference/formats.htm#TAP
 */
struct TAPFile {
public:
    constexpr static const char* FILE_EXTENSION     = ".tap";
    constexpr static const char* FILE_PATTERN       = "*.tap";
    constexpr static const size_t FILE_SIZE_LIMIT   = 512000;

    using Block = std::span<const uint8_t>;

    /**
     * Load a TAP file or directory.
     * @param path TAP file name or directory.
     * @exception IOError
     * @see load(std::string_view)
     */
    TAPFile(std::string_view path = {});

    virtual ~TAPFile();

    /**
     * Load a TAP file.
     * Reset this tap file and:
     * - If the specified path is a regular file, it is assumed to be TAP formatted.
     * - If the specified path is a directory, it is traversed and up to
     *   fs::DIR_ENTRIES_LIMIT TAP files that do not exceed FILE_SIZE_LIMIT
     *   are taken and concatenated into a single big TAP file.
     * - If the specified path is an empty string, reset is done.
     * @param path TAP file name or directory.
     * @exception IOError
     * @see fs::DIR_ENTRIES_LIMIT
     * @see FILE_SIZE_LIMIT
     * @see FILE_EXTENSION
     */
    void load(std::string_view path);

    /**
     * Retrieve the next block.
     * @return The next TAP block or nullptr if there are no more blocks.
     * @exception IOError
     */
    Block next_block();

    /**
     * Create a simple TAP file consisting of a header block and a data block.
     * If the specified file exists it is appended with the new content.
     * @param fname  TAP file name;
     * @param header Header block;
     * @param data   Data block.
     * @exception IOError
     * @see Block
     */
    static void save(std::string_view fname, Block header, Block data);

private:
    void reset();

    bool more_data();

    std::string         _path{};
    fs::dir_t           _entries{};
    fs::dir_t::iterator _dirit{};
    buffer_t            _buf{};
    size_t              _bufpos{};
};

}
}
}
