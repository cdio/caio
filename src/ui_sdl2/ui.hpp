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

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "keyboard.hpp"
#include "joystick.hpp"
#include "rgb.hpp"
#include "types.hpp"

#include "ui_config.hpp"

#include "ui_sdl2/sdl2.hpp"
#include "ui_sdl2/audio.hpp"
#include "ui_sdl2/panel.hpp"

namespace caio {
namespace ui {
namespace sdl2 {

/**
 * SDL2 User Interface.
 */
class UI {
public:
    constexpr static const Rgba CRT_COLOR                   = { 0x00, 0x00, 0x00, 0xFF };
    constexpr static const Rgba SCANLINE_COLOR              = { 0x00, 0x00, 0x00, 0x80 };
    constexpr static const float SCANLINE_LUMINOSITY        = 1.2f;
    constexpr static const float ADV_SCANLINE_LUMINOSITY    = 1.2f;
    constexpr static const float ADV_SCANLINE_BRIGHT        = 0.9f;
    constexpr static const float ADV_SCANLINE_REFLECTION    = 0.4f;
    constexpr static const uint64_t MOUSE_INACTIVE_TIME     = 2'000'000;    /* us */
    constexpr static const uint8_t PANEL_BUTTON             = SDL_BUTTON_RIGHT;

    using keybptr_t     = sptr_t<Keyboard>;
    using joyptr_t      = sptr_t<Joystick>;
    using hotkeys_cb_t  = std::function<void(keyboard::Key)>;

    /**
     * Get the instance to the user interface.
     * @return The only instance to the user interface.
     */
    static sptr_t<UI> instance(const Config& conf);

    virtual ~UI();

    /**
     * Main loop.
     * This method starts the UI and returns when the user closes the
     * main window or when any other thread calls the stop() method.
     * @exception UIError
     * @see process_events()
     * @see stop()
     * @note Method must run on the main thread.
     */
    void run();

    /**
     * Set the main window's title.
     * @param title The new title.
     */
    void title(const std::string& title);

    /**
     * Set the emulated keyboard.
     * @param kbd Keyboard to set.
     */
    void keyboard(const keybptr_t& kbd);

    /**
     * Set the emulated joysticks and associate connected gamepads to them.
     * @param il Joysticks to set.
     */
    void joystick(const std::initializer_list<joyptr_t>& il);

    /**
     * Set the hot-keys callback.
     * @param hotkeys_cb Callback.
     */
    void hotkeys(const hotkeys_cb_t& hotkeys_cb);

    /**
     * Set the pause callbacks.
     * @param pause_cb   Callback to pause the emulator;
     * @param ispause_cb Callback get the pause status of the emulator.
     */
    void pause(const std::function<void(bool)>& pause_cb, const std::function<bool()>& ispause_cb);

    /**
     * Set the reset callback.
     * @param reset_cb Called when the user requests a reset.
     */
    void reset(const std::function<void()>& reset_cb);

    /**
     * Suspend/Resume emulator.
     * This only works if the pause callbacks are set.
     * @param suspend true to suspend, false to resume.
     * @see pause(const std::function<void(bool)>&, const std::function<bool()>&)
     */
    void pause(bool suspend);

    /**
     * Get the suspend status of the emulator.
     * This only works if the pause callbacks are set.
     * @return true if suspended, false if running.
     * @see pause()
     */
    bool paused() const;

    /**
     * Get the audio status.
     * @return true if audio is enabled; false otherwise.
     */
    bool audio_enabled() const;

    /**
     * Reset the audio driver.
     */
    void audio_reset();

    /**
     * Stop the driver.
     * Once stopped in can be restarted using audio_reset().
     * @see audio_reset()
     */
    void audio_stop();

    /**
     * Start playing audio samples.
     * @see audio_pause()
     */
    void audio_play();

    /**
     * Pause the audio stream.
     * Once paused it can be resumed using audio_play().
     * @see audio_play()
     */
    void audio_pause();

    /**
     * Set the audio volume.
     * @param vol Volume to set (between 0 and 1).
     * @see audio_volume()
     */
    void audio_volume(float vol);

    /**
     * Get the audio volume.
     * @return The current audio volume.
     * @see audio_volume(float)
     */
    float audio_volume() const;

    /**
     * Get an audio buffer.
     * This method must be called by an emulated system to get a buffer
     * to be filled with generated audio samples; once the buffer is
     * filled its dispatch() method must be called in order to deliver
     * it to the sound streaming subsystem (if the dispatch method is
     * not called the AudioBuffer's destructor will do it anyway).
     * @return An audio buffer.
     * @see AudioBuffer
     */
    AudioBuffer audio_buffer();

    /**
     * Render a scanline.
     * This method must be called by an emulated video controller to render a scanline.
     * @param line  Number of scanline to render;
     * @param sline Scanline pixel data.
     */
    void render_line(unsigned line, const Scanline& sline);

    /**
     * Fill the screen with a colour.
     * @param color Colour (default is CRT_COLOR).
     * @see CRT_COLOR
     */
    void clear_screen(Rgba color = CRT_COLOR);

    /**
     * Stop the main loop.
     * This method returns immediatly, it does not wait
     * for the main loop thread to leave the run() method.
     * @see run()
     */
    void stop();

    /**
     * Return a human readable string describing the backend libraries used by this user interface.
     * @return The string showing the libraries used by this user interface.
     */
    std::string to_string() const;

    /**
     * Get a pointer to the UI panel.
     * @return A pointer to the UI panel.
     */
    sptr_t<Panel> panel();

    /**
     * Get the backend renderer.
     * @return A raw pointer to the backend renderer.
     */
    SDL_Renderer* renderer();

private:
    /**
     * Initialise this user interface.
     * @param conf Configuration parameters.
     * @exception UIError
     */
    UI(const ui::Config& conf);

    /**
     * Set the size of the screen based on the configured scanline effect.
     * @param scale The screen scale.
     * @see _screen_width
     * @see _screen_height
     */
    void screen_sizes(float scale);

    /**
     * Apply post-rendering special effects.
     */
    void postrender_effects();

    /**
     * Create the info panel.
     * @exception UIError
     */
    void create_panel();

    /**
     * Toogle panel visibility.
     */
    void toggle_panel_visibility();

    /**
     * Call the hotkeys callback with the specified key.
     * @param key Key.
     */
    void hotkeys(keyboard::Key key);

    /**
     * Toggle the fullscreen mode.
     */
    void toggle_fullscreen();

    /**
     * Process a window resize event.
     * @param width  New window size;
     * @param height New window height.
     */
    void resize(int width, int height);

    /**
     * Render the screen texture with the emulated screen data.
     * @see _screen_tex
     * @see render_line()
     */
    void render_screen();

    /**
     * Process SDL events.
     * This method runs in an infinite loop and returns when
     * the main window is closed or the stop() method is called.
     * @see win_event()
     * @see kbd_event()
     * @see gamepad_event()
     * @see mouse_event()
     */
    void event_loop();

    /**
     * Process window events.
     * @param event SDL event.
     */
    void win_event(const SDL_Event& event);

    /**
     * Process keyboard events.
     * @param event SDL event.
     */
    void kbd_event(const SDL_Event& event);

    /**
     * Process joystick/game controller events.
     * @param event SDL event.
     */
    void joy_event(const SDL_Event& event);

    /**
     * Process mouse events.
     * @param event SDL event.
     */
    void mouse_event(const SDL_Event& event);

    /**
     * Associate a new game controller to an emulated joystick.
     * @param devid SDL game controller device index.
     */
    void joy_add(int devid);

    /**
     * Dis-associate a game controller from an emulated joystick.
     * @param jid SDL game controller instance.
     */
    void joy_del(SDL_JoystickID jid);

    /**
     * Find an emulated joystick given its identifier.
     * @param jid SDL game controller instance.
     * @return The requested joystick on success; nullptr if the specified joystick is not found.
     */
    joyptr_t find_joystick(SDL_JoystickID jid);

    Config                      _conf{};                /* UI configuration                                   */
    uint64_t                    _fps_time{};            /* FPS in microseconds                                */
    keybptr_t                   _kbd{};                 /* Emulated keyboard                                  */
    std::vector<joyptr_t>       _joys{};                /* List of emulated joysticks                         */
    hotkeys_cb_t                _hotkeys_cb{};          /* User defined hot-keys callback                     */
    std::function<void(bool)>   _pause_cb{};            /* Pause callback                                     */
    std::function<bool()>       _ispause_cb{};          /* Pause status callback                              */
    std::function<void()>       _reset_cb{};            /* Reset callback                                     */

    int                         _win_width{};           /* Width of the main window                           */
    int                         _win_height{};          /* Height of the main window                          */
    int                         _tex_width{};           /* Width of the emulated screen                       */
    int                         _tex_height{};          /* Height of the emulated screen                      */
    int                         _screen_width{};        /* Width of the scaled emulated screen                */
    int                         _screen_height{};       /* Height of the scaled emulated screen               */
    float                       _screen_ratio{};        /* Aspect ratio of the emulated screen                */
    float                       _screen_scale{};        /* Current screen scale factor                        */
    bool                        _is_fullscreen{};       /* Fullscreen mode                                    */
    std::atomic_bool            _stop{};                /* True if the main loop must be stopped              */
    uint64_t                    _mouse_active_time{};   /* Inactivity time until the cursor becomes invisible */
    bool                        _mouse_visible{true};   /* Whether the mouse cursor is visible or not         */

    SDL_Window*                 _window{nullptr};       /* Main window                                        */
    SDL_Renderer*               _renderer{nullptr};     /* Main window renderer                               */
    SDL_Surface*                _icon{nullptr};         /* Main window icon                                   */
    std::vector<Rgba>           _screen_raw{};          /* Emulated screen raw RGBA data                      */
    SDL_Texture*                _screen_tex{nullptr};   /* Emulated screen texture ready to be rendered       */
    SDL_Rect                    _screen_rect{};         /* Emulated screen rendering coordinates              */
    sptr_t<Panel>               _panel{};               /* Info Panel                                         */

    AudioStream                 _audio_stream{};        /* Audio driver                                       */

    std::map<SDL_JoystickID, SDL_Joystick*> _sdl_joys{};    /* Map of SDL detected joysticks                  */
};

}
}
}
