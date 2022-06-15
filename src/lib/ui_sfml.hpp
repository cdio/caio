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

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

#include "image.hpp"
#include "joystick.hpp"
#include "keyboard.hpp"
#include "rgb.hpp"
#include "types.hpp"

#include "ui_config.hpp"
#include "ui_sfml_audio.hpp"
#include "ui_sfml_panel.hpp"


namespace caio {
namespace ui {
namespace sfml {

/**
 * SFML error stream.
 */
extern std::stringstream sfml_err;


/**
 * @return The SFML library version.
 */
constexpr const char *sfml_version()
{
    return "SFML-" CAIO_STR(SFML_VERSION_MAJOR) "." \
                   CAIO_STR(SFML_VERSION_MINOR) "." \
                   CAIO_STR(SFML_VERSION_PATCH);
}


/**
 * Scanline.
 * A Scanline represents a single line of an emulated screen;
 * it is filled by a video controller device with RGBa pixel data.
 * When a scanline is fully filled it must be sent to the user interface
 * to be rendered.
 */
using Scanline = std::vector<Rgba>;


/**
 * SFML user interface.
 */
class UISfml {
public:
    constexpr static const uint32_t CRT_COLOR           = 0x000000FF;
    constexpr static const uint32_t SCANLINE_COLOR      = 0x00000080;

    constexpr static const uint64_t MOUSE_INACTIVE_TIME = 2'000'000;    /* us */


    explicit UISfml(const Config &conf);

    virtual ~UISfml() {
        stop();
    }

    /**
     * Set the main window's title.
     * @param title The new title.
     * @exception UIError
     */
    void title(const std::string &title) {
        _window.setTitle(title);
    }

    /**
     * Set the emulated keyboard.
     * @param kbd Keyboard to set.
     */
    void keyboard(std::shared_ptr<Keyboard> kbd) {
        _kbd = kbd;
    }

    /**
     * Set the emulated joysticks and associate connected gamepads to them.
     * @param il Joysticks to set.
     */
    void joystick(const std::initializer_list<std::shared_ptr<Joystick>> &il);

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
     * Hot-key event handler.
     * @param key The hot-key code.
     */
    void hotkey(Keyboard::Key key) {
        if (_hotkey_cb) {
            _hotkey_cb(key);
        }
    }

    /**
     * If audio is enabled, start the audio stream.
     * @see Config
     * @see audio_pause()
     * @see audio_stop()
     */
    void audio_play() {
        if (audio_enabled()) {
            _audio_stream.play();
        }
    }

    /**
     * If audio is enabled, pause the audio stream.
     * Once paused it can be resumed using audio_play().
     * @see Config
     * @see audio_play()
     */
    void audio_pause() {
        if (audio_enabled()) {
            _audio_stream.pause();
        }
    }

    /**
     * If audio is enabled, stop the audio stream.
     * Once stopped in can be restarted using audio_play().
     * @see Config
     * @see audio_play()
     */
    void audio_stop() {
        if (audio_enabled()) {
            _audio_stream.stop();
        }
    }

    /**
     * If audio is enabled, set the volume.
     * @param vol Volume value to set (between 0.0f and 1.0f).
     * @see Config
     * @see audio_volume()
     */
    void audio_volume(float vol) {
        if (audio_enabled()) {
            _audio_stream.setVolume((vol > 1.0f) ? 100.0f : vol * 100.0f);
        }
    }

    /**
     * @return The current volume if audio is enabled; 0.0f otherwise.
     * @see Config
     * @see audio_volume(float)
     */
    float audio_volume() const {
        return (audio_enabled() ? _audio_stream.getVolume() / 100.0f : 0.0f);
    }

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
    AudioBuffer audio_buffer() {
        if (audio_enabled()) {
            return _audio_stream.buffer();
        }

        return {{}, {}};
    }

    /**
     * @return true if audio is enabled; false otherwise.
     */
    bool audio_enabled() const {
        return _conf.audio.enabled;
    }

    /**
     * Render a scanline.
     * This method must be called by an emulated video controller to render a scanline.
     * @param line  Number of scanline to render;
     * @param sline Scanline pixel data.
     */
    void render_line(unsigned line, const Scanline &sline);

    /**
     * @return The info panel.
     * @exception UIError
     * @see ui::Panel
     */
    std::shared_ptr<PanelSfml> panel() {
        return _panel;
    }

    /**
     * Main loop.
     * This method starts the UI and returns when the user closes the
     * main window or when any other thread calls the stop() method.
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
     * @return A human-readable string showing the libraries used by this user interface.
     */
    std::string to_string() const {
        return sfml_version();
    }

private:
    /**
     * Process windowing events.
     * This method runs in an infinite loop and returns when
     * the main window is closed or the stop() method is called.
     * @see run()
     */
    void process_events();

    /**
     * Get an emulated joystick.
     * @param jid Joystick index.
     * @return The requested joystick on success; nullptr if the specified joystick does not exist.
     */
    std::shared_ptr<Joystick> joystick(unsigned jid);

    /**
     * Create the info panel.
     * Create the info panel and add the ui widgets.
     */
    void create_panel();

    /**
     * Detect a panel coordinates.
     * @param x Coordinate x;
     * @param y Coordinate y.
     * @return True if the specified cooridnates belongs to the panel area; false otherwise.
     */
    bool is_panel_area(unsigned x, unsigned y);

    /**
     * Render and display the main window.
     * @see render_screen()
     */
    void render_window();

    /**
     * Render the emulated screen.
     * @return A sprite containing the emulated screen.
     * @see _screen_tex
     * @see render_line()
     */
    sf::Sprite render_screen();

    /**
     * Toggle the fullscreen mode.
     */
    void toggle_fullscreen();

    /**
     * Toggle panel visibility.
     */
    void toggle_panel_visibility();

    /**
     * Process the window resize event.
     * Adapt the video renderer to the new size.
     * @param width  New main window width;
     * @param height New main window height.
     */
    void resize_event(unsigned width, unsigned height);

    /**
     * Keyboard event handler.
     * @param event SFML keyboard event.
     */
    void kbd_event(const sf::Event &event);

    /**
     * Joystick event handler.
     * @param event SFML joystick event.
     */
    void joy_event(const sf::Event &event);


    /**
     * Convert a SFML key code to Keyboard::Key code.
     * @param key SFML Key code.
     * @return Keyboard::Key code.
     */
    static Keyboard::Key to_key(const sf::Keyboard::Key &key);

    /**
     * Get the size of the main window based on the size an the emulated screen and the visibilty of an info panel.
     * @param panel_visible True if the panel is visible, false otherwise:
     * @param screen_size   Emulated screen size.
     * @return The main window size.
     */
    static sf::Vector2u window_size(bool panel_visible, const sf::Vector2u &screen_size);


    /**
     * UI configuration.
     */
    Config _conf{};

    /**
     * True if the main loop must be stopped.
     */
    std::atomic_bool _stop{};

    /**
     * Emulated keyboard.
     */
    std::shared_ptr<Keyboard> _kbd{};

    /**
     * List of emulated joysticks.
     */
    std::vector<std::shared_ptr<Joystick>> _joys{};

    /**
     * User defined hot-keys callback.
     */
    std::function<void(Keyboard::Key)> _hotkey_cb{};

    /**
     * Video mode.
     */
    sf::VideoMode _desktop_mode{};

    /**
     * Main window size.
     */
    sf::Vector2u _win_size{};

    /**
     * Saved main window size (set before moving to fullscreen mode).
     */
    sf::Vector2u _saved_win_size{};

    /**
     * Saved main window position (set before moving to fullscreen mode).
     */
    sf::Vector2i _saved_win_pos{};

    /**
     * Screen texture size.
     * Size of the rendered emulated screen.
     */
    sf::Vector2u _screen_size{};

    /**
     * Emulated screen aspect ratio.
     * Fixed value.
     */
    float _screen_ratio{};

    /**
     * Scale factor.
     * This value is set from configuration but changes when the main window is resized.
     */
    sf::Vector2f _scale{};

    /**
     * Info panel.
     */
    std::shared_ptr<PanelSfml> _panel{};

    /**
     * Fullscreen mode.
     */
    bool _is_fullscreen{};

    /**
     * Main window.
     */
    sf::RenderWindow _window{};

    /**
     * View Port.
     * Support for resizing events.
     */
    sf::View _view{};

    /**
     * Window render texture.
     * This texture is rendered into the main window at frame rate speed.
     */
    sf::RenderTexture _render_tex{};

    /**
     * Emulated screen pixel data.
     * @see _screen_tex
     */
    std::vector<Rgba> _screen_raw{};

    /**
     * Emulated screen texture.
     * This texture is updated with new pixel data at frame rate speed.
     * @see _screen_raw
     */
    sf::Texture _screen_tex{};

    /**
     * Texture for the scanline effect.
     */
    sf::Texture _scanline_tex{};

    /**
     * Audio output stream.
     */
    AudioStream _audio_stream{};

    /**
     * Work-around for the keyboard handling deficiency of SFML.
     * Used to handle key codes missing in sf::Keyboard::Key.
     */
    bool _unknown_key_pressed{};
    Keyboard::Key _unknown_key{Keyboard::KEY_NONE};

    /**
     * Mouse activity time.
     * If this time is greater than MOUSE_INACTIVE_TIME the mouse cursor is hidden.
     * @see MOUSE_INACTIVE_TIME
     */
    uint64_t _mouse_active_time{};
    bool _mouse_visible{true};


    /**
     * Conversion map from SFML key code to Keyboard::Key code.
     */
    static std::map<sf::Keyboard::Key, Keyboard::Key> sfml_to_key;
};

}
}
}
