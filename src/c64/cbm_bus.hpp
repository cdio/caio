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

#include <cstdint>
#include <functional>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "clock.hpp"
#include "name.hpp"
#include "types.hpp"
#include "utils.hpp"


namespace caio {
namespace commodore {
namespace cbm_bus {

using buf_t = std::vector<uint8_t>;

/**
 * CBM-BUS data.
 */
class BusData {
public:
    constexpr static const bool RELEASED = true;

    BusData() {
    }

    BusData(const BusData& bd)
        : _srq{bd._srq},
          _atn{bd._atn},
          _clk{bd._clk},
          _dat{bd._dat},
          _rst{bd._rst} {
    }

    bool is_released() const {
        return (_srq == RELEASED && _atn == RELEASED && _clk == RELEASED && _dat == RELEASED && _rst == RELEASED);
    }

    bool srq() const {
        return _srq;
    }

    bool atn() const {
        return _atn;
    }

    bool clk() const {
        return _clk;
    }

    bool dat() const {
        return _dat;
    }

    bool rst() const {
        return _rst;
    }

    void srq(bool release) {
        _srq = release;
    }

    void atn(bool release) {
        _atn = release;
    }

    void clk(bool release) {
        _clk = release;
    }

    void dat(bool release) {
        _dat = release;
    }

    void rst(bool release) {
        _rst = release;
    }

    void release() {
        _srq = _atn = _clk = _dat = _rst = RELEASED;
    }

    BusData& operator&=(const BusData& bd) {
        _srq &= bd._srq;
        _atn &= bd._atn;
        _clk &= bd._clk;
        _dat &= bd._dat;
        _rst &= bd._rst;
        return *this;
    }

    std::string to_string() const;

private:
    bool _srq{RELEASED};
    bool _atn{RELEASED};
    bool _clk{RELEASED};
    bool _dat{RELEASED};
    bool _rst{RELEASED};
};

/**
 * CBM-BUS (Commodore-IEC serial bus).
 * The CBM-BUS is a stripped down version of IEEE-488/IEC-425.
 * @see http://www.zimmers.net/anonftp/pub/cbm/programming/serial-bus.pdf
 */
class Bus : public Name {
public:
    constexpr static const char* TYPE = "CBM-BUS";

    explicit Bus(const std::string& label = "")
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
    bool add(class Controller* dev);

    /**
     * Detach a device from this bus.
     * @param dev Device to detach.
     */
    void del(class Controller* dev);

    /**
     * @return A reference to this bus' data lines.
     */
    const BusData& data() const {
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
    BusData                        _data{};     /* Bus lines                        */
    std::vector<class Controller*> _devs{};     /* Devices connected to this bus    */
};

/**
 * CBM-BUS Controlller.
 * The bus controller handles the BUS lines.
 */
class Controller : public Name {
public:
    constexpr static const char* TYPE              = "CBM-BUS-DEVICE";
    constexpr static const char* LABEL             = "controller";
    constexpr static const uint8_t CONTROLLER_UNIT = 255;

    /**
     * Initialise this bus controller.
     * @param bus   Bus to connect to;
     * @param label Label asssigned to this controller.
     * @exception InvalidArgument if the bus is empty.
     */
    Controller(const sptr_t<Bus>& bus)
        : Controller{CONTROLLER_UNIT, bus, LABEL} {
    }

    virtual ~Controller() {
    }

    uint8_t unit() const {
        return _unit;
    }

    const BusData& bus_data() const {
        return _bus->data();
    }

    const BusData& data() const {
        return _data;
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

protected:
    /**
     * Initialise this bus controller.
     * @param unit  Unit number assigned to this controller;;
     * @param bus   Bus to connect to;
     * @param label Label asssigned to this controller.
     * @exception InvalidArgument if the bus is empty.
     */
    Controller(uint8_t unit, const sptr_t<Bus>& bus, const std::string& label = LABEL);

    uint8_t     _unit;
    sptr_t<Bus> _bus;
    BusData     _data{};
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

    buf_t& param() {
        return _param;
    }

    std::string param_str() const {
        return utils::to_string({_param.data(), _param.size()});
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
class Device : public Controller, public Clockable {
public:
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

    constexpr static const uint64_t NON_EOI_TIME        = 60;           /* T_NE */
    constexpr static const uint64_t EOI_TIME            = 200;          /* T_YE */
    constexpr static const uint64_t EOI_HOLD_TIME       = 80;           /* T_EI */
    constexpr static const uint64_t TURN_HOLD_TIME      = 80;           /* T_DA */

    constexpr static const uint64_t BIT_SETUP_TIME      = 80;           /* T_S  */
    constexpr static const uint64_t BIT_VALID_TIME      = 80;           /* T_V  */

    constexpr static const uint64_t BETWEEN_BYTES_TIME  = 100;          /* T_BB */
    constexpr static const uint64_t EOI_ACK_TIME        = 60;           /* T_FR */
    constexpr static const uint64_t FRAME_TIMEOUT       = 1000;

    constexpr static const uint64_t TIMEOUT             = 1'000'000;    /* Timeout of 1s on blocking states */

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
        PASSIVE,            /* Device not selected but listening ATN commands   */
        LISTENER,           /* This device is a listener                        */
        TALKER              /* This device is a talker                          */
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
    Device(uint8_t unit, const sptr_t<Bus>& bus);

    virtual ~Device();

    /**
     * Reset this device.
     * This device is moved to IDLE mode and the bus lines are released.
     */
    virtual void reset();

    /**
     * Open a channel.
     * @param ch    Channel to open;
     * @param param Channel parameters.
     */
    virtual void open(uint8_t ch, const std::string& param) = 0;

    /**
     * Close a channel.
     * @param ch Channel to close.
     */
    virtual void close(uint8_t ch) = 0;

    /**
     * Read a byte from a channel.
     * @param ch Channel to read from.
     * @return The read byte.
     * @see ReadByte
     * @see push_back()
     */
    virtual ReadByte read(uint8_t ch) = 0;

    /**
     * Push back the previous read byte.
     * This method is called when the bus controller aborts
     * (activating the ATN line) an ongoing transmission
     * from this device in order to push the untransmitted
     * value back into the channel.
     * @param ch Channel.
     * @see read()
     */
    virtual void push_back(uint8_t ch) = 0;

    /**
     * Write a byte buffer into a channel.
     * @param ch    Channel to write to:
     * @param value Buffer to write.
     */
    virtual void write(uint8_t ch, const buf_t& buf) = 0;

    /**
     * Get the device activity status.
     * @return true if the device is idle; false if the device is receiving or transmitting data.
     */
    bool is_idle() const {
        return (_role == Role::PASSIVE);
    }

protected:
    /**
     * @see Clockable::tick()
     */
    size_t tick(const Clock& clock) override;

    bool tick_rx();

    void tick_tx();

    bool parse_command(uint8_t byte);

    bool process_command();

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
    Mode     _mode{Mode::IDLE};
    Role     _role{Role::PASSIVE};
    State    _state{State::IDLE};
    BusData  _data{};
    uint64_t _time{};
    Command  _cmd{};
    ByteTR   _bytetr{};
};

}
}
}
