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
#include "guiapp.hpp"

#include <sys/wait.h>
#include <csignal>
#include <filesystem>

#include "endian.hpp"
#include "fs.hpp"
#include "icon.hpp"
#include "utils.hpp"

namespace caio {
namespace ui {
namespace sdl2 {

static std::atomic_bool quit_signal{};

static void signal_handler(int signo)
{
    switch (signo) {
    case SIGTERM:
    case SIGQUIT:
        quit_signal = true;
        break;
    case SIGCHLD:
        ::wait3(nullptr, WNOHANG, nullptr);
    default:;
    }
}

GuiApp::GuiApp(const std::string& title)
{
    /*
     * Configuration directory.
     */
    const auto cdir = confdir();
    std::error_code ec{};
    std::filesystem::create_directories(cdir, ec);
    if (ec) {
        throw IOError{"Can't create configuration directory: {}: {}. Error Ignored\n", cdir, ec.message()};
    }

    /*
     * SDL initialization.
     */
    if (::SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0 ||
        ::IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        throw UIError{"Can't initialise SDL library: {}", sdl_error()};
    }

    _ratio = 1.5f;
    _width = Gui::FONT_SIZE * 130;
    _height = _width / _ratio;

    _window = Window_uptr{::SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        _width, _height, SDL_WINDOW_RESIZABLE), ::SDL_DestroyWindow};

    if (!_window) {
        throw UIError{"Can't create main window: {}", sdl_error()};
    }

    _renderer = Renderer_uptr{::SDL_CreateRenderer(_window.get(), -1, 0), ::SDL_DestroyRenderer};
    if (!_renderer) {
        throw UIError{"Can't create renderer: {}", sdl_error()};
    }

    if (::SDL_SetRenderDrawBlendMode(_renderer.get(), SDL_BLENDMODE_BLEND) < 0) {
        throw UIError{"Can't set renderer blend mode: {}", sdl_error()};
    }

    const Image& ico = icon();

#ifdef __LITTLE_ENDIAN__
    _icon = Surface_uptr{::SDL_CreateRGBSurfaceWithFormatFrom(const_cast<Rgba*>(ico.data.data()),
        ico.width, ico.height, 32, ico.width * 4, SDL_PIXELFORMAT_ABGR8888), ::SDL_FreeSurface};
#else
    _icon = Surface_uptr{::SDL_CreateRGBSurfaceWithFormatFrom(const_cast<Rgba*>(ico.data.data()),
        ico.width, ico.height, 32, ico.width * 4, SDL_PIXELFORMAT_RGBA8888), ::SDL_FreeSurface};
#endif

    if (!_icon) {
        throw UIError{"Can't create main window icon: {}", sdl_error()};
    }

    ::SDL_SetWindowIcon(_window.get(), _icon.get());

    Gui::init(inifile(), Gui::FONT_SCALE, _window.get(), _renderer.get());
}

GuiApp::~GuiApp()
{
    Gui::release();
    _window.reset();
    _renderer.reset();
    _icon.reset();
    ::IMG_Quit();
    ::SDL_Quit();
}

int GuiApp::run()
{
    auto prev_term = std::signal(SIGTERM, signal_handler);
    auto prev_quit = std::signal(SIGQUIT, signal_handler);
    auto prev_chld = std::signal(SIGCHLD, signal_handler);
    if (prev_term == SIG_ERR || prev_quit == SIG_ERR || prev_chld == SIG_ERR) {
        throw Error{"Can't set signal handler: {}\n", Error::to_string()};
    }

    event_loop();

    std::signal(SIGCHLD, prev_chld);
    std::signal(SIGQUIT, prev_quit);
    std::signal(SIGTERM, prev_term);
    return 0;
}

void GuiApp::event_loop()
{
    ::SDL_Event event{};
    int64_t start{};
    int64_t activity_counter{};

    while (is_running()) {
        start = utils::now() - start;

        while (::SDL_PollEvent(&event)) {
            Gui::process_event(event);

            switch (event.type) {
            case SDL_QUIT:
                stop();
                return;

            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                    _width = event.window.data1;
                    _height = event.window.data2;
                    break;
                default:;
                }
                break;

            case SDL_KEYUP:
                _guikey = false;
                break;

            case SDL_KEYDOWN:
#ifdef __APPLE__
                _guikey = (event.key.keysym.mod & KMOD_GUI);
#else
                _guikey = (event.key.keysym.mod & KMOD_ALT);
#endif
                if (_guikey) {
                    /*
                     * Handle ALT-'+' and ALT-'-'
                     */
                    switch (event.key.keysym.sym) {
                    case SDLK_EQUALS:
                        process_font_incdec(true);
                        break;
                    case SDLK_MINUS:
                        process_font_incdec(false);
                        break;
                    default:;
                    }
                }
                break;

            case SDL_MOUSEWHEEL:
                if (_guikey) {
                    /*
                     * Handle ALT+mouse wheel up/down
                     */
                    if (event.wheel.y != 0) {
                        process_font_incdec(event.wheel.y > 0);
                    }
                }
                break;

            default:;
            }

            activity_counter = ACTIVITY_COUNTER;
        }

        if (quit_signal) {
            stop();
        }

        render_screen();

        /*
         * Immediate mode GUIs are very CPU consuming, minimize
         * it with a varying frame rate based on the user activity.
         */
        if (activity_counter > 0) {
            activity_counter -= FRAME_TIME_FAST;
        }

        int64_t delay = (activity_counter > 0 ? FRAME_TIME_FAST : FRAME_TIME_SLOW) - utils::now() + start;
        start = (delay > 0 ? utils::sleep(delay) - delay : 0);
    }
}

void GuiApp::render_screen()
{
    pre_render();
    render();
    post_render();
}

const std::string& GuiApp::confdir()
{
    static std::string cdir = fs::fix_home(CONFDIR);
    return cdir;
}

const std::string& GuiApp::inifile()
{
    static std::string ini = std::format("{}/{}", confdir(), INIFILE);
    return ini;
}

}
}
}
