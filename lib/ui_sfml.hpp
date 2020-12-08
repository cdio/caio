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

#include <map>
#include <memory>
#include <sstream>

#include <SFML/Graphics.hpp>

#include "types.hpp"
#include "ui.hpp"
#include "ui_audio_sfml.hpp"
#include "ui_panel_sfml.hpp"

#define UISFML_VERSION      "SFML-" CEMU_STR(SFML_VERSION_MAJOR) "." \
                                    CEMU_STR(SFML_VERSION_MINOR) "." \
                                    CEMU_STR(SFML_VERSION_PATCH)

namespace cemu {
namespace ui {
namespace sfml {

/**
 * SFML error stream.
 */
extern std::stringstream sfml_err;

/**
 * SFML user interface.
 */
class UISfml : public UI {
public:
    explicit UISfml(const Config &conf);

    virtual ~UISfml() {
    }

    /**
     * @see UI::audio_play()
     */
    void audio_play() override {
        if (audio_enabled()) {
            _audio_stream.play();
        }
    }

    /**
     * @see UI::audio_pause()
     */
    void audio_pause() override {
        if (audio_enabled()) {
            _audio_stream.pause();
        }
    }

    /**
     * @see UI::audio_stop()
     */
    void audio_stop() override {
        if (audio_enabled()) {
            _audio_stream.stop();
        }
    }

    /**
     * @see UI::audio_volume(float)
     */
    void audio_volume(float vol) override {
        if (audio_enabled()) {
            _audio_stream.setVolume((vol > 1.0f) ? 100.0f : vol * 100.0f);
        }
    }

    /**
     * @see UI::audio_volume()
     */
    float audio_volume() const override {
        return (audio_enabled() ? _audio_stream.getVolume() / 100.0f : 0.0f);
    }

    /**
     * @see UI::audio_buffer()
     */
    AudioBuffer audio_buffer() override {
        if (audio_enabled()) {
            return _audio_stream.buffer();
        }

        return {{}, {}};
    }

    /**
     * @see UI::render_line()
     */
    void render_line(unsigned line, const Scanline &sline) override;

    /**
     * @see UI::process_events()
     */
    bool process_events() override;

    /**
     * @see UI::title()
     */
    void title(const std::string &title) override {
        _window.setTitle(title);
    }

    /**
     * @see UI::icon()
     */
    void icon(const Image &img) override;

    /**
     * @see UI::to_string()
     */
    std::string to_string() const override {
        return UISFML_VERSION;
    }

    /**
     * @see UI::panel()
     */
    std::shared_ptr<Panel> panel() override {
        return _panel;
    }

    /**
     * Create a SFML user interface.
     * @param conf Configuration parameters;
     * @param icon Icon image.
     * @return The user interface using SFML as backend.
     * @exception UIError
     */
    static std::shared_ptr<UI> create(const ui::Config &conf);

private:
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
     * Window icon.
     */
    Image _icon{};

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
     * Conversion map from SFML key code to Keyboard::Key code.
     */
    static std::map<sf::Keyboard::Key, Keyboard::Key> sfml_to_key;
};

}
}
}
