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
#include "c1541_fs.hpp"

#include <iomanip>
#include <regex>

#include "fs.hpp"
#include "logger.hpp"
#include "types.hpp"
#include "utils.hpp"


namespace cemu {
namespace c1541 {

void C1541Fs::attach(const std::string &path)
{
    if (!path.empty() && (!fs::exists(path) || !fs::is_directory(path))) {
        std::ostringstream os{};
        os << "Can't attach: " << std::quoted(path) << ": Not a directory";
        throw IOError{*this, os.str()};
    }

    attached_path(path.size() && path.rfind("/") != path.size() - 1 ? path + "/" : path);
    reset();
}

std::string C1541Fs::to_basic(addr_t &addr, const std::filesystem::path &fspath, size_t fsize)
{
    /*
     * PRG format for LOAD "$", 8:
     *  $0401 $next $line <...> \0
     *  $next $line <...> \0
     *  ...
     *  $next $free <BLOCKS FREE...> \0 \0 \0
     *
     * Each entry is 32 bytes long.
     */
    constexpr static const addr_t TEXT_SIZE = 28;                   /* 28 char + \0             */
    constexpr static const addr_t LINE_SIZE = TEXT_SIZE + 2;        /* text + basic line number */
    constexpr static const addr_t STATEMENT_SIZE = LINE_SIZE + 2;   /* line + next address      */

    std::ostringstream os{};

    if (addr == 0) {
        /*
         * Header: Disk name, id.
         */
        addr = c1541::PRG_START_ADDR + STATEMENT_SIZE;
        std::string disk_name = u8_to_pet(attached_path()).substr(0, 16);

        os << static_cast<uint8_t>(c1541::PRG_START_ADDR & 255)
           << static_cast<uint8_t>(c1541::PRG_START_ADDR >> 8)
           << static_cast<uint8_t>(addr & 255)
           << static_cast<uint8_t>(addr >> 8)
           << static_cast<uint8_t>(0)
           << static_cast<uint8_t>(0)
           << static_cast<uint8_t>(0x12)
           << "\"" << std::setw(16) << std::left << disk_name << "\" FS 2A"
           << static_cast<uint8_t>(0);

    } else if (fsize != static_cast<size_t>(-1)) {
        /*
         * Body: Directory entries.
         */
        auto fname = fspath;
        const std::string entry{fname.replace_extension("").string()};
        const std::string ext{fspath.extension().string()};

        std::string pentry{u8_to_pet(entry).substr(0, 16)};
        addr_t blocks = (fsize >> 8) | 1;

        std::string stype;
        if (ext == ".prg") {
            stype = "PRG";
        } else if (ext == ".seq") {
            stype = "SEQ";
        } else {
            stype = "???";
        }

        size_t hspace = 3 - (std::to_string(blocks).size() - 1);
        size_t tspace = 2 + 3 - hspace;

        os << static_cast<uint8_t>(addr & 255)
           << static_cast<uint8_t>(addr >> 8)
           << static_cast<uint8_t>(blocks & 255)
           << static_cast<uint8_t>(blocks >> 8)
           << std::setw(hspace) << std::setfill(' ') << " "
           << std::setw(18) << std::left << ("\"" + pentry + "\"")
           << " " << stype << std::setw(tspace) << std::setfill(' ') << " "
           << static_cast<uint8_t>(0);

    } else {
        /*
         * Footer: Blocks free.
         */
        constexpr static size_t width = TEXT_SIZE - 3 /* \0\0\0 */;

        os << static_cast<uint8_t>(addr & 255)
           << static_cast<uint8_t>(addr >> 8)
           << static_cast<uint8_t>(255)
           << static_cast<uint8_t>(255)
           << std::setw(width) << std::setfill(' ') << std::left << "BLOCKS FREE."
           << static_cast<uint8_t>(0)
           << static_cast<uint8_t>(0)
           << static_cast<uint8_t>(0);
    }

    addr += STATEMENT_SIZE;

    return os.str();
}

Status C1541Fs::open_dir(uint8_t ch, Channel &channel, const std::string &fname, FileType type, OpenMode mode)
{
    const std::string pattern{attached_path() + (fname.empty() ? "*" : fname)};
    size_t root_len = attached_path().size();
    std::stringstream ss{};
    addr_t addr{};

    ss << to_basic(addr, {}, 0);

    fs::directory(attached_path(), pattern,
        [this, &ss, &addr, &root_len](const std::string &entry, uint64_t size) -> bool {
            ss << to_basic(addr, entry.substr(root_len), size);
            return true;
        }
    );

    ss << to_basic(addr, {}, -1);

    channel.fname = fname;
    channel.type  = type;
    channel.mode  = mode;
    channel.fs    = {};
    channel.ss    = std::move(ss);
    channel.pos   = 0;
    channel.size  = channel.ss.str().size();

    log.debug("%s: Open directory listing \"%s\", pattern \"%s\" (\"%s\")\n", name(ch).c_str(),
        attached_path().c_str(), pattern.c_str(), fname.c_str());

    return Status::OK;
}

Status C1541Fs::open_file(uint8_t ch, Channel &channel, const std::string &fname, FileType type, OpenMode mode)
{
    std::ios_base::openmode fmode{std::ios_base::binary};

    switch (mode) {
    case OpenMode::READ:
        fmode |= std::ios_base::in;
        break;

    case OpenMode::WRITE:
        fmode |= std::ios_base::out;
        break;

    case OpenMode::TRUNC:
        fmode |= std::ios_base::out | std::ios_base::trunc;
        break;

    default:;
    }

    std::string ext{};
    switch (type) {
    case FileType::SEQ:
        ext = ".seq";
        break;

    case FileType::PRG:
        ext = ".prg";
        break;

    default:
        log.error("%s: Can't open file: \"%s\": Unsupported file type: %d\n", name(ch).c_str(), fname.c_str(),
            static_cast<unsigned>(type));
        return Status::FILE_TYPE_MISMATCH;
    }

    std::string fullpath{};

    if (mode == OpenMode::READ && (fname.empty() || c1541::is_pattern(fname))) {
        /*
         * READ mode: Name is missing or it is a pattern string.
         */
        if (fname == "*" && !_last_file.empty()) {
            /*
             * The '*' wildcard is replaced with the last accessed file.
             */
            fullpath = _last_file;

            log.debug("%s: Open file: \"%s\": Pevious accessed file: \"%s\"\n", name(ch).c_str(), fname.c_str(),
                fullpath.c_str());

        } else {
            /*
             * The '*' wildcard w/o a previously accessed file
             * loads the first file in the disk.
             */
            const std::string &pattern = attached_path() + c1541::pet_to_u8(fname) + ext;
            fs::directory(attached_path(), pattern, [&fullpath](const std::string &entry, uint64_t size) -> bool {
                fullpath = entry;
                return false;
            });

            if (fullpath.empty()) {
                log.debug("%s: Can't open file: \"%s\": No match for pattern \"%s\"\n", name(ch).c_str(), fname.c_str(),
                    pattern.c_str());
                return Status::FILE_NOT_FOUND;
            }

            log.debug("%s: Open file: \"%s\": Pattern \"%s\", match found: \"%s\"\n", name(ch).c_str(),
                fname.c_str(), pattern.c_str(), fullpath.c_str());
        }

    } else {
        /*
         * WRITE mode: The file name must be specified.
         */
        if (c1541::is_pattern(fname)) {
            log.debug("%s: Write file: Detected pattern matching expression: \"%s\"\n", name(ch).c_str(),
                fname.c_str());
            return Status::INVALID_FILENAME;
        }

        fullpath = attached_path() + fname + ext;
    }

    std::fstream fs{fullpath, fmode};
    if (!fs) {
        log.error("%s: Can't open file: \"%s\": %s\n", name(ch).c_str(), fullpath.c_str(), Error::to_string().c_str());

        switch (errno) {
        case EEXIST:
            return Status::FILE_EXISTS;

        case ENOENT:
            return Status::FILE_NOT_FOUND;

        default:;
        }

        return Status::INVALID_FILENAME;
    }

    channel.fname = fname;
    channel.type  = type;
    channel.mode  = mode;
    channel.fs    = std::move(fs);
    channel.ss    = {};
    channel.pos   = 0;
    channel.size  = fs::file_size(fullpath);

    if (type == FileType::PRG) {
        _last_file = fullpath;
    }

    return Status::OK;
}

Status C1541Fs::channel_open(uint8_t ch, const std::string &petfname, FileType type, OpenMode mode)
{
    auto &channel = _channels[ch];

    if (channel.is_open()) {
        if (ch != LOAD_CHANNEL && ch != SAVE_CHANNEL) {
            log.error("%s: Open \"%s\": Channel already open on file \"%s\"\n", name(ch).c_str(), petfname.c_str(),
                channel.fname.c_str());
            return Status::NO_CHANNELS_AVAILABLE;
        }

        channel_close(ch);
    }

    auto fname = pet_to_u8(petfname);

    auto it = std::find_if(_channels.begin(), _channels.end(), [&fname](const Channel &channel) -> bool {
        return (channel.is_open() && channel.fname == fname);
    });

    if (it != _channels.end()) {
        if (it->is_write() && mode == OpenMode::READ) {
            log.error("%s: Open \"%s\": File open for write on channel %d\n", name(ch).c_str(), petfname.c_str(),
                static_cast<unsigned>(it - _channels.begin()));
            return Status::FILE_OPEN_FOR_WRITE;
        }
    }

    return (mode == OpenMode::DIR ? open_dir(ch, channel, fname, type, mode) :
                                    open_file(ch, channel, fname, type, mode));
}

Status C1541Fs::channel_close(uint8_t ch)
{
    auto &channel = _channels[ch];

    if (channel.is_open()) {
        if (channel.fs) {
            channel.fs.flush();
        }

        channel = {};
        log.debug("%s: Channel closed\n", name(ch).c_str());
    }

    return Status::OK;
}

std::pair<ReadByte, Status> C1541Fs::channel_read(uint8_t ch)
{
    //TODO: PRGs are loaded on 401 not 801

    auto &channel = _channels[ch];

    if (!channel.is_open()) {
        return {{}, Status::FILE_NOT_OPEN};
    }

    if (channel.is_write()) {
        return {{}, Status::FILE_OPEN_FOR_WRITE};
    }

    if (channel.pos >= channel.size) {
        /*
         * EOF.
         */
        return {{}, Status::OK};
    }

    auto &is = (channel.mode == OpenMode::DIR ?
        static_cast<std::istream &>(channel.ss) : static_cast<std::istream &>(channel.fs));

    bool is_last = (channel.pos + 1 == channel.size);
    uint8_t value{};
    is.read(reinterpret_cast<char *>(&value), 1);
    if (is.fail()) {
        log.error("%s: Can't read: \"%s\": %s\n", name(ch).c_str(), channel.fname.c_str(), Error::to_string().c_str());
        return {{}, Status::READ_ERROR};
    }

    if (channel.pos == 0) {
        channel.elapsed = utils::now();
    }

    ++channel.pos;

    if (log.is_debug()) {
        size_t part = channel.size / 10;
        if (part == 0 || (channel.pos % part) == 0 || is_last) {
            float perc = (100.0f * channel.pos) / channel.size;
            log.debug("%s: Read %d of %d bytes (%.1f%%)\n", name(ch).c_str(), channel.pos, channel.size, perc);
        }
    }

    if (channel.pos == channel.size) {
        channel.elapsed = utils::now() - channel.elapsed;
        if (log.is_debug()) {
            if (channel.elapsed != 0) {
                float speed = channel.size / (channel.elapsed / 1000000.0f);
                log.debug("%s: Transmission rate %.01fBytes/s\n", name(ch).c_str(), speed);
            }
        }
    }

    return {ReadByte{value, is_last}, Status::OK};
}

void C1541Fs::channel_push_back(uint8_t ch)
{
    auto &channel = _channels[ch];

    if (channel.is_open() && !channel.is_write() && channel.pos > 0) {
        --channel.pos;

        auto &is = (channel.mode == OpenMode::DIR ?
            static_cast<std::istream &>(channel.ss) : static_cast<std::istream &>(channel.fs));

        is.seekg(-1, std::ios_base::cur);
    }
}

Status C1541Fs::channel_write(uint8_t ch, const buf_t &buf)
{
    auto &channel = _channels[ch];

    if (!channel.is_open()) {
        return Status::FILE_NOT_OPEN;
    }

    if (!channel.is_write()) {
        return Status::WRITE_ERROR;
    }

    channel.fs.write(reinterpret_cast<const char *>(&buf[0]), buf.size());
    if (channel.fs.fail()) {
        log.error("%s: Can't write: \"%s\": %s\n", name(ch).c_str(), channel.fname.c_str(), Error::to_string().c_str());
        return Status::WRITE_ERROR;
    }

    log.debug("%s: Write success, buffer size %d, data:\n%s\n", name(ch).c_str(), buf.size(), utils::dump(buf).c_str());

    return Status::OK;
}

Status C1541Fs::command(DOSCommand cmd, const std::string &param)
{
    switch (cmd) {
    case DOSCommand::COPY:
        /*
         * Copy or concatenate files:
         *      "COPY[0]:newfile=old1,old2,..."
         *      "C[0]:new=old1,..."
         */
        return copy(param);

    case DOSCommand::RENAME:
        /*
         * Rename:
         *      "RENAME[0]:new=old"
         *      "R[0]:new=old"
         * The renamed file must not be open.
         */
        return rename(param);

    case DOSCommand::SCRATCH:
        /*
         * Scratch:
         *      "SCRATCH[0]:name"
         *      "S[0]:name"
         * Remove file. name can have willcards (? and *)
         */
        return scratch(param);

    case DOSCommand::INITIALIZE:
    case DOSCommand::USER_9:
    case DOSCommand::USER_J:
        /*
         * Initialize:
         *      "INITIALIZE"
         *      "I"
         * Reset the unit drive.
         */
        return initialize();

    case DOSCommand::NEW:
        /*
         * Format Disk:
         *      "NEW[0]:name,id"
         *      "N[0]:name,id"
         * name: Name of the disk;
         * id:   2 chars.
         */
        break;

    case DOSCommand::VALIDATE:
        /*
         * Validate:
         *      "VALIDATE"
         *      "V"
         * Disk defrag.
         */
        break;

    case DOSCommand::DUPLICATE:
        /*
         * Duplicate:
         *      "DUPLICATE"
         * Copy disk between units.
         */
        break;

    case DOSCommand::BLOCK_READ:
    case DOSCommand::BLOCK_WRITE:
    case DOSCommand::BLOCK_ALLOCATE:
    case DOSCommand::BLOCK_FREE:
    case DOSCommand::BLOCK_EXECUTE:
    case DOSCommand::MEMORY_READ:
    case DOSCommand::MEMORY_WRITE:
    case DOSCommand::MEMORY_EXECUTE:
    case DOSCommand::BUFFER_POINTER:
    case DOSCommand::POSITION:
    case DOSCommand::USER_1:
    case DOSCommand::USER_2:
    case DOSCommand::USER_3:
    case DOSCommand::USER_4:
    case DOSCommand::USER_5:
    case DOSCommand::USER_6:
    case DOSCommand::USER_7:
    case DOSCommand::USER_8:
    case DOSCommand::UI_P:
    case DOSCommand::UI_M:
    default:;
    }

    return Status::OK;
}

Status C1541Fs::copy(const std::string &param)
{
    /*
     * Copy or concatenate files:
     *      "COPY[0]:newfile=old1,old2,..."
     *      "C[0]:new=old1,..."
     */
    static const std::regex re_copy("^[^:]*:([^=]+)=(.+)$", std::regex::extended);

    std::smatch result{};

    if (!std::regex_match(param, result, re_copy) || result.size() != 3) {
        return Status::COMMAND_INVALID_COMMAND;
    }

    const auto &newfile = result.str(1);
    if (newfile.empty()) {
        return Status::NO_FILE_GIVEN;
    }

    if (fs::exists(newfile)) {
        return Status::FILE_EXISTS;
    }

    auto catfiles = utils::split(result.str(2), ',');
    if (catfiles.size() == 0) {
        return Status::NO_FILE_GIVEN;
    }

    for (const auto &fname : catfiles) {
        if (c1541::is_pattern(fname)) {
            return Status::SYNTAX_ERROR;
        }
    }

    for (const auto &fname : catfiles) {
        if (!fs::exists(fname)) {
            return Status::FILE_NOT_FOUND;
        }

        try {
            fs::concat(newfile, fname);
        } catch (const IOError &err) {
            log.error("%s: %s: %s\n", name(15).c_str(), param.c_str(), err.what());
            return Status::WRITE_ERROR;  /* Another error code? */
        }
    }

    log.debug("%s: Files copied: \"%s\" -> \"%s\"\n", name(15).c_str(), result.str(2).c_str(), newfile.c_str());

    return Status::OK;
}

Status C1541Fs::rename(const std::string &param)
{
    /*
     * Rename:
     *      "RENAME[0]:new=old"
     *      "R[0]:new=old"
     * The renamed file must not be open.
     */
    static const std::regex re_rename("^[^:]*:([^=]+)=(.+)$", std::regex::extended);

    std::smatch result{};

    if (!std::regex_match(param, result, re_rename) && result.size() != 3) {
        return Status::COMMAND_INVALID_COMMAND;
    }

    const auto &newname = c1541::pet_to_u8(result.str(1));
    const auto &oldname = c1541::pet_to_u8(result.str(2));

    if (c1541::is_pattern(newname) || c1541::is_pattern(oldname)) {
        return Status::SYNTAX_ERROR;
    }

    if (newname.empty() || oldname.empty()) {
        return Status::NO_FILE_GIVEN;
    }

    if (fs::exists(newname)) {
        return Status::FILE_EXISTS;
    }

    if (!fs::exists(oldname)) {
        return Status::FILE_NOT_FOUND;
    }

    std::error_code err{};
    std::filesystem::rename(oldname, newname, err);
    if (err) {
        log.error("%s: \"%s\": %s\n", name(15).c_str(), param.c_str(), err.message().c_str());
        return Status::WRITE_ERROR;  /* Another error code? */
    }

    log.debug("%s: File renamed: \"%s\" -> \"%s\"\n", name(15).c_str(), oldname.c_str(), newname.c_str());

    return Status::OK;
}

Status C1541Fs::scratch(const std::string &param)
{
    /*
     * Scratch (remove) files:
     *      "SCRATCH[0]:<name>"
     *      "S[0]:<name>"
     *   <name> can have willcards.
     */
    static const std::regex re_scratch("^[^:]*:(.+)$", std::regex::extended);

    std::smatch result{};

    if (std::regex_match(param, result, re_scratch) && result.size() == 2) {
        auto remove = [this](const std::string &entry, uint64_t size) -> bool {
            if (!fs::unlink(entry)) {
                log.error("%s: Can't scratch: \"%s\": %s\n", name(15).c_str(), entry.c_str(),
                    Error::to_string().c_str());
                return false;
            }
            log.debug("%s: File scratched: \"%s\"\n", name(15).c_str(), entry.c_str());
            return true;
        };

        const auto &fname = attached_path() + c1541::pet_to_u8(result.str(1));
        bool success{};

        if (!fname.empty() && fs::exists(fname)) {
            success = remove(fname, 0);
        } else if (c1541::is_pattern(fname)) {
            success = fs::directory(attached_path(), fname, remove);
        }

        if (success) {
            return Status::FILES_SCRATCHED;
        }
    }

    return Status::WRITE_ERROR; /* Another error code? */
}

Status C1541Fs::initialize()
{
    reset();
    return Status::OK;
}

}
}
