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
#include "platform.hpp"

#include "version.hpp"

#include <thread>

namespace caio {

Platform::Platform(std::string_view label)
    : Name{TYPE, label}
{
}

void Platform::run(const fs::Path& fname)
{
    create(fname);
    start();
}

void Platform::create(const fs::Path& fname)
{
    detect_format(fname);

    create_devices();
    connect_devices();

    create_ui();
    make_widgets();
    connect_ui();

    const auto& conf = config();
    const auto& snapshot = conf.snapshot;
    if (!snapshot.empty()) {
        /*
         * Load a snapshot file.
         */
        try {
            auto ser = Serializer::create_deserializer(snapshot);
            serdes(ser);
        } catch (const std::exception& err) {
            throw IOError{"Can't load snapshot file: {}: {}", snapshot, err.what()};
        }
    }

    if (conf.monitor) {
        // XXX FIXME input/output streams
        init_monitor(STDIN_FILENO, STDOUT_FILENO);
    }
}

void Platform::start()
{
    log.info("Starting {} - {}\n{}\n", full_version(), label(), to_string());

    /*
     * The emulator runs on its own thread.
     */
    std::exception_ptr error{};

    std::thread th{[&error, this]() {
        try {
            /*
             * Start emulation.
             */
            clock().run();

        } catch (...) {
            error = std::current_exception();
        }

        /*
         * The clock is terminated: Stop the user interface and exit this thread.
         */
        _ui->stop();
    }};

    if (!th.joinable()) {
        log.error("Can't start the clock thread: {}\n", Error::to_string());
        return;
    }

    /*
     * The UI runs on the main thread.
     */
    _ui->run();

    clock().stop();

    th.join();

    if (error) {
        std::rethrow_exception(error);
    }

    log.info("Terminating {}\n", label());
}

std::string Platform::to_string() const
{
    return std::format("{}\n\n"
        "Connected devices:\n"
        "{}\n\n"
        "UI backend: {}\n",
        const_cast<Platform*>(this)->config().to_string(),
        to_string_devices(),
        _ui->to_string());
}

void Platform::reset()
{
    /* This method runs in the context of the UI thread; see connect_ui()). */

    auto& clk = clock();

    if (!clk.paused()) {
        clk.pause_wait(true);
        reset_devices();
        clk.reset();
        clk.pause(false);
    }
}

void Platform::create_ui()
{
    const auto& uiconf = ui_config();
    _ui = ui::UI::instance(uiconf);
}

void Platform::connect_ui()
{
    /*
     * Connect Pause and Reset widgets.
     */
    const auto do_pause = [this](bool suspend) {
        clock().toggle_pause();
        log.debug("System {}paused\n", (clock().paused() ? "" : "un"));
    };

    const auto is_paused = [this]() {
        return clock().paused();
    };

    const auto do_reset = [this]() {
        reset();
    };

    _ui->pause(do_pause, is_paused);
    _ui->reset(do_reset);

    /*
     * Connect the hot-keys handler.
     */
    const auto hotkeys = [this](keyboard::Key key) {
        this->hotkeys(key);
    };

    _ui->hotkeys(hotkeys);
}

void Platform::hotkeys(keyboard::Key key)
{
}

bool Platform::is_snapshot(const fs::Path& fname) const
{
    if (!fname.empty()) {
        try {
            auto ser = Serializer::create_deserializer(fname);
            Name name{type()};

            ser & name;     /* Throws if different platform */

            return true;

        } catch (...) {
        }
    }

    return false;
}

bool Platform::detect_format(const fs::Path& fname)
{
    if (!is_snapshot(fname)) {
        return false;
    }

    auto& conf = config();
    if (!conf.snapshot.empty()) {
        log.warn("Snapshot file overrided. From {} to {}\n", conf.snapshot, fname.string());
    }

    conf.snapshot = fname;
    return true;
}

Serializer& operator&(Serializer& ser, Platform& platform)
{
    return (ser & static_cast<Name&>(platform));
}

}
