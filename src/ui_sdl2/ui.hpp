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

#include "ui_sdl2/sdl2.hpp"
#include "ui_sdl2/audio.hpp"
#include "ui_sdl2/panel.hpp"
#include "ui_sdl2/widget_empty.hpp"
#include "ui_sdl2/widget_fullscreen.hpp"
#include "ui_sdl2/widget_keyboard.hpp"
#include "ui_sdl2/widget_pause.hpp"
#include "ui_sdl2/widget_photocamera.hpp"
#include "ui_sdl2/widget_reset.hpp"
#include "ui_sdl2/widget_volume.hpp"

#include "ui_config.hpp"

#include "keyboard.hpp"
#include "joystick.hpp"
#include "rgb.hpp"
#include "types.hpp"

#include <atomic>
#include <functional>
#include <semaphore>
#include <tuple>
#include <unordered_map>
#include <utility>

namespace caio {
namespace ui {
namespace sdl2 {

/**
 * Emulator User Interface.
 * The user interface implements the application main loop.
 * SDL2 is used as audio/video backend.
 */
class UI {
public:
    constexpr static const int FULLSCREEN_MIN_WIDTH         = 1024;
    constexpr static const int FULLSCREEN_MIN_HEIGHT        = 768;
    constexpr static const int REFRESH_RATE                 = 60;
    constexpr static const Rgba CRT_COLOR                   = { 0x00, 0x00, 0x00, 0xFF };
    constexpr static const Rgba SCANLINE_COLOR              = { 0x00, 0x00, 0x00, 0x80 };
    constexpr static const float SCANLINE_LUMINOSITY        = 1.2f;
    constexpr static const float ADV_SCANLINE_LUMINOSITY    = 1.2f;
    constexpr static const float ADV_SCANLINE_BRIGHT        = 0.9f;
    constexpr static const float ADV_SCANLINE_REFLECTION    = 0.4f;
    constexpr static const uint64_t MOUSE_INACTIVE_TIME     = 2'000'000;    /* us */
    constexpr static const uint8_t PANEL_BUTTON             = SDL_BUTTON_RIGHT;
    constexpr static const char* SCREENSHOT_PREFIX          = "caio_screenshot_";

    using KeyboardPtr   = sptr_t<Keyboard>;
    using JoystickPtr   = sptr_t<Joystick>;
    using HotkeysCb     = std::function<void(keyboard::Key)>;

    /**
     * Get the instance to the user interface.
     * The user interface implements the main application so there is only one instance.
     * @return The instance to the user interface.
     */
    static sptr_t<UI> instance(const Config& conf);

    virtual ~UI() noexcept;

    /**
     * Start the UI.
     * This method starts the UI and returns when the user closes the
     * main window or when some thread calls the stop() method.
     * @exception UIError
     * @see event_loop()
     * @see stop()
     * @note This method must be called by from the main thread.
     * @exception UIError
     */
    void run();

    /**
     * Set the main window's title.
     * @param title The title to set.
     */
    void title(const std::string& title);

    /**
     * Connect the emulated keyboard.
     * @param kbd Keyboard to set.
     * @see Keyboard
     */
    void keyboard(const KeyboardPtr& kbd);

    /**
     * Set the (emulated) joysticks.
     * Emulated joysticks are connected to real gamepads connected to the host system.
     * @param il List of joystick ports.
     * @see attach_controllers()
     * @see Joystick
     */
    void joystick(const std::initializer_list<JoystickPtr>& il);

    /**
     * Set the hot-keys callback.
     * @param hotkeys_cb Callback.
     */
    void hotkeys(const HotkeysCb& hotkeys_cb);

    /**
     * Set the pause callbacks.
     * @param pause_cb   Callback to pause the emulator;
     * @param ispause_cb Callback get the pause status of the emulator.
     * @see pause(bool)
     */
    void pause(const std::function<void(bool)>& pause_cb, const std::function<bool()>& ispause_cb);

    /**
     * Set the reset callback.
     * @param reset_cb Called when the user requests a reset.
     */
    void reset(const std::function<void()>& reset_cb);

    /**
     * Suspend/Resume the emulator.
     * This method works only when the pause callbacks are set.
     * @param suspend true to suspend, false to resume.
     * @see pause(const std::function<void(bool)>&, const std::function<bool()>&)
     */
    void pause(bool suspend);

    /**
     * Get the status of the emulator.
     * This method works only when the pause callbacks are set.
     * @return true if paused, false if running.
     * @see pause()
     * @see pause(const std::function<void(bool)>&, const std::function<bool()>&)
     */
    bool paused() const;

    /**
     * Get the audio status.
     * @return true if audio is enabled; false otherwise.
     */
    bool audio_enabled() const;

    /**
     * Reset the audio stream.
     * @see AudioStream::reset();
     */
    void audio_reset();

    /**
     * Stop the audio stream.
     * Once stopped in can be restarted by calling audio_reset().
     * @see AudioStream::stop();
     */
    void audio_stop();

    /**
     * Start playing audio samples.
     * @see AudioStream::play();
     */
    void audio_play();

    /**
     * Pause the audio stream.
     * Once paused it can be resumed by calling audio_play().
     * @see AudioStream::pause();
     */
    void audio_pause();

    /**
     * Set the audio volume.
     * @param vol Volume to set (between 0 and 1).
     * @see AudioStream::volume(float)
     */
    void audio_volume(float vol);

    /**
     * Get the audio volume.
     * @return The current audio volume.
     * @see AudioStream::volume() const
     */
    float audio_volume() const;

    /**
     * Get an audio buffer.
     * This method is called by an emulated audio system, it returns
     * a buffer that must be filled with PCM audio samples; once the buffer
     * is filled its dispatch() method must be called in order to deliver
     * it to the sound streaming system (if the dispatch method is not
     * called the AudioBuffer's destructor will do it anyway).
     * @return An audio buffer.
     * @see AudioStream::buffer()
     * @see AudioBuffer
     */
    AudioBuffer audio_buffer();

    /**
     * Get the index of the raw RGBA buffer ready to be rendered.
     * @return The index of the raw RGBA buffer ready to be rendered.
     */
    size_t raw_index();

    /**
     * Render a scanline.
     * This method must be called by an emulated video controller to render a scanline.
     * @param line  Number of scanline to render;
     * @param sline Scanline pixel data.
     * @return true if the last line of the screen was rendered; false otherwise.
     */
    bool render_line(unsigned line, const Scanline& sline);

    /**
     * Fill the screen with an RGBA colour.
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
     * Return a human readable string with the description
     * of the backend libraries used by this user interface.
     * @return The string describing the backend libraries.
     */
    std::string to_string() const;

    /**
     * Get a pointer to the UI panel.
     * @return A pointer to the UI panel.
     * @see Panel
     */
    sptr_t<Panel> panel();

    /**
     * Get the backend renderer.
     * @return A pointer to the backend renderer.
     */
    sptr_t<::SDL_Renderer> renderer();

    /**
     * Take a screenshot.
     * The screenshot is saved on the filesystem as a PNG file
     * with its name formatted as: SCREENSHOT_PREFIX + "YYYY-MM-DD-hh.mm.ss".
     * @exception UIError
     * @see SCREENSHOT_PREFIX
     */
    void screenshot();

private:
    /**
     * Initialise this user interface.
     * @param conf Configuration parameters.
     * @exception UIError
     */
    UI(const ui::Config& conf);

    /**
     * Initialise the SDL library.
     * @exception UIError
     */
    void init_sdl();

    /**
     * Calculate sizes of texture, emulated screen, and main
     * window as well as aspect ratio and scaling values.
     */
    void init_window_size();

    /**
     * Initialise the application main window.
     * @exception UIError
     */
    void init_window();

    /**
     * Initialise the main window renderer.
     * @exception UIError
     */
    void init_renderer();

    /**
     * Initialise the texture for the emulated screen.
     */
    void init_texture();

    /**
     * Calculate the screen aspect ratio based on configuration values.
     * @param wsize Original size of emulated screen (which defines the native aspect ratio).
     * @return The aspect ratio value.
     */
    float aspect_ratio(const Size2& wsize);

    /**
     * Resize a window to fit inside another window.
     * @param csize Size of the container window;
     * @param wsize Size of the window to resize.
     * @return The new size.
     */
    Size2 scale(const Size2& csize, const Size2& wsize);

    /**
     * Get the best display mode to contain a screen.
     * @param wsize Non-scaled screen size.
     * @return A tuple containing the new (scaled) screen size and the selected video display mode.
     * @exception IOError
     */
    std::tuple<Size2, ::SDL_DisplayMode> closest_display_mode(const Size2& wsize);

    /**
     * Get the refresh of the current video mode.
     * @return The refresh rate.
     */
    int current_refresh();

    /**
     * Initialise the fullscreen video mode.
     * Select and set the best resolution for the fullscreen mode.
     * @exception IOError
     * @see closest_display_mode(const Size2&)
     */
    void init_fs_mode();

    /**
     * Create the info panel.
     * @exception UIError
     * @see Panel
     */
    void create_panel();

    /**
     * Toogle panel visibility.
     * @see Panel
     */
    void toggle_panel_visibility();

    /**
     * Call the hotkeys callback.
     * @param key Key pressed by the user.
     */
    void hotkeys(keyboard::Key key);

    /**
     * Toggle the fullscreen mode.
     */
    void toggle_fullscreen();

    /**
     * Apply post-rendering special effects.
     */
    void postrender_effects();

    /**
     * Render the screen texture with the emulated screen data.
     * The postrender_effects() method is called after rendering.
     * @exception UIError
     * @see postrender_effects()
     */
    void render_screen();

    /**
     * Main loop.
     * This method runs an infinite loop and returns when the main window
     * is closed by the user or when the stop() method is called.
     * @exception UIError
     * @see joy_event()
     * @see kbd_event()
     * @see mouse_event()
     * @see win_event()
     * @see stop()
     * @see Panel::event(const ::SDL_Event&)
     */
    void event_loop();

    /**
     * Process window events.
     * @param event SDL event.
     */
    void win_event(const ::SDL_Event& event);

    /**
     * Process the window resize event.
     * Resize the screen to fit inside the new resized window keeping its aspect ratio.
     * @param width  New window width;
     * @param height New window height.
     */
    void resize_event(int width, int height);

    /**
     * Process keyboard events.
     * Process key-combinations and hot-keys,
     * other key-strokes are sent to emulated keyboard.
     * @param event SDL event.
     * @see hotkeys(keyboard::Key key)
     * @see keyboard(const KeyboardPtr& kbd)
     */
    void kbd_event(const ::SDL_Event& event);

    /**
     * Process game controller events.
     * Newly detected games controllers are attached to emulated joysticks;
     * already attached game controller actions are sent to their respective emulated joysticks.
     * @param event SDL event.
     */
    void joy_event(const ::SDL_Event& event);

    /**
     * Process mouse events.
     * The mouse PANEL_BUTTON toggles the visibility of the info panel.
     * @param event SDL event.
     * @see PANEL_BUTTON
     * @see Panel
     */
    void mouse_event(const ::SDL_Event& event);

    /**
     * Attach a game controller to an emulated joystick.
     * @param devid SDL game controller device index.
     */
    void joy_add(int devid);

    /**
     * Detach a game controller from an emulated joystick.
     * @param jid SDL game controller instance.
     */
    void joy_del(::SDL_JoystickID jid);

    /**
     * Get the emulated joystick attached to a game controller.
     * @param jdi Game controller id.
     * @return The requested emulated joystick on success;
     * nullptr if the specified game controller is not attached to an emulated joystick.
     */
    JoystickPtr find_joystick(::SDL_JoystickID jid);

    /**
     * Find game controllers and attach them to emulated joysticks.
     */
    void attach_controllers();

    Config                      _conf{};                    /* UI configuration                                 */
    KeyboardPtr                 _kbd{};                     /* Emulated keyboard                                */
    std::vector<JoystickPtr>    _joys{};                    /* List of emulated joysticks                       */
    HotkeysCb                   _hotkeys_cb{};              /* Hot-keys callback                                */
    std::function<void(bool)>   _pause_cb{};                /* Pause callback                                   */
    std::function<bool()>       _ispause_cb{};              /* Pause status callback                            */
    std::function<void()>       _reset_cb{};                /* Reset callback                                   */
    std::atomic_bool            _stop{};                    /* True if the main loop must be stopped            */
    std::atomic_bool            _running{};                 /* True if the main loop is running                 */
    uint64_t                    _mouse_active_time{};       /* Mouse inactive time until it becomes invisible   */
    bool                        _mouse_visible{true};       /* True if the mouse cursor is currently visible    */
    std::atomic_bool            _screenshot{};              /* True if a screenshot must be taken               */

    sptr_t<::SDL_Window>        _window{};                  /* Main window                                      */
    uptrd_t<::SDL_Surface>      _icon{nullptr, nullptr};    /* Main window icon                                 */
    sptr_t<::SDL_Renderer>      _renderer{};                /* Main window renderer                             */
    sptr_t<::SDL_Texture>       _screen_tex{};              /* Screen texture to render                         */

    Size2                       _win_size{};                /* Size of the main window                          */
    Size2                       _win_pos{};                 /* Position of the main window on desktop           */
    Size2                       _fs_size{};                 /* Size of the fullscreen window                    */
    uint32_t                    _fs_flags{};                /* Fullscreen flags                                 */
    bool                        _is_maximized{};            /* True if the main window is maximized             */
    bool                        _is_fullscreen{};           /* True if fullscreen mode is active                */

    Size2                       _screen_size{};             /* Size of the screen                               */
    Size2                       _tex_size{};                /* Size of the screen texture                       */
    float                       _screen_ratio{};            /* Screen aspect ratio                              */
    float                       _screen_scale{};            /* Screen scale factor                              */
    ::SDL_Rect                  _screen_rect{};             /* Screen position inside the main window           */
    ::SDL_Rect                  _screen_fs_rect{};          /* Screen position inside the fullscreen window     */
    Scanline                    _screen_raw[2]{};           /* Screen RGBA data buffers                         */
    size_t                      _raw_index{};               /* Index of screen buffer being filled              */
    std::binary_semaphore       _raw_sem{0};                /* GUI/clock frame synchronisation semaphore        */

    sptr_t<Panel>               _panel{};                   /* Info Panel                                       */
    sptr_t<widget::Fullscreen>  _wid_fullscreen{};          /* Fullscreen button panel widget                   */
    sptr_t<widget::Reset>       _wid_reset{};               /* Reset button panel widget                        */
    sptr_t<widget::Pause>       _wid_pause{};               /* Puase button panel widget                        */
    sptr_t<widget::Volume>      _wid_volume{};              /* Volume control panel widget                      */
    sptr_t<widget::Keyboard>    _wid_keyboard{};            /* Keyboard button panel widget                     */
    sptr_t<widget::PhotoCamera> _wid_photocamera{};         /* Screenshot button panel widget                   */

    AudioStream                 _audio_stream{};            /* Audio driver                                     */

    std::unordered_map<::SDL_JoystickID, uptrd_t<::SDL_GameController>> _sdl_joys{};
                                                            /* Game controllers attached to emulated joysticks  */
};

}
}
}
