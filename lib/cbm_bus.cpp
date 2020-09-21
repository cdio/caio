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
#include "cbm_bus.hpp"

#include <sstream>

#include "logger.hpp"


//#define CEMU_CBMBUS_DEBUG
//#define CEMU_CBMBUS_DEBUG_STATE

#ifdef CEMU_CBMBUS_DEBUG
#define CBMBUS_DEBUG(fmt, args...)              do {                                                        \
                                                    log.debug("%s: %s: " fmt,                               \
                                                        bus_name().c_str(), dev_name().c_str(), ## args);   \
                                                } while (0)
#else
#define CBMBUS_DEBUG(...)
#endif

#ifdef CEMU_CBMBUS_DEBUG_STATE
#define CBMBUS_DEBUG_STATE(fmt, args...)        do {                                                        \
                                                    log.debug("%s: %s: " fmt,                               \
                                                        bus_name().c_str(), dev_name().c_str(), ## args);   \
                                                } while (0)
#else
#define CBMBUS_DEBUG_STATE(...)
#endif


namespace cemu {
namespace cbm_bus {

std::string BusData::to_string() const
{
    std::ostringstream os{};

    os << "SRQ "   << +srq()
       << ", ATN " << +atn()
       << ", CLK " << +clk()
       << ", DAT " << +dat()
       << ", RST " << +rst();

    return os.str();
}


bool Bus::add(Controller *dev)
{
    auto it = std::find_if(_devs.begin(), _devs.end(), [&dev](const Controller *rdev) -> bool {
        return (rdev == dev || rdev->unit() == dev->unit());
    });

    if (it != _devs.end()) {
        log.error("%s: Can't add device to bus: %s. Existing device: %s\n", Name::to_string().c_str(),
            dev->to_string().c_str(), (*it)->to_string().c_str());
        return false;
    }

    _devs.push_back(dev);
    return true;
}

void Bus::del(Controller *dev)
{
    auto it = std::find(_devs.begin(), _devs.end(), dev);
    if (it != _devs.end()) {
        _devs.erase(it);
    }
}

void Bus::propagate()
{
    BusData bd{};

    for (auto &dev : _devs) {
        bd &= dev->data();
    }

    _data = bd;
}

std::string Bus::to_string() const
{
    std::ostringstream os{};

    os << Name::to_string() << (_devs.size() ? ": " : "");

    for (auto it = _devs.begin(); it != _devs.end(); ++it) {
        os << (*it)->Name::to_string() << ((it + 1) == _devs.end() ? "" : ", ");
    }

    return os.str();
}


Controller::Controller(uint8_t unit, const std::shared_ptr<Bus> &bus, const std::string &label)
    : Name{TYPE, label},
      _unit{unit},
      _bus{bus}
{
    if (!_bus) {
        throw InvalidArgument{*this, "Empty bus parameter"};
    }

    _bus->add(this);
    _bus->propagate();
}


Device::Device(uint8_t unit, const std::shared_ptr<Bus> &bus)
    : Controller{unit, bus, LABEL_PREFIX + std::to_string(+unit)},
      Clockable{}
{
}

Device::~Device()
{
    _bus->del(this);
}

void Device::reset()
{
    _mode = Mode::IDLE;
    _role = Role::PASSIVE;
    _state = State::IDLE;
    release();
}

size_t Device::tick(const Clock &clock)
{
    //TODO: General timeout on blocking states.

    switch (_mode) {
    case Mode::IDLE:
        if (!(atn() == ACTIVE && clk() == ACTIVE)) {
            break;
        }

        /*
         * ATN and CLK active: The controller wants to send a command.
         */
        _cmd.clear();
        _mode = Mode::COMMAND;
        _role = Role::PASSIVE;
        state(State::IDLE);
        CBMBUS_DEBUG("ATN line ON: Mode IDLE -> Mode COMMAND\n");
        /* PASSTHROUGH */

    case Mode::COMMAND:
        if (_role == Role::PASSIVE && atn() == INACTIVE) {
            release();
            _mode = Mode::IDLE;
            CBMBUS_DEBUG("Passive device: Mode COMMAND -> Mode IDLE\n");
            break;
        }

        if (!tick_rx()) {
            break;
        }

        /*
         * ATN command received.
         */
        if (!parse_command(_bytetr.byte())) {
            /*
             * Unrecognised command.
             */
            release();
            _mode = Mode::WAIT;
            CBMBUS_DEBUG("Unrecognised command: Mode COMMAND -> Mode WAIT, bus %s\n", bus_data().to_string().c_str());
            break;
        }

        if (process_command()) {
            /*
             * Command processed, the secondary is not present.
             */
            release();
            _mode = Mode::IDLE;
            CBMBUS_DEBUG("Unselected device: Mode COMMAND -> Mode IDLE\n");
            break;
        }

        if (_role == Role::PASSIVE) {
            /*
             * Device not selected, release the bus and wait until ATN is OFF.
             *
             * It seems that passive devices must receive all commands
             * (and ignore them) until ATN is OFF then its bus lines
             * must be released. We won't do that unless it is necessary.
             */
            release();
            _mode = Mode::WAIT;
            CBMBUS_DEBUG("Passive device: Mode COMMAND -> Mode WAIT\n");
            break;
        }

        _mode = Mode::SECONDARY;
        CBMBUS_DEBUG("Selected device: Mode COMMAND -> Mode SECONDARY\n");
        /* PASSTHROUGH */

    case Mode::SECONDARY:
        if (!tick_rx()) {
            break;
        }

        /*
         * Secondary command received.
         */
        if (!parse_command(_bytetr.byte())) {
            /*
             * Unrecognised command: Go back to COMMAND mode.
             */
            _mode = Mode::COMMAND;
            _role = Role::PASSIVE;
            CBMBUS_DEBUG("Unrecognised command: Mode SECONDARY -> Mode COMMAND, new role PASSIVE\n");
            break;
        }

        if (process_secondary(false) == false) {
            /*
             * Secondary command executed.
             */
            if (_role == Role::TALKER) {
                /*
                 * Become a talker.
                 */
                _mode = Mode::TURNAROUND;
                CBMBUS_DEBUG("Selected device: Mode SECONDARY -> Mode TURNAROUND\n");
                break;
            }

            /*
             * Get the next command.
             */
            _mode = Mode::COMMAND;
            CBMBUS_DEBUG("Selected device: Mode SECONDARY -> Mode COMMAND\n");
            break;
        }

        /*
         * Secondary command execution delayed: More data is required.
         */
        _mode = Mode::DATA;
        CBMBUS_DEBUG("Selected device: Mode SECONDARY -> Mode DATA\n");
        /* PASSTHROUGH */

    case Mode::DATA:
        if (tick_rx()) {
            /*
             * Receive the secondary arguments.
             */
            _cmd.param(_bytetr.byte());
            if (_bytetr.last()) {
                /*
                 * Process the previously delayed secondary
                 * command then go back to COMMAND mode.
                 */
                process_secondary(true);
                _mode = Mode::COMMAND;
                CBMBUS_DEBUG("Selected device: Mode DATA -> Mode COMMAND\n");
            }
        }
        break;

    case Mode::WAIT:
        /*
         * Move to IDLE mode only when the ATN line is off.
         */
        if (atn() == INACTIVE) {
            _mode = Mode::IDLE;
            CBMBUS_DEBUG("Mode WAIT -> Mode IDLE\n");
        }
        break;

    case Mode::TURNAROUND:
        /*
         * Wait until the controller becomes a listener.
         */
        if (!(atn() == INACTIVE && clk() == INACTIVE)) {
            break;
        }

        /*
         * Acknowledge this device is now a talker.
         */
        dat(INACTIVE);
        clk(ACTIVE);
        _mode = Mode::TURN_HOLD;
        _time = 0;
        CBMBUS_DEBUG("Selected device: Mode TURNAROUND -> Mode TURN_HOLD\n");
        /* PASSTHROUGH */

    case Mode::TURN_HOLD:
        if (_time < TURN_HOLD_TIME) {
            break;
        }

        _bytetr = {};
        _bytetr.ready(true);
        _mode = Mode::TALKER;
        CBMBUS_DEBUG("Selected device: Mode TURN_HOLD -> Mode TALKER\n");
        /* PASSTHROUGH */

    case Mode::TALKER:
        if (atn() == ACTIVE) {
            /*
             * ATN active: Go back to IDLE mode aborting the current transmission
             * (the controller can turn ON the ATN line at any moment).
             */
            release();
            _mode = Mode::IDLE;
            if (!_bytetr.ready()) {
                /*
                 * The ongoing transmission is aborted. Put back the read byte.
                 */
                push_back(_cmd.chunit());
                CBMBUS_DEBUG("Talker device: ATN line ON: Mode TALKER -> Mode IDLE. "
                    "Pushed back untransmitted byte $%02X\n", _bytetr.byte());
            } else {
                CBMBUS_DEBUG("Talker device: ATN line ON: Mode TALKER -> Mode IDLE\n");
            }
            break;
        }

        if (_bytetr.last() && _bytetr.ready()) {
            /*
             * No more data to transmit: Go back to IDLE mode.
             * This is executed after the last byte is transmitted.
             */
            release();
            _mode = Mode::IDLE;
            CBMBUS_DEBUG("Talker device: No more data to transmit: Mode TALKER -> Mode IDLE\n");
            break;
        }

        /*
         * Transmit.
         */
        if (_bytetr.ready()) {
            auto rb = read(_cmd.chunit());
            if (rb.is_eof()) {
                /*
                 * Emtpy stream: Release the bus lines and go IDLE.
                 * When the stream is empty we just move to IDLE mode and stay there,
                 * after 512us in that state the controller recognises it is an empty stream
                 * (see https://www.pagetable.com/?p=1135#fnref:7, Empty Stream).
                 */
                release();
                _mode = Mode::IDLE;
                CBMBUS_DEBUG("Talker device: Empty stream: Mode TALKER -> Mode IDLE\n");
                break;
            }

            _bytetr.byte(rb.value(), rb.is_last());
            state(State::IDLE);

            CBMBUS_DEBUG("Talker device: Transmitting byte $%02X, islast %d, bus %s\n", _bytetr.byte(),
                _bytetr.last(), bus_data().to_string().c_str());
        }

        tick_tx();
        break;
    }

    /*
     * Do not starve the hosting cpu unnecessarily,
     * sleep for 900us when this cbm bus device is in idle mode.
     */
    uint64_t T = (_mode == Mode::IDLE ? 900 : 10);
    _time += T;
    return clock.cycles(T / 1000000.0f);
}

bool Device::tick_rx()
{
    switch (_state) {
    case State::IDLE:
        _bytetr = {};
        release();
        dat(ACTIVE);
        state(State::INIT);
        CBMBUS_DEBUG_STATE("RX: IDLE -> INIT\n");
        break;

    case State::INIT:
        if (clk() == INACTIVE) {
            /*
             * Talker wants to send data.
             */
            _bytetr = {};
            dat(INACTIVE);
            state(State::READY);
            CBMBUS_DEBUG_STATE("RX: INIT -> READY\n");
        }
        break;

    case State::READY:
        /*
         * We are ready to receive data.
         */
        if (_time < EOI_TIME) {
            if (clk() == ACTIVE) {
                /*
                 * Talker is transmitting a bit.
                 */
                state(State::BIT_WAIT);
                CBMBUS_DEBUG_STATE("RX: READY -> BIT_WAIT\n");
            }
        } else {
            /*
             * Talker is signaling the transmission of the last byte.
             */
            dat(ACTIVE);
            state(State::EOI);
            CBMBUS_DEBUG_STATE("RX: READY -> EOI\n");
        }
        break;

    case State::EOI:
        if (_time > EOI_HOLD_TIME) {
            /*
             * EOI ACK finished. Go back and receive the last byte.
             */
            dat(INACTIVE);
            _bytetr.last(true);
            state(State::READY);
            CBMBUS_DEBUG_STATE("RX: EOI -> READY\n");
        }
        break;

    case State::BIT_WAIT:
        if (clk() == INACTIVE) {
            /*
             * The data bit is ready.
             */
            _bytetr.bit(dat());
            state(State::BIT_DONE);
            CBMBUS_DEBUG_STATE("RX: BIT_WAIT -> BIT_DONE, received bit %d\n", dat());
        }
        break;

    case State::BIT_DONE:
        if (clk() == ACTIVE) {
            if (_bytetr.complete()) {
                /*
                 * Byte received. Frame handshake.
                 */
                state(State::FRAME);
                CBMBUS_DEBUG_STATE("RX: BIT_DONE -> FRAME\n");
            } else {
                /*
                 * Byte reception not complete. Get the next bit.
                 */
                state(State::BIT_WAIT);
                CBMBUS_DEBUG_STATE("RX: BIT_DONE -> BIT_WAIT\n");
            }
        } else if (_time > TIMEOUT) {
            log.error(bus_name() + ": " + dev_name() + ": RX: Timeout on state BIT_DONE. Moving to state IDLE\n");
            release();
            state(State::IDLE);
        }
        break;

    case State::FRAME:
        dat(ACTIVE);
        _bytetr.ready(true);
        state(State::IDLE);
        CBMBUS_DEBUG_STATE("RX: FRAME -> IDLE, received byte $%02X\n", _bytetr.byte());

        /* Byte reception completed */
        return true;

    case State::FRAME_WAIT:
        break;
    }

    /* Byte reception not completed */
    return false;
}

void Device::tick_tx()
{
    switch (_state) {
    case State::IDLE:
        if (dat() == INACTIVE) {
            /*
             * Wait until the listener becomes IDLE.
             */
            break;
        }

        /*
         * Ready to send.
         */
        clk(INACTIVE);
        CBMBUS_DEBUG_STATE("TX: IDLE -> INIT, time %lld, bus %s\n", _time, bus_data().to_string().c_str());
        state(State::INIT);
        /* PASSTHROUGH */

    case State::INIT:
        if (dat() == ACTIVE) {
            break;
        }

        /*
         * The listener is ready to receive.
         */
        CBMBUS_DEBUG_STATE("TX: INIT -> READY, time %lld, bus %s\n", _time, bus_data().to_string().c_str());
        state(State::READY);
        /* PASSTHROUGH */

    case State::READY:
        if (_time < NON_EOI_TIME) {
            /*
             * Wait a little bit before the transmission starts.
             */
            break;
        }

        if (!_bytetr.last()) {
            /*
             * This is not the last byte we are sending: Transmit it normally.
             */
            CBMBUS_DEBUG_STATE("TX: READY -> BIT_WAIT, time %lld\n", _time);
            clk(ACTIVE);
            state(State::BIT_WAIT);
            break;
        }

        /*
         * This is the last byte we are sending:
         * Do nothing and wait until the listener acknowledges the EOI.
         */
        if (dat() == INACTIVE) {
            break;
        }

        CBMBUS_DEBUG_STATE("TX: READY -> EOI, time %lld\n", _time);
        state(State::EOI);
        /* PASSTHROUGH */

    case State::EOI:
        if (dat() == ACTIVE) {
            break;
        }

        /*
         * EOI handshake done.
         */
        CBMBUS_DEBUG_STATE("TX: EOI -> BIT_WAIT, time %lld\n", _time);
        clk(ACTIVE);
        dat(INACTIVE);
        state(State::BIT_WAIT);
        /* PASSTHROUGH */

    case State::BIT_WAIT:
        if (_time < BIT_SETUP_TIME) {
            break;
        }

        /*
         * Transmit bit.
         */
        dat(_bytetr.bit());
        clk(INACTIVE);
        CBMBUS_DEBUG_STATE("TX: BIT_WAIT -> BIT_DONE, bit %d, time %lld\n", _data.dat(), _time);
        state(State::BIT_DONE);
        /* PASSTHROUGH */

    case State::BIT_DONE:
        if (_time < BIT_VALID_TIME) {
            break;
        }

        clk(ACTIVE);
        dat(INACTIVE);
        if (!_bytetr.complete()) {
            /*
             * Bit transmitted. Go for the next one.
             */
            CBMBUS_DEBUG_STATE("TX: BIT_DONE -> BIT_WAIT, time %lld\n", _time);
            state(State::BIT_WAIT);
            break;
        }

        /*
         * Byte completed: Frame handshake.
         */
        CBMBUS_DEBUG_STATE("TX: BIT_DONE -> FRAME, time %lld\n", _time);
        state(State::FRAME);
        /* PASSTHROUGH */

    case State::FRAME:
        if (_time > FRAME_TIMEOUT) {
            //FIXME abort() or something similar?
            log.error("%s: %s: TX: FRAME timeout error. time %lld, bus %s\n", bus_name().c_str(), dev_name().c_str(),
                _time, bus_data().to_string().c_str());
            state(State::IDLE);
            release();
            _mode = Mode::IDLE;
            break;
        }

        if (dat() == INACTIVE) {
            break;
        }

        /*
         * (Frame) Byte acknowledged by listener.
         */
        CBMBUS_DEBUG_STATE("TX: FRAME -> FRAME_WAIT, time %lld\n", _time);
        state(State::FRAME_WAIT);
        /* PASSTHROUGH */

    case State::FRAME_WAIT:
        if (_time < BETWEEN_BYTES_TIME) {
            break;
        }

        _bytetr.ready(true);

        if (_bytetr.last()) {
            /*
             * End of transmission.
             */
            release();
            CBMBUS_DEBUG_STATE("TX: FRAME_WAIT -> FRAME_WAIT, transmission ended\n");
        } else {
            /*
             * Ready to transmit next byte.
             */
            CBMBUS_DEBUG_STATE("TX: FRAME_WAIT -> IDLE, time %lld, bus %s\n", _time, bus_data().to_string().c_str());
            state(State::IDLE);
            break;
        }
    }
}

bool Device::parse_command(uint8_t byte)
{
    _cmd.clear();

    if (byte == UNLISTEN || byte == UNTALK) {
        _cmd.command(byte);
        return true;
    }

    uint8_t cmd = byte & SELECT_MASK;
    switch (cmd) {
    case LISTEN:
    case TALK:
        _cmd.command(cmd);
        _cmd.chunit(byte & UNIT_MASK);
        return true;

    default:;
    }

    cmd = byte & SECONDARY_MASK;
    switch (cmd) {
    case OPEN:
    case REOPEN:
    case CLOSE:
        _cmd.command(cmd);
        _cmd.chunit(byte & CHANNEL_MASK);
        return true;

    default:;
    }

    log.error("%s: %s: Invalid command: $%02x\n", bus_name().c_str(), dev_name().c_str(), byte);
    return false;
}

bool Device::process_command()
{
    switch (_cmd.command()) {
    case LISTEN:
        _role = (_unit == _cmd.chunit() ? Role::LISTENER : Role::PASSIVE);
        CBMBUS_DEBUG("Exec: LISTEN unit %d, role %s\n", _cmd.chunit(),
            (_role == Role::PASSIVE ? "PASSIVE" : "LISTENER"));
        break;

    case TALK:
        _role = (_unit == _cmd.chunit() ? Role::TALKER : Role::PASSIVE);
        CBMBUS_DEBUG("Exec: TALK unit %d, role %s\n", _cmd.chunit(), (_role == Role::PASSIVE ? "PASSIVE" : "TALKER"));
        break;

    case UNLISTEN:
        _role = Role::PASSIVE;
        CBMBUS_DEBUG("Exec: UNLISTEN\n");
        return true;

    case UNTALK:
        _role = Role::PASSIVE;
        CBMBUS_DEBUG("Exec: UNTALK\n");
        return true;

    default:;
    }

    return false;
}

bool Device::process_secondary(bool with_param)
{
    switch (_cmd.command()) {
    case OPEN:
        if (with_param) {
            auto arg = _cmd.param_str();
            CBMBUS_DEBUG("Exec: OPEN channel %d, param \"%s\"\n", _cmd.chunit(), arg.c_str());
            open(_cmd.chunit(), arg);
        } else {
            /*
             * OPEN has an optional argument and it was not received.
             */
            CBMBUS_DEBUG("Exec delayed: OPEN channel %d\n", _cmd.chunit());
            return true;
        }
        break;

    case CLOSE:
        CBMBUS_DEBUG("Exec: CLOSE channel %d\n", _cmd.chunit());
        close(_cmd.chunit());   /* FIXME: Should this be done just after UNLISTEN? */
        break;

    case REOPEN:
        if (_role == Role::LISTENER) {
            if (!with_param) {
                /*
                 * REOPEN as listener has an optional data buffer and it was not received.
                 */
                CBMBUS_DEBUG("Exec delayed: REOPEN channel %d\n", _cmd.chunit());
                return true;
            } else {
                /*
                 * Write the received buffer into the specified channel.
                 */
                write(_cmd.chunit(), _cmd.param());
            }
        } else {
            /*
             * REOPEN a channel as a talker.
             */
            CBMBUS_DEBUG("Exec: REOPEN channel %d\n", _cmd.chunit());
        }
        break;

    default:;
    }

    return false;
}

}
}
