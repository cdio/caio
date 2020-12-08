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

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "dispatcher.hpp"
#include "image.hpp"
#include "joystick.hpp"
#include "keyboard.hpp"
#include "rgb.hpp"
#include "types.hpp"
#include "signal.hpp"

#include "ui_config.hpp"
#include "ui_panel.hpp"


namespace cemu {
namespace ui {

/**
 * Audio Buffer.
 * An audio buffer is a self-dispatchable object.
 * @see DispatcherT
 */
using AudioBuffer = DispatcherT<samples_i16>;

/**
 * Scanline.
 * A Scanline represents a single line of an emulated screen;
 * it is filled by a video controller device with RGBa pixel data.
 * When a scanline is fully filled it must be sent to the video driver
 * to be rendered.
 */
using Scanline = std::vector<Rgba>;

/**
 * User Interface.
 * The user interface manages the application main window.
 */
class UI {
public:
    constexpr static const uint32_t CRT_COLOR      = 0x000000FF;
    constexpr static const uint32_t SCANLINE_COLOR = 0x00000080;

    virtual ~UI() {
        stop();
    }

    /**
     * If audio is enabled, start the audio stream.
     * @see Config
     * @see audio_pause()
     * @see audio_stop()
     */
    virtual void audio_play() = 0;

    /**
     * If audio is enabled, pause the audio stream.
     * Once paused it can be resumed using audio_play().
     * @see Config
     * @see audio_play()
     */
    virtual void audio_pause() = 0;

    /**
     * If audio is enabled, stop the audio stream.
     * Once stopped in can be restarted using audio_play().
     * @see Config
     * @see audio_play()
     */
    virtual void audio_stop() = 0;

    /**
     * If audio is enabled, set the volume.
     * @param vol Volume value to set (between 0.0f and 1.0f).
     * @see Config
     * @see audio_volume()
     */
    virtual void audio_volume(float vol) = 0;

    /**
     * @return The current volume if audio is enabled; 0.0f otherwise.
     * @see Config
     * @see audio_volume(float)
     */
    virtual float audio_volume() const = 0;

    /**
     * Get an audio buffer.
     * This method must be called by an emulated audio controller
     * to get a buffer to be filled with generated audio samples;
     * once the buffer is filled its dispatch() method must be
     * called in order to deliver it to the sound streaming thread
     * (if the dispatch method is not called the AudioBuffer's
     * destructor will do it anyway).
     * @return An audio buffer.
     * @see AudioBuffer
     */
    virtual AudioBuffer audio_buffer() = 0;

    /**
     * Render a scanline.
     * This method must be called by an emulated video controller to render a scanline.
     * @param line  Number of scanline to render;
     * @param sline Scanline pixel data.
     */
    virtual void render_line(unsigned line, const Scanline &sline) = 0;

    /**
     * Wait for (windowing) events and process them.
     * This method is called by the UI's main loop and it must process all
     * windowing events present in the event queue and it must refresh the main
     * window ath te configured frame rate.
     * @return false if the main window was closed by the user; true otherwise.
     * @see run()
     */
    virtual bool process_events() = 0;

    /**
     * Set the main window's title.
     * @param title The new title.
     * @exception UIError
     */
    virtual void title(const std::string &title) = 0;

    /**
     * Set the main window's icon image.
     * @param img Icon image.
     * @see Image
     */
    virtual void icon(const Image &img) = 0;

    /**
     * @return A human-readable string showing the libraries used by this user interface.
     */
    virtual std::string to_string() const = 0;

    /**
     * @return The info panel.
     * @exception UIError
     * @see ui::Panel
     */
    virtual std::shared_ptr<Panel> panel() = 0;

    /**
     * Set the emulated keyboard.
     * @param kbd Keyboard to set.
     */
    void keyboard(std::shared_ptr<Keyboard> kbd) {
        _kbd = kbd;
    }

    /**
     * Set the emulated joysticks.
     * @param il Joysticks to set.
     */
    void joystick(const std::initializer_list<std::shared_ptr<Joystick>> &il) {
        _joys = il;
    }

    /**
     * Set the hot-keys callback.
     * @param hotkey_cb Callback.
     */
    void hotkeys(const std::function<void(Keyboard::Key)> &hotkey_cb) {
        _hotkey_cb = hotkey_cb;
    }

    /**
     * @return The emulated keyboard.
     * @exception UIError if the keyboard is not set.
     */
    std::shared_ptr<Keyboard> keyboard() {
        if (!_kbd) {
            throw UIError{"Keyboard not set"};
        }

        return _kbd;
    }

    /**
     * Get an emulated joystick.
     * @param jid Joystick index.
     * @return The requested joystick on success; nullptr if the specified joystick does not exist.
     */
    std::shared_ptr<Joystick> joystick(unsigned jid) {
        if (jid < _joys.size()) {
            return _joys[jid];
        }

        return {};
    }

    /**
     * Hot-key event handler.
     * @param key The hot-key code.
     */
    void hotkey(Keyboard::Key key) {
        if (_hotkey_cb) {
            _hotkey_cb(key);
        }
    }

    /**
     * Main loop.
     * This method handles the user input and returns when the user closes
     * the main window or when any other thread calls the stop() method.
     * @exception UIError
     * @see stop()
     */
    void run();

    /**
     * Stop the main loop.
     * This method returns immediatly, it does not wait
     * for the main loop thread to leave the run() method.
     * @see run()
     */
    void stop() {
        _stop = true;
    }

    /**
     * @return true if audio is enabled; false otherwise.
     */
    bool audio_enabled() const {
        return _conf.audio.enabled;
    }

protected:
    /**
     * Initialise this user interface.
     * @param conf Configuration parameters.
     * @exception UIError
     */
    UI(const Config &conf)
        : _conf{conf} {
    }

    static void signal_handler(int signo);

    static std::atomic<Keyboard::Key> signal_key;

    Config                                 _conf{};
    std::shared_ptr<Keyboard>              _kbd{};
    std::vector<std::shared_ptr<Joystick>> _joys{};
    std::function<void(Keyboard::Key)>     _hotkey_cb{};
    std::atomic_bool                       _stop{};
};

}

using UI = ui::UI;

}
