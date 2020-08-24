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
#pragma once

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "clock.hpp"
#include "name.hpp"
#include "types.hpp"
#include "utils.hpp"


namespace cemu {
namespace cbm_bus {

using buf_t = std::vector<uint8_t>;


/**
 * CBM-BUS data.
 */
class BusData {
public:
    constexpr static const uint8_t SRQ        = 0x01;
    constexpr static const uint8_t ATN        = 0x02;
    constexpr static const uint8_t CLK        = 0x04;
    constexpr static const uint8_t DAT        = 0x08;
    constexpr static const uint8_t RST        = 0x10;
    constexpr static const uint8_t RELEASED   = 0xFF;


    explicit BusData(uint8_t data = RELEASED)
        : _data{data} {
    }

    BusData(const BusData &bd)
        : BusData{bd._data} {
    }

    bool is_released() const {
        return (_data == RELEASED);
    }

    bool srq() const {
        return (_data & SRQ);
    }

    bool atn() const {
        return (_data & ATN);
    }

    bool clk() const {
        return (_data & CLK);
    }

    bool dat() const {
        return (_data & DAT);
    }

    bool rst() const {
        return (_data & RST);
    }

    void srq(bool release) {
        _data = (release ? (_data | SRQ) : (_data & ~SRQ));
    }

    void atn(bool release) {
        _data = (release ? (_data | ATN) : (_data & ~ATN));
    }

    void clk(bool release) {
        _data = (release ? (_data | CLK) : (_data & ~CLK));
    }

    void dat(bool release) {
        _data = (release ? (_data | DAT) : (_data & ~DAT));
    }

    void rst(bool release) {
        _data = (release ? (_data | RST) : (_data & ~RST));
    }

    void release() {
        _data = RELEASED;
    }

    BusData &operator&=(const BusData &bd) {
        _data &= bd._data;
        return *this;
    }

    std::string to_string() const;

private:
    uint8_t _data{RELEASED};
};


/**
 * CBM-BUS (Commodore-IEC serial bus).
 * The CBM-BUS is a stripped down version of IEEE-488/IEC-425.
 * @see http://www.zimmers.net/anonftp/pub/cbm/programming/serial-bus.pdf
 */
class Bus : public Name {
public:
    constexpr static const char *TYPE = "CBM-BUS";


    explicit Bus(const std::string &label = "")
        : Name{TYPE, label} {
    }

    virtual ~Bus() {
    }

    /**
     * Attach a new device to this bus.
     * @param dev Device to attach.
     * @return True on success; false if another device with
     * the same unit number is already attached to this bus.
     */
    bool add(class Device *dev);

    /**
     * Detach a device from this bus.
     * @param dev Device to detach.
     */
    void del(class Device *dev);

    /**
     * @return A reference to this bus' data lines.
     */
    const BusData &data() const {
        return _data;
    }

    /**
     * Recalculate the status of this bus' data lines.
     * Traverse all connected devices and update the status of this bus lines.
     */
    void propagate();

    /**
     * @return The name of this bus and the list of devices connected to it.
     * @see cbm_bus::Device::to_string()
     */
    std::string to_string() const override;

private:
    BusData                     _data{};    /* Bus lines                        */
    std::vector<class Device *> _devs{};    /* Devices connected to this bus    */
};


/**
 * Byte being transmitted or received.
 */
class ByteTR {
public:
    ByteTR() {
    }

    /**
     * @return The byte being transmitted or received.
     */
    uint8_t byte() const {
        return _byte;
    }

    /**
     * @return True if this is the last byte to be transmitted or received; false otherwise.
     */
    bool last() const {
        return _last;
    }

    /**
     * @return True if this byte is ready to be transmitted or it is fully received
     * and the state machine is ready for another byte; false otherwise.
     */
    bool ready() const {
        return _ready;
    }

    /**
     * @return True if the transmission or reception of this byte is completed
     * but the state machine is not ready for another byte; false otherwise.
     */
    bool complete() const {
        return (_curbit == 0);
    }

    /**
     * Set a new byte to transmit.
     * @param byte Byte to transmit;
     * @param last True if this is the last byte to transmit.
     */
    void byte(uint8_t byte, bool last = false) {
        _byte   = byte;
        _last   = last;
        _ready  = false;
        _curbit = 1;
    }

    /**
     * Set the last state of this byte.
     * @param last True if this is the last byte to be transmitted or received; false otherwise.
     */
    void last(bool last) {
        _last = last;
    }

    /**
     * Set the ready state of this byte.
     * A byte is ready when all its bits are completely transmitted or received.
     * @param ready True if this is byte is ready; false otherwise.
     * @see ready()
     */
    void ready(bool ready) {
        _ready = ready;
    }

    /**
     * Set a new received bit.
     * @param bit Received bit (true is 1, false is 0).
     */
    void bit(bool bit) {
        if (bit) {
            _byte |= _curbit;
        }
        _curbit <<= 1;
    }

    /**
     * @return The next bit to transmit (true is 1, false is 0).
     */
    bool bit() {
        bool b = _byte & _curbit;
        _curbit <<= 1;
        return b;
    }

private:
    uint8_t _byte{};
    bool    _ready{};
    uint8_t _curbit{1};
    bool    _last{};
};


/**
 * Received command.
 */
class Command {
public:
    Command() {
    }

    ~Command() {
    }

    bool operator!() const {
        return (_cmd == 0);
    }

    operator bool() const {
        return (_cmd != 0);
    }

    uint8_t command() const {
        return _cmd;
    }

    uint8_t chunit() const {
        return _chunit;
    }

    buf_t &param() {
        return _param;
    }

    std::string param_str() const {
        return utils::to_string(_param);
    }

    void command(uint8_t cmd) {
        _cmd = cmd;
    }

    void chunit(uint8_t chunit) {
        _chunit = chunit;
    }

    void param(uint8_t byte) {
        _param.push_back(byte);
    }

    void clear() {
        _cmd = 0;
        _chunit = 0;
        _param.clear();
    }

private:
    uint8_t _cmd{};
    uint8_t _chunit{};
    buf_t   _param{};
};


/**
 * Byte read from a channel.
 */
class ReadByte {
public:
    ReadByte()
        : _value{-1} {
    }

    ReadByte(uint8_t byte, bool is_last)
        : _value{byte | (is_last ? ~255 : 0)} {
    }

    uint8_t value() const {
        return (_value & 255);
    }

    bool is_last() const {
        return (_value < 0);
    }

    bool is_eof() const {
        return (_value == -1);
    }

private:
    int _value;
};


/**
 * CBM-BUS Device.
 * Device attached to a CBM-BUS.
 * This class must be derived by the actual device implementation.
 */
class Device : public Name, public Clockable {
public:
    constexpr static const char *TYPE                   = "CBM-BUS-DEVICE";
    constexpr static const char *LABEL_PREFIX           = "unit-";

    constexpr static const bool ACTIVE                  = false;
    constexpr static const bool INACTIVE                = true;

    constexpr static const uint8_t LISTEN               = 0x20;
    constexpr static const uint8_t UNLISTEN             = 0x3F;
    constexpr static const uint8_t TALK                 = 0x40;
    constexpr static const uint8_t UNTALK               = 0x5F;
    constexpr static const uint8_t REOPEN               = 0x60;
    constexpr static const uint8_t CLOSE                = 0xE0;
    constexpr static const uint8_t OPEN                 = 0xF0;

    constexpr static const uint8_t SELECT_MASK          = 0xE0;
    constexpr static const uint8_t SECONDARY_MASK       = 0xF0;

    constexpr static const uint8_t UNIT_MASK            = 0x1F;
    constexpr static const uint8_t CHANNEL_MASK         = 0x0F;

    constexpr static const size_t MAX_CHANNELS          = 16;

    constexpr static const uint64_t NON_EOI_TIME        = 60;               /* T_NE */
    constexpr static const uint64_t EOI_TIME            = 200;              /* T_YE */
    constexpr static const uint64_t EOI_HOLD_TIME       = 80;               /* T_EI */
    constexpr static const uint64_t TURN_HOLD_TIME      = 80;               /* T_DA */

    constexpr static const uint64_t BIT_SETUP_TIME      = 80;               /* T_S  */
    constexpr static const uint64_t BIT_VALID_TIME      = 80;               /* T_V  */

    constexpr static const uint64_t BETWEEN_BYTES_TIME  = 100;              /* T_BB */
    constexpr static const uint64_t EOI_ACK_TIME        = 60;               /* T_FR */
    constexpr static const uint64_t FRAME_TIMEOUT       = 1000;

    constexpr static const uint64_t TIMEOUT             = 1'000'000;        /* Timeout of 1s on blocking states */


    /*
     * Device operation mode.
     */
    enum class Mode {
        IDLE,               /* IDLE                                             */
        COMMAND,            /* The controller is sending a command              */
        SECONDARY,          /* The controller is sending a secondary address    */
        DATA,               /* The controller is sending secondary data         */
        TURNAROUND,         /* This device is becoming a talker                 */
        TURN_HOLD,          /* This device is becoming a talker (ack)           */
        TALKER,             /* This device is a talker                          */
        WAIT                /* Wait until ATN is disabled                       */
    };


    /*
     * Device role.
     */
    enum class Role {
        NONE,               /* Not selected                                     */
        PASSIVE,            /* Not selected but listening ATN commands          */
        LISTENER,           /* Selected, this device is a listener              */
        TALKER              /* Selected, this device is a talker                */
    };


    /**
     * Bit TX/RX state.
     */
    enum class State {
        IDLE,
        INIT,
        READY,
        EOI,
        BIT_WAIT,
        BIT_DONE,
        FRAME,
        FRAME_WAIT
    };


    /**
     * Initialise this CBM-BUS Device.
     * @param unit The unit number;
     * @param bus  The CBM-BUS to connect to.
     * @exception InvalidArgument
     */
    Device(uint8_t unit, const std::shared_ptr<Bus> &bus);

    virtual ~Device();

    /**
     * Reset this device.
     * This device is moved to IDLE mode and the bus lines are released.
     */
    virtual void reset();

    /**
     * @return This device's unit number.
     */
    uint8_t unit() const {
        return _unit;
    }

    /**
     * @return A reference to this device's lines (not the bus lines).
     * @see bus_data()
     * @see BusData
     */
    const BusData &data() const {
        return _data;
    }

    /**
     * Open a channel.
     * @param ch    Channel to open (0..15);
     * @param param Channel parameters.
     */
    virtual void open(uint8_t ch, const std::string &param) = 0;

    /**
     * Close a channel.
     * @param ch Channel to close (0..15).
     */
    virtual void close(uint8_t ch) = 0;

    /**
     * Read a byte from a channel.
     * @param ch Channel to read from (0..15).
     * @return The read byte.
     * @see ReadByte
     */
    virtual ReadByte read(uint8_t ch) = 0;

    /**
     * Write a byte buffer into a channel.
     * @param ch    Channel to write to (0..15);
     * @param value Buffer to write.
     */
    virtual void write(uint8_t ch, const buf_t &buf) = 0;

protected:
    /**
     * @see Clockable::tick()
     */
    size_t tick(const Clock &clock) override;

    const BusData &bus_data() const {
        return _bus->data();
    }

    bool srq() const {
        return bus_data().srq();
    }

    bool atn() const {
        return bus_data().atn();
    }

    bool clk() const {
        return bus_data().clk();
    }

    bool dat() const {
        return bus_data().dat();
    }

    bool rst() const {
        return bus_data().rst();
    }

    void srq(bool release) {
        _data.srq(release);
        _bus->propagate();
    }

    void atn(bool release) {
        _data.atn(release);
        _bus->propagate();
    }

    void clk(bool release) {
        _data.clk(release);
        _bus->propagate();
    }

    void dat(bool release) {
        _data.dat(release);
        _bus->propagate();
    }

    void rst(bool release) {
        _data.rst(release);
        _bus->propagate();
    }

    void release() {
        _data.release();
        _bus->propagate();
    }

    bool tick_rx();

    void tick_tx();

    bool parse_command(uint8_t byte);

    void process_command();

    bool process_secondary(bool with_param);

    void state(State state) {
        _state = state;
        _time = 0;
    }

    bool is_timeout(uint64_t timeout) const {
        return (_time >= timeout);
    }

    const std::string bus_name() const {
        return _bus->type();
    }

    const std::string dev_name() const {
        return label();
    }

private:
    uint8_t              _unit;
    std::shared_ptr<Bus> _bus;

    Mode                 _mode{Mode::IDLE};
    Role                 _role{Role::NONE};
    State                _state{State::IDLE};
    BusData              _data{};
    uint64_t             _time{};
    Command              _cmd{};
    ByteTR               _bytetr{};
};


/**
 * CBM-BUS Controlller.
 * A bus controller handles the BUS lines directly and it is not scheduled by a clock.
 */
class Controller : public Device {
public:
    constexpr static const char *LABEL  = "controller";
    constexpr static const uint8_t UNIT = 255;


    Controller(const std::shared_ptr<Bus> &bus)
        : Device{UNIT, bus} {
        label(LABEL);
    }

    virtual ~Controller() {
    }

private:
    void open(uint8_t ch, const std::string &param) override {
    }

    void close(uint8_t ch) override {
    }

    ReadByte read(uint8_t ch) override {
        return {};
    }

    void write(uint8_t ch, const buf_t &value) override {
    }

    size_t tick(const Clock &clock) override {
        throw InternalError{"Bus Controller can't be ticked"};
    }
};

}
}
