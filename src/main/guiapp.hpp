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
#include <string>
#include <string_view>

#include "config.hpp"
#include "types.hpp"
#include "ui_sdl2/sdl2.hpp"

#include "gui.hpp"

namespace caio {
namespace ui {
namespace sdl2 {

/**
 * GUI Application.
 */
class GuiApp : public Gui {
public:
    constexpr static const char* CONFDIR            = D_HOMECONFDIR;
    constexpr static const char* INIFILE            = "caio.ini";
    constexpr static const int64_t FPS_FAST         = 20;
    constexpr static const int64_t FPS_SLOW         = 3;
    constexpr static const int64_t FRAME_TIME_FAST  = 1'000'000 / FPS_FAST; /* us */
    constexpr static const int64_t FRAME_TIME_SLOW  = 1'000'000 / FPS_SLOW; /* us */
    constexpr static const int64_t ACTIVITY_COUNTER = 1'000'000;

    virtual ~GuiApp();

    /**
     * Start the GUI Application.
     * This method returns when the application is terminated (see event_loop()).
     * @return 0.
     * @exception UIError
     * @see event_loop()
     */
    int run();

    /**
     * Send the stop signal to the application.
     * This method returns immediatly (it does not wait for the application to terminate).
     */
    void stop() {
        _stop = true;
    }

    /**
     * Return the status of this GUI application
     * @return true if this GUI application is running; otherwise false.
     */
    bool is_running() const {
        return !_stop;
    }

    /**
     * Return the size of the main window.
     * @return A pair with the width and height values of the main window.
     */
    Size window_size() const {
        return {static_cast<float>(_width), static_cast<float>(_height)};
    }

    /**
     * Get the user configuration directory path.
     * @return The user configuration directory path.
     * @see CONFDIR
     */
    static const std::string& confdir();

    /**
     * Get the settings file full path.
     * @return The settings file full path.
     */
    static const std::string& inifile();

protected:
    /**
     * Initialise this GUI application.
     * @param title    Main window title.
     * @exception IOError
     * @exception UIError
     */
    GuiApp(const std::string& title);

private:
    using Window_uptr   = uptr_t<::SDL_Window,   void(*)(::SDL_Window*)>;
    using Renderer_uptr = uptr_t<::SDL_Renderer, void(*)(::SDL_Renderer*)>;
    using Surface_uptr  = uptr_t<::SDL_Surface,  void(*)(::SDL_Surface*)>;

    /**
     * Main event loop.
     * This method returns when the user terminates the application
     * through the GUI or when the stop() method is called.
     * @see stop()
     */
    void event_loop();

    /**
     * Render the main window.
     */
    void render_screen();

    /**
     * Render the application screen.
     */
    virtual void render() = 0;

    int                 _width{};           /* Main window width            */
    int                 _height{};          /* Main window height           */
    float               _ratio{};           /* Main window ratio            */
    std::atomic_bool    _stop{};            /* Exit the event loop          */
    bool                _guikey{};          /* ALT (COMMAND) key pressed    */
    Window_uptr         _window{{}, {}};    /* Main window                  */
    Renderer_uptr       _renderer{{}, {}};  /* Main window renderer         */
    Surface_uptr        _icon{{}, {}};      /* Main window icon             */
};

}
}
}
