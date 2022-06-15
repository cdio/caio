/*
 * Copyright (C) 2020-2022 Claudio Castiglia
 *
 * This file is part of CAIO.
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
#include "c1541.hpp"

#include <iomanip>
#include <regex>

#include "logger.hpp"
#include "types.hpp"
#include "utils.hpp"


namespace caio {
namespace c1541 {

std::array<C1541::DiskCommand, C1541::COMMANDS> C1541::commands{{
    { "NEW:",               "N:",   DOSCommand::NEW             },
    { "COPY:",              "C:",   DOSCommand::COPY            },
    { "RENAME:",            "R:",   DOSCommand::RENAME          },
    { "SCRATCH:",           "S:",   DOSCommand::SCRATCH         },
    { "INITIALIZE",         "I",    DOSCommand::INITIALIZE      },
    { "VALIDATE",           "V",    DOSCommand::VALIDATE        },
    { "DUPLICATE",          "D",    DOSCommand::DUPLICATE       },
    { "NEW0:",              "N0:",  DOSCommand::NEW             },
    { "COPY0:",             "C0:",  DOSCommand::COPY            },
    { "RENAME0:",           "R0:",  DOSCommand::RENAME          },
    { "SCRATCH0:",          "S0:",  DOSCommand::SCRATCH         },
    { "INITIALIZE0",        "I0",   DOSCommand::INITIALIZE      },
    { "VALIDATE0",          "V0",   DOSCommand::VALIDATE        },
    { "BLOCK-READ:",        "B-R:", DOSCommand::BLOCK_READ      },
    { "BLOCK-WRITE:",       "B-W:", DOSCommand::BLOCK_WRITE     },
    { "BLOCK_ALLOCATE:",    "B-A:", DOSCommand::BLOCK_ALLOCATE  },
    { "BLOCK-FREE:",        "B-F:", DOSCommand::BLOCK_FREE      },
    { "BLOCK-EXECUTE:",     "B-E:", DOSCommand::BLOCK_EXECUTE   },
    { "M-R",                "",     DOSCommand::MEMORY_READ     },
    { "M-W",                "",     DOSCommand::MEMORY_WRITE    },
    { "M-E",                "",     DOSCommand::MEMORY_EXECUTE  },
    { "BUFFER-POINTER:",    "B-P:", DOSCommand::BUFFER_POINTER  },
    { "POSITION",           "P",    DOSCommand::POSITION        },
    { "U1:",                "UA:",  DOSCommand::USER_1          },
    { "U2:",                "UB:",  DOSCommand::USER_2          },
    { "U3",                 "UC",   DOSCommand::USER_3          },
    { "U4",                 "UD",   DOSCommand::USER_4          },
    { "U5",                 "UE",   DOSCommand::USER_5          },
    { "U6",                 "UF",   DOSCommand::USER_6          },
    { "U7",                 "UG",   DOSCommand::USER_7          },
    { "U8",                 "UH",   DOSCommand::USER_8          },
    { "U9",                 "UI",   DOSCommand::USER_9          },
    { "U:",                 "UJ",   DOSCommand::USER_J          },
    { "UI+",                "",     DOSCommand::UI_P            },
    { "UI-",                "",     DOSCommand::UI_M            }
}};


std::string to_string(Status st)
{
    switch (st) {
    case Status::OK:
        return "OK";

    case Status::FILES_SCRATCHED:
        return "FILES SCRATCHED";

    case Status::READ_ERROR_HEADER_NOT_FOUND:
    case Status::READ_ERROR_SYNC_NOT_FOUND:
    case Status::READ_ERROR_NO_DATA_BLOCK:
    case Status::READ_ERROR_CHECKSUM:
    case Status::READ_ERROR:
    case Status::READ_ERROR_HEADER_CHECKSUM:
        return "READ ERROR";

    case Status::WRITE_ERROR:
    case Status::WRITE_ERROR_DATA_NEXT_BLOCK:
        return "WRITE ERROR";

    case Status::WRITE_PROTECT_ON:
        return "WRITE PROTECT ON";

    case Status::DISK_ID_MISMATCH:
        return "DISK ID MISMATCH";

    case Status::SYNTAX_ERROR:
    case Status::INVALID_COMMAND:
    case Status::LINE_TOO_LONG:
    case Status::INVALID_FILENAME:
    case Status::NO_FILE_GIVEN:
    case Status::COMMAND_INVALID_COMMAND:
        return "SYNTAX ERROR";

    case Status::RECORD_NOT_PRESENT:
        return "RECORD NOT PRESENT";

    case Status::OVERFLOW_IN_RECORD:
        return "OVERFLOW IN RECORD";

    case Status::FILE_TOO_LARGE:
        return "FILE TOO LARGE";

    case Status::FILE_OPEN_FOR_WRITE:
        return "WRITE FILE OPEN";

    case Status::FILE_NOT_OPEN:
        return "FILE NOT OPEN";

    case Status::FILE_NOT_FOUND:
        return "FILE NOT FOUND";

    case Status::FILE_EXISTS:
        return "FILE EXISTS";

    case Status::FILE_TYPE_MISMATCH:
        return "FILE TYPE MISMATCH";

    case Status::NO_BLOCK:
        return "NO BLOCK";

    case Status::ILLEGAL_TRACK_OR_SECTOR:
        return "ILLEGAL TRACK OR SECTOR";

    case Status::ILLEGAL_SYSTEM_TRACK_OR_SECTOR:
        return "ILLEGAL SYSTEM T OR S";

    case Status::NO_CHANNELS_AVAILABLE:
        return "NO CHANNEL";

    case Status::DIRECTORY_ERROR:
        return "DIRECTORY ERROR";

    case Status::DISK_OR_DIRECTORY_FULL:
        return "DISK FULL";

    case Status::DOS_MISMATCH:
        return "DOS MISMATCH";

    case Status::DRIVE_NOT_READY:
        return "DRIVE NOT READY";

    default:;
    }

    return "";
}


void C1541::reset()
{
    cbm_bus::Device::reset();
    close(COMMAND_CHANNEL);
    _statusch = Status::OK;
}

std::string C1541::to_string() const
{
    std::stringstream ss{};

    ss << Name::to_string() << ", path " << std::quoted(attached_path());

    return ss.str();
}

void C1541::open(uint8_t ch, const std::string &param)
{
    if (!check_attached()) {
        return;
    }

    if (ch == COMMAND_CHANNEL) {
        /*
         * Execute a DOS command.
         */
        //FIXME executed during CLOSE
        command(param);
        return;
    }

    /*
     * OPEN:
     *    OPEN file#, device#, channel#, "[[@|$][0]:]<name>[,<type>[,<dir>]]"
     *
     *    The @ prefix is used as "save & replace" (truncate instead of append)
     *    The $ prefix is the directory listing DOS command, in this case <name> is a pattern matching string.
     *
     *    name: File name in PETSCII (READ/SAVE) or a pattern matching string (READ mode only)
     *
     *    type: PRG | SEQ | USR | REL | P | S | U | R
     *          Default is PRG on READ and SEQ on WRITE
     *
     *    dir:  READ | WRITE | R | W
     *          Default is WRITE on save channel, READ on the rest.
     */
    static const std::regex re_open("^(@0:|@:|0:|\\$0?:|:?)([^,]+|.*$),?([^,]+|.*$),?(.*)$", std::regex::extended);

    constexpr static const size_t PREFIX  = 1;
    constexpr static const size_t NAME    = 2;
    constexpr static const size_t TYPE    = 3;
    constexpr static const size_t MODE    = 4;
    constexpr static const size_t MATCHES = 5;

    if (param.empty()) {
        _statusch = Status::NO_FILE_GIVEN;
        log.debug("%s: Open: Empty parameter\n", name(ch).c_str());
        return;
    }

    std::smatch result{};

    if (std::regex_match(param, result, re_open) && result.size() != MATCHES) {
        _statusch = Status::INVALID_FILENAME;
        log.debug("%s: Open: Match error: \"%s\"\n", name(ch).c_str(), param.c_str());
        return;
    }

    const std::string &prefix = result.str(PREFIX);
    std::string fname         = result.str(NAME);
    const std::string &type   = result.str(TYPE);
    const std::string &mode   = result.str(MODE);

    if (fname.empty() && prefix.starts_with("$")) {
        _statusch = Status::NO_FILE_GIVEN;
        log.debug("%s: Open: File name not detected: \"%s\"\n", name(ch).c_str(), param.c_str());
        return;
    }

    OpenMode omode;
    bool is_dir = (prefix.find("$") != std::string::npos);
    if (is_dir) {
        /*
         * If fname is defined it must be a matching expression.
         */
        omode = OpenMode::DIR;
    } else if (fname == "$" || fname == "$0") {
        omode = OpenMode::DIR;
        fname = "";
    } else {
        bool trunc = (prefix.find("@") == 0);
        if (mode.empty()) {
            /*
             * Default mode is WRITE on the save channel, READ on the rest.
             */
            omode = (ch == SAVE_CHANNEL ? OpenMode::WRITE : OpenMode::READ);
        } else if (mode == "R" || mode == "READ") {
            omode = OpenMode::READ;
        } else if (mode == "W" || mode == "WRITE") {
            omode = (trunc ? OpenMode::TRUNC : OpenMode::WRITE);
        } else {
            _statusch = Status::INVALID_FILENAME;
            log.debug("%s: Open: \"%s\": Invalid open mode: \"%s\"\n", name(ch).c_str(), param.c_str(), mode.c_str());
            return;
        }
    }

    /*
     * Overrided types:
     *   - PRG: Load and Save channels
     *   - PRG: Any other channel on READ mode
     *   - SEQ: Any other channel on WRITE mode
     */
    FileType ftype;
    if (ch == 0 || ch == 1) {
        ftype = FileType::PRG;
    } else if (type.empty()) {
        ftype = ((omode == OpenMode::WRITE || omode == OpenMode::TRUNC) ? FileType::SEQ : FileType::PRG);
    } else if (type == "P" || type == "PRG") {
        ftype = FileType::PRG;
    } else if (type == "S" || type == "SEQ") {
        ftype = FileType::SEQ;
    } else if (type == "U" || type == "USR") {
        ftype = FileType::USR;
    } else if (type == "L" || type == "REL") {
        ftype = FileType::REL;
    } else {
        _statusch = Status::INVALID_FILENAME;
        log.debug("%s: Open: \"%s\": Invalid file type: \"%s\"\n", name(ch).c_str(), param.c_str(), type.c_str());
        return;
    }

    _statusch = channel_open(ch, fname, ftype, omode);
    log.debug("%s: Open \"%s\": %s\n", name(ch).c_str(), param.c_str(), _statusch.to_string().c_str());
}

void C1541::close(uint8_t ch)
{
    if (!check_attached()) {
        return;
    }

    if (ch == COMMAND_CHANNEL) {
        for (uint8_t i = 0; i < MAX_CHANNELS - 1; ++i) {
            channel_close(i);
        }
        _statusch = Status::OK;
        log.debug("%s: Channel closed\n", name(ch).c_str());
    } else {
        _statusch = channel_close(ch);
    }
}

ReadByte C1541::read(uint8_t ch)
{
    if (!check_attached()) {
        return {};
    }

    if (ch == COMMAND_CHANNEL) {
        return _statusch.read();
    }

    auto [rb, st] = channel_read(ch);
    _statusch = st;
    return rb;
}

void C1541::push_back(uint8_t ch)
{
    if (check_attached()) {
        if (ch == COMMAND_CHANNEL) {
            _statusch.push_back();
        } else {
            channel_push_back(ch);
        }
    }
}

void C1541::write(uint8_t ch, const buf_t &buf)
{
    if (!check_attached()) {
        return;
    }

    if (ch == COMMAND_CHANNEL) {
        /*
         * PRINT#15, "<CMD>[\r]"
         */
        auto cmd = utils::to_string(buf);
        auto pos = cmd.rfind("\r");
        const auto &param = (pos == std::string::npos ? cmd : cmd.substr(0, pos));
        command(param);
    } else {
        /*
         * PRINT#n, "<SOMETHING>"
         */
        _statusch = channel_write(ch, buf);
    }
}

void C1541::command(const std::string &param)
{
    /*
     * OPEN#15,8,15, "<CMD>"
     * PRINT#15, "<CMD>[\r]"
     */
    Status st{Status::OK};

    if (!param.empty()) {
        if (param.size() > COMMAND_MAXLEN) {
            st = Status::LINE_TOO_LONG;

        } else {
            auto it = std::find_if(commands.begin(), commands.end(), [&param](const DiskCommand &dc) -> bool {
                return (param.find(dc.name) == 0 || param.find(dc.alias) == 0);
            });

            st = (it == commands.end() ? Status::INVALID_COMMAND : command(it->code, param));
        }
    }

    _statusch = st;

    log.debug("%s: Executed command \"%s\", result \"%s\"\n", name(COMMAND_CHANNEL).c_str(), param.c_str(),
        _statusch.to_string().c_str());
}

bool C1541::check_attached()
{
    if (is_attached()) {
        return true;
    }

    log.error(Name::to_string() + ": Unit is not attached\n");

    _statusch = Status::DRIVE_NOT_READY;
    return false;
}


void C1541::StatusChannel::reset(const Status &st, uint8_t track, uint8_t sector)
{
    /*
     * Fill the channel buffer with the execution status:
     * INPUT#15, A$, B$, C$, D$ [, E$]
     *    A$ = Status code
     *    B$ = Status string
     *    C$ = Disk track where the error occurred
     *    D$ = Sector on track where the error occurred
     *    E$ = Drive number (devices with more than one drive)
     */
    if (_status != st || _track != track || _sector != sector || _size == 0) {
        _status = st;
        _track = track;
        _sector = sector;

        _size = std::snprintf(_buf.data(), _buf.size(), "%02d\r%s\r%02d\r%02d\r", static_cast<uint8_t>(_status),
            c1541::to_string(_status).c_str(), _track, _sector);
    }

    _pos = 0;
}

ReadByte C1541::StatusChannel::read()
{
    if (_pos == _size) {
        reset(Status::OK, 0, 0);
    }

    ++_pos;
    return {static_cast<uint8_t>(_buf[_pos - 1]), _pos == _size};
}

void C1541::StatusChannel::push_back()
{
    if (_pos != 0) {
        --_pos;
    }
}

std::string C1541::StatusChannel::to_string() const
{
    std::stringstream os{};

    os << std::setfill('0') << std::setw(2) << std::to_string(static_cast<uint8_t>(_status)) << " - "
       << std::quoted(c1541::to_string(_status));

    return os.str();
}

}
}
