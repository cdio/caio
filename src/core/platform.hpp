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

#include "clock.hpp"
#include "config.hpp"
#include "fs.hpp"
#include "name.hpp"
#include "types.hpp"
#include "ui.hpp"

#include <string>
#include <string_view>

namespace caio {

/**
 * Generic platform emulator.
 */
class Platform : public Name {
public:
    constexpr static const char* TYPE = "PLATFORM";

    using UI = ui::UI;
    using Config = config::Config;

    Platform(std::string_view label = "");

    /**
     * Build this platform and start it.
     * This method returns on error or when the
     * user terminates the emulator through the UI.
     * @param fname If not empty, name of a program, cartridge or
     * snapshot file to launch (its format is auto-detected).
     * @see create(const fs::Path&)
     * @see start()
     */
    virtual void run(const fs::Path& fname);

    /**
     * Return a human readable string describing the components that build this platform.
     * @return A human readable string representation of this platform.
     */
    std::string to_string() const override;

protected:
    /**
     * Return the user interface.
     * @return The user interface.
     */
    sptr_t<UI>& ui()
    {
        return _ui;
    }

    /**
     * Detect whether a file contains a snapshot image.
     * This method detects whether a file contains a snapshot
     * but it does not guarantee the validity of all its
     * components (for example cartridge or rom signatures).
     * That would be known during actual deserialization.
     * @param fname File name.
     * @return true if the specified file contains a snapshot; false otherwise.
     */
    bool is_snapshot(const fs::Path& fname) const;

    /**
     * Detect the format of a file.
     * If the specified file contains a snapshot image
     * set the proper configuration option accordingly.
     * @param fname File to detect.
     * @return true if the specified file is a snapshot image; false otherwise.
     */
    virtual bool detect_format(const fs::Path& fname);

    /**
     * Initialise the CPU monitor.
     * @param ifd Input file descriptor;
     * @param ofd Output file descriptor.
     * @see monitor::Monitor(inti, int, MonitoredCPU&&)
     */
    virtual void init_monitor(int ifd, int ofd) = 0;

    /**
     * Reset all the devices of this platform.
     * This method runs in the context of the UI thread.
     */
    virtual void reset_devices() = 0;

    /**
     * Return a human readable string describing the devices that build up this platform.
     * @return A human readable string representation of the devices of this platform.
     */
    virtual std::string to_string_devices() const = 0;

    /**
     * Instantiate the devices needed to build this platform.
     */
    virtual void create_devices() = 0;

    /**
     * Connect the devices and build this platform.
     * @see create_devices()
     */
    virtual void connect_devices() = 0;

    /**
     * Create the user interface widgets used by this platform.
     */
    virtual void make_widgets() = 0;

    /**
     * Connect this platform to the user interface.
     * Connect the hot-keys handler and widgets reset and pause
     * to the user interface.
     * The actual platform should derive this method to connect
     * the emulated keyboard, joystick ports and other widgets.
     */
    virtual void connect_ui();

    /**
     * Process hot-keys.
     * This method does nothing and it should be derived by the
     * actual platform that process specific hot-keys.
     *
     * It runs in the context of the UI (main) thread.
     */
    virtual void hotkeys(keyboard::Key key);

    /**
     * Get the system clock.
     * @return The system clock.
     */
    virtual Clock& clock() = 0;

    /**
     * Get the base configuraton.
     * @return The base configuration.
     */
    virtual Config& config() = 0;

    /**
     * Get the UI configuraton.
     * @return The UI configuration.
     */
    virtual ui::Config ui_config() = 0;

    /**
     * Serialize/Deserialize this platform.
     * @param ser Serializer/Deserializer.
     * @exception InvalidArgument
     * @exception IOError
     */
    virtual void serdes(Serializer& ser) = 0;

private:
    /**
     * Create this platform.
     * Instantiate and interconnect the platform devices,
     * instantiate the user interface and connect it to those devices.
     * If a snapshot file is configured, the devices are deserialized.
     * @param fname If not empty, name of a program, cartridge or
     * snapshot file to launch (its format is auto-detected).
     * @exception IOError
     * @exception InvalidArgument
     * @see detect_format(const fs::Path&)
     * @see create_devices()
     * @see connect_devices()
     * @see create_ui()
     * @see make_widgets()
     * @see connect_ui()
     * @see deserialize(const fs::Path&)
     */
    void create(const fs::Path& fname);

    /**
     * Start this platform.
     * - Run the emulator on its own thread;
     * - Run the UI on the calling thread.
     * This method returns on error or when the emualtion is terminated using the UI.
     */
    void start();

    /**
     * Reset this platform.
     * This method is connected to the UI's reset widget.
     * @see connect_ui()
     * @see reset_device();
     */
    void reset();

    /**
     * Create the user interface.
     * @see ui_config()
     */
    void create_ui();

    sptr_t<UI> _ui{};

    friend Serializer& operator&(Serializer&, Platform&);
};

}
