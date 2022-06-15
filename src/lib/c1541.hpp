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
#pragma once

#include <array>
#include <cctype>
#include <memory>
#include <string>

#include "cbm_bus.hpp"


namespace caio {
namespace c1541 {

using buf_t = cbm_bus::buf_t;
using ReadByte = cbm_bus::ReadByte;

constexpr static const uint8_t MAX_CHANNELS    = cbm_bus::Device::MAX_CHANNELS;
constexpr static const uint8_t CHANNEL_MASK    = cbm_bus::Device::CHANNEL_MASK;

constexpr static const uint8_t LOAD_CHANNEL    = 0;
constexpr static const uint8_t SAVE_CHANNEL    = 1;
constexpr static const uint8_t COMMAND_CHANNEL = 15;

constexpr static const size_t COMMAND_MAXLEN   = 58;

constexpr static const addr_t PRG_START_ADDR   = 0x0401;    /* C1541 forces this value on PRGs */


/**
 * Convert a PETSCII character to UTF-8.
 * @param petscii The PETSCII character.
 * @return The UTF-8 code.
 * @see u8_to_pet(uint8_t)
 * FIXME The conversion is not good
 */
static inline uint8_t pet_to_u8(uint8_t petscii)
{
    return ((petscii & ~128) | 32);
}


/**
 * Convert a UTF-8 character to PETSCII.
 * @param u8 The UTF-8 character.
 * @return The PETSCII code.
 * @see pet_to_u8(uint8_t)
 * FIXME The conversion is not good
 */
static inline uint8_t u8_to_pet(uint8_t u8)
{
    return std::toupper(u8);
}


/**
 * Convert a PETSCII string to UTF-8.
 * @param petscii The PETSCII string.
 * @return The UTF-8 string.
 * @see pet_to_u8(uint8_t)
 * @see u8_to_pet(string &)
 */
inline std::string pet_to_u8(const std::string &petscii)
{
    std::string u8{petscii};
    std::transform(u8.begin(), u8.end(), u8.begin(), static_cast<uint8_t(*)(uint8_t)>(&pet_to_u8));
    return u8;
}


/**
 * Convert a UTF-8 string to PETSCII.
 * @param u8 The UTF-8 string.
 * @return The PETSCII string.
 * @see u8_to_pet(uint8_t)
 * @see pet_to_u8(string &)
 */
inline std::string u8_to_pet(const std::string &u8)
{
    std::string petscii{u8};
    std::transform(petscii.begin(), petscii.end(), petscii.begin(), static_cast<uint8_t(*)(uint8_t)>(&u8_to_pet));
    return petscii;
}


/**
 * @return True if the string contains pattern matching values ('?', '*'); false otherwise.
 */
static inline bool is_pattern(const std::string &fname)
{
    return (fname.find_first_of("*?") != std::string::npos);
}


/**
 * Commodore 1541 status codes.
 */
enum class Status {
    OK                              = 0,        /* Not an error */
    FILES_SCRATCHED                 = 1,        /* Not an error */
    READ_ERROR_HEADER_NOT_FOUND     = 20,
    READ_ERROR_SYNC_NOT_FOUND       = 21,
    READ_ERROR_NO_DATA_BLOCK        = 22,
    READ_ERROR_CHECKSUM             = 23,
    READ_ERROR                      = 24,
    WRITE_ERROR                     = 25,
    WRITE_PROTECT_ON                = 26,
    READ_ERROR_HEADER_CHECKSUM      = 27,
    WRITE_ERROR_DATA_NEXT_BLOCK     = 28,
    DISK_ID_MISMATCH                = 29,
    SYNTAX_ERROR                    = 30,
    INVALID_COMMAND                 = 31,       /* Command not recognised                       */
    LINE_TOO_LONG                   = 32,       /* Command lenght > 58 characters               */
    INVALID_FILENAME                = 33,       /* OPEN/SAVE filename pattern matching failed   */
    NO_FILE_GIVEN                   = 34,       /* File name not present in a command           */
    COMMAND_INVALID_COMMAND         = 39,       /* Channel 15 command not recognised            */
    RECORD_NOT_PRESENT              = 50,
    OVERFLOW_IN_RECORD              = 51,
    FILE_TOO_LARGE                  = 52,
    FILE_OPEN_FOR_WRITE             = 60,
    FILE_NOT_OPEN                   = 61,
    FILE_NOT_FOUND                  = 62,
    FILE_EXISTS                     = 63,
    FILE_TYPE_MISMATCH              = 64,
    NO_BLOCK                        = 65,
    ILLEGAL_TRACK_OR_SECTOR         = 66,
    ILLEGAL_SYSTEM_TRACK_OR_SECTOR  = 67,
    NO_CHANNELS_AVAILABLE           = 70,
    DIRECTORY_ERROR                 = 71,
    DISK_OR_DIRECTORY_FULL          = 72,
    DOS_MISMATCH                    = 73,
    DRIVE_NOT_READY                 = 74
};


/**
 * Get the string representation of a status code.
 * @param st Status code.
 * @return The string representation of the specified status code.
 * @see Status
 */
std::string to_string(Status st);


/**
 * Commodore 1541 DOS commands.
 */
enum class DOSCommand {
    NEW,                    /* NEW:<name>,<id>                          */
    COPY,                   /* COPY:<newfile>=<oldfile>                 */
    RENAME,                 /* RENAME:<newname>=<oldname>               */
    SCRATCH,                /* SCRATCH:<name>                           */
    INITIALIZE,             /* INITIALIZE                               */
    VALIDATE,               /* VALIDATE                                 */
    DUPLICATE,              /* DUPLICATE                                */
    BLOCK_READ,             /* BLOCK-READ:<ch>,<drive>,<trk>,<sect>     */
    BLOCK_WRITE,            /* BLOCK-WRITE:<ch>,<drive>,<trk>,<sect>    */
    BLOCK_ALLOCATE,         /* BLOCK-ALLOCATE:<drv>,<trk>,<sect>        */
    BLOCK_FREE,             /* BLOCK-FREE:<drv>,<trk>,<sect>            */
    BLOCK_EXECUTE,          /* BLOCK-EXECUTE:<ch>,<drv>,<trk>,<sect>    */
    MEMORY_READ,            /* MEMORY-READ <lo <hi>                     */
    MEMORY_WRITE,           /* MEMORY-WRITE <lo> <hi> <n> <d1..dn>      */
    MEMORY_EXECUTE,         /* MEMORY-EXECUTE <lo> <hi>                 */
    BUFFER_POINTER,         /* BUFFER-POINTER:<ch,<position>            */
    POSITION,               /* POSITION:<chan>,<reclo>,<rechi>,<pos>    */
    USER_1,                 /* BLOCK-READ w/o changing buffer pointer   */
    USER_2,                 /* BLOCK-WRITE w/o changeing buffer pointer */
    USER_3,                 /* JMP $0500                                */
    USER_4,                 /* JMP $0503                                */
    USER_5,                 /* JMP $0506                                */
    USER_6,                 /* JMP $0509                                */
    USER_7,                 /* JMP $050C                                */
    USER_8,                 /* JMP $050F                                */
    USER_9,                 /* JMP $FFFA (soft reset)                   */
    USER_J,                 /* Power up vector (hard reset)             */
    UI_P,                   /* Set C64 speed                            */
    UI_M,                   /* Set VIC20 speed                          */
};


/**
 * Commodore 1541 file open modes.
 */
enum class OpenMode {
    READ,                   /* Open for read (filename matching)        */
    WRITE,                  /* Open/create for write                    */
    TRUNC,                  /* Open for write, truncate if it exists    */
    DIR,                    /* Open directory (filename matching)       */
};


/**
 * Commodore 1541 file types.
 */
enum class FileType {
    DEL = 0,
    SEQ = 1,
    PRG = 2,
    USR = 3,
    REL = 4,
};


/**
 * Commodore 1541 base class.
 * This class handles the bus communications, it parses and executes commands
 * received from the controller. Derived classes must implement these commands.
 */
class C1541 : public cbm_bus::Device {
public:
    constexpr static const char *TYPE = "C1541";


    /**
     * Initialise this C1541 drive.
     * In order to work this disk drive must be attached (see attach()).
     * @param unit Unit number (usually 8, 9, 10 or 11);
     * @param bus  Bus to connect to.
     * @exception InvalidArgument
     * @see cbm_bus::Device
     * @see attach()
     */
    C1541(uint8_t unit, const std::shared_ptr<cbm_bus::Bus> &bus)
        : cbm_bus::Device{unit, bus} {
        type(TYPE);
    }

    virtual ~C1541() {
    }

    /**
     * Attach this disk drive to a specified path.
     * An empty path detaches from a currently attached path.
     * @param path File, directory or device to attach to.
     * @exception IOError
     */
    virtual void attach(const std::string &path) = 0;

    /**
     * Reset this drive.
     * Close the open channels and reset the bus lines.
     * @see cbm_bus::Device::reset()
     */
    void reset() override;

    /**
     * @return A reference to the attached path.
     */
    const std::string &attached_path() const {
        return _attached_path;
    }

    /**
     * @return True if this disk drive is attached; false otherwise.
     */
    bool is_attached() const {
        return (!_attached_path.empty());
    }

    /**
     * @see cbm_bus::Device::to_string()
     */
    std::string to_string() const override;

protected:
    /**
     * Get the name of a channel.
     * @param ch Channel.
     * @return The name of a channel.
     */
    std::string name(uint8_t ch) const {
        return Name::to_string() + "-ch-" + std::to_string(ch);
    }

    /**
     * Open/Create a file on a channel.
     * @param ch       Channel;
     * @param petfname File name (PETSCII encoded);
     * @param type     File type;
     * @param mode     Open mode.
     * @return A status code.
     * @see FileType
     * @see OpenMode
     * @see Status
     */
    virtual Status channel_open(uint8_t ch, const std::string &petfname, FileType type, OpenMode mode) = 0;

    /**
     * Close a channel.
     * @param ch Channel.
     * @return A status code.
     * @see Status
     */
    virtual Status channel_close(uint8_t ch) = 0;

    /**
     * Read a value from a channel.
     * @param ch Channel.
     * @return The read value and a status code.
     * @see ReadByte
     * @see Status
     */
    virtual std::pair<ReadByte, Status> channel_read(uint8_t ch) = 0;

    /**
     * Put back a previously read value.
     * @param ch Channel.
     * @see Status
     */
    virtual void channel_push_back(uint8_t ch) = 0;

    /**
     * Write a data buffer into a channel.
     * @param ch  Channel;
     * @param buf Data buffer.
     * @return A status code.
     * @see Status
     */
    virtual Status channel_write(uint8_t ch, const buf_t &buf) = 0;

    /**
     * Execute a DOS command.
     * @param cmd   DOS Command code;
     * @param param Command parameters.
     * @return A status code.
     * @see DOSCommand
     * @see Status
     */
    virtual Status command(DOSCommand cmd, const std::string &param) = 0;

    /**
     * Set attached native path name.
     * @param path Path to attach to.
     */
    void attached_path(const std::string &path) {
        _attached_path = path;
    }

private:
    struct DiskCommand {
        const std::string name;     /* Command name                 */
        const std::string alias;    /* Command alias/Compact form   */
        DOSCommand        code;     /* Command code                 */
    };


    class StatusChannel {
    public:
        explicit StatusChannel(Status st, uint8_t track = 0, uint8_t sector = 0) {
            reset(st, track, sector);
        }

        StatusChannel &operator=(const Status &st) {
            reset(st, 0, 0);
            return *this;
        }

        bool operator==(const Status &st) const {
            return _status == st;
        }

        void reset(const Status &st, uint8_t track, uint8_t sector);

        ReadByte read();

        void push_back();

        std::string to_string() const;

    private:
        Status                _status{};
        uint8_t               _track{};
        uint8_t               _sector{};
        std::array<char, 128> _buf{};
        size_t                _size{};
        size_t                _pos{};
    };


    /**
     * @see cbm_bus::Device::open()
     */
    void open(uint8_t ch, const std::string &param) override;

    /**
     * @see cbm_bus::Device::close()
     */
    void close(uint8_t ch) override;

    /**
     * @see cbm_bus::Device::read()
     */
    ReadByte read(uint8_t ch) override;

    /**
     * @see cbm_bus::Device::push_back()
     */
    void push_back(uint8_t ch) override;

    /**
     * @see cbm_bus::Device::write()
     */
    void write(uint8_t ch, const buf_t &buf) override;

    /**
     * Execute a command.
     * This method is called when a DOS command is received.
     * @param param Command parameters.
     * @see COMMAND_CHANNEL
     * @see command(DOSCommand, const std::string &)
     */
    void command(const std::string &param);

    /**
     * Check wether this disk drive is attached.
     * If it is not attached the DRIVE_NOT_READY status code is set.
     * @return True if this disk drive is attached; false otherwise.
     * @see Status
     */
    bool check_attached();

    std::string   _attached_path{};
    StatusChannel _statusch{Status::OK};

    constexpr static const size_t COMMANDS = 35;
    static std::array<DiskCommand, COMMANDS> commands;
};

}
}
