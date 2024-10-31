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
#include "ui_sdl2/ui.hpp"

#include <algorithm>
#include <cmath>
#include <csignal>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <thread>

#include <SDL_image.h>
#include <SDL_ttf.h>

#include "endian.hpp"
#include "icon.hpp"
#include "logger.hpp"
#include "utils.hpp"

namespace caio {
namespace ui {
namespace sdl2 {

static std::atomic<keyboard::Key> signal_key{keyboard::KEY_NONE};

static void signal_handler(int signo)
{
    switch (signo) {
    case SIGSEGV:
        std::cout.flush();
        log.fatal("caio: Segmentation Fault\n{}\n", stacktrace());
        /* NOTREACHED */
        break;

    case SIGABRT:
        std::cout.flush();
        log.fatal("caio: Abort\n{}\n", stacktrace());
        /* NOTREACHED */
        break;

    case SIGINT:
        signal_key = keyboard::KEY_CTRL_C;
        break;

    default:;
    }
}

sptr_t<UI> UI::instance(const ui::Config& conf)
{
    static sptr_t<UI>gui{};
    if (!gui) {
        gui = sptr_t<UI>(new UI{conf});
    }

    return gui;
}

UI::UI(const ui::Config& conf)
    : _conf{conf}
{
    if (::SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0 ||
        ::IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG ||
        ::TTF_Init() != 0) {
        throw UIError{"Can't initialise SDL library: {}", sdl_error()};
    }

    auto& vconf = _conf.video;

    _fps_time = 1'000'000 / vconf.fps;
    _screen_ratio = vconf.width / static_cast<float>(vconf.height);

    switch (vconf.sleffect) {
    case SLEffect::Adv_Horizontal:
        /*
         * In the advanced horizontal scanlines effect a new "empty"
         * scanline is intercalated between two valid scanlines.
         */
        _tex_width = vconf.width;
        _tex_height = 2 * vconf.height - 1;
        break;
    case SLEffect::Adv_Vertical:
        /*
         * In the advanced vertical scanlines effect a new "empty"
         * vertical line is intercalated between two valid vertical lines.
         */
        _tex_width = 2 * vconf.width - 1;
        _tex_height = vconf.height;
        break;
    default:
        _tex_width = vconf.width;
        _tex_height = vconf.height;
    }

    screen_sizes(vconf.scale);

    _screen_rect = {
        .x = 0,
        .y = 0,
        .w = _screen_width,
        .h = _screen_height
    };

    _win_width = _screen_width;
    _win_height = _screen_height;

    switch (vconf.sleffect) {
    case SLEffect::Adv_Horizontal:
    case SLEffect::Adv_Vertical:
        if (vconf.sresize) {
            log.warn("Smooth resize cannot be used with advanced scanlines effects: Smooth resize disabled\n");
            vconf.sresize = false;
        }
    default:;
    }

    /*
     * Create the main window.
     */
    auto* windowp = ::SDL_CreateWindow(_conf.video.title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        _win_width, _win_height, SDL_WINDOW_RESIZABLE);

    if (windowp == nullptr) {
        throw UIError{"Can't create main window: {}", sdl_error()};
    }

    _window = sptr_t<::SDL_Window>{windowp, ::SDL_DestroyWindow};

    /*
     * Create the main window icon.
     */
    const Image& ico = icon();

#if __BYTE_ORDER == __LITTLE_ENDIAN
    auto* iconp = ::SDL_CreateRGBSurfaceWithFormatFrom(const_cast<Rgba*>(ico.data.data()),
        ico.width, ico.height, 32, ico.width * 4, SDL_PIXELFORMAT_ABGR8888);
#else
    auto* iconp = ::SDL_CreateRGBSurfaceWithFormatFrom(const_cast<Rgba*>(ico.data.data()),
        ico.width, ico.height, 32, ico.width * 4, SDL_PIXELFORMAT_RGBA8888);
#endif

    if (iconp == nullptr) {
        throw UIError{"Can't create window icon: {}", sdl_error()};
    }

    _icon = uptrd_t<::SDL_Surface>{iconp, ::SDL_FreeSurface};
    ::SDL_SetWindowIcon(_window.get(), _icon.get());

    /*
     * Create the renderer.
     */
    auto* rendp = ::SDL_CreateRenderer(_window.get(), -1, 0);
    if (rendp == nullptr) {
        throw UIError{"Can't create renderer: {}", sdl_error()};
    }

    _renderer = sptr_t<::SDL_Renderer>{rendp, ::SDL_DestroyRenderer};

    if (::SDL_SetRenderDrawBlendMode(_renderer.get(), SDL_BLENDMODE_BLEND) < 0) {
        throw UIError{"Can't set renderer blend mode: {}", sdl_error()};
    }

    /*
     * Create the raw (rgba) screen (this is filled with the emulated video data).
     */
    _screen_raw = std::vector<Rgba>(vconf.width * vconf.height, CRT_COLOR);

    /*
     * Create the raw screen texture.
     */
    auto* screenp = ::SDL_CreateTexture(_renderer.get(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
        _tex_width, _tex_height);

    if (screenp == nullptr) {
        throw UIError{"Can't create screen texture: {}", sdl_error()};
    }

    _screen_tex = sptr_t<::SDL_Texture>{screenp, ::SDL_DestroyTexture};

    /*
     * Set fullscreen/normal mode.
     */
    _is_fullscreen = false;
    if (vconf.fullscreen) {
        toggle_fullscreen();
    }

    /*
     * Initialise audio system and create the info panel.
     */
    audio_reset();
    create_panel();
}

UI::~UI()
{
    stop();

    while (_running) {
        std::this_thread::yield();
    }

    _panel = {};

    ::IMG_Quit();
    ::SDL_Quit();
}

void UI::title(const std::string& title)
{
    if (_window) {
        ::SDL_SetWindowTitle(_window.get(), title.c_str());
    }
}

void UI::keyboard(const keybptr_t& kbd)
{
    _kbd = kbd;
}

void UI::joystick(const std::initializer_list<joyptr_t>& il)
{
    _joys = il;
}

void UI::hotkeys(const hotkeys_cb_t& hotkeys_cb)
{
    _hotkeys_cb = hotkeys_cb;
}

void UI::hotkeys(keyboard::Key key)
{
    if (_hotkeys_cb) {
        _hotkeys_cb(key);
    }
}

void UI::pause(const std::function<void(bool)>& pause_cb, const std::function<bool()>& ispause_cb)
{
    _pause_cb = pause_cb;
    _ispause_cb = ispause_cb;
}

void UI::pause(bool suspend)
{
    if (_pause_cb) {
        _pause_cb(suspend);
        title(_conf.video.title + (suspend ? " (PAUSED)" : ""));
    }
}

bool UI::paused() const
{
    if (_ispause_cb) {
        return _ispause_cb();
    }

    return false;
}

void UI::reset(const std::function<void()>& reset_cb)
{
    _reset_cb = reset_cb;
}

bool UI::audio_enabled() const
{
    return _conf.audio.enabled;
}

void UI::audio_reset()
{
    if (audio_enabled()) {
        _audio_stream.reset(_conf.audio);
    }
}

void UI::audio_stop()
{
    if (audio_enabled()) {
        _audio_stream.stop();
    }
}

void UI::audio_play()
{
    if (audio_enabled()) {
        _audio_stream.play();
    }
}

void UI::audio_pause()
{
    if (audio_enabled()) {
        _audio_stream.pause();
    }
}

void UI::audio_volume(float vol)
{
    if (audio_enabled()) {
        _audio_stream.volume(vol);
    }
}

float UI::audio_volume() const
{
    return (audio_enabled() ? _audio_stream.volume() : 0.0f);
}

AudioBuffer UI::audio_buffer()
{
    if (audio_enabled()) {
        return _audio_stream.buffer();
    }

    return {{}, {}};
}

void UI::render_line(unsigned line, const Scanline& sline)
{
    if (_stop) {
//        log.debug("ui: Can't render line: System is stopped\n");
        return;
    }

    if (line >= _conf.video.height || sline.size() != _conf.video.width) {
        log.warn("ui: Can't render line: Invalid raster line {}, size {}. Ignored\n", line, sline.size());
        return;
    }

    std::copy(sline.begin(), sline.end(), _screen_raw.begin() + line * _conf.video.width);
}

void UI::clear_screen(Rgba color)
{
    std::fill(_screen_raw.begin(), _screen_raw.end(), color);
}

void UI::stop()
{
    _stop = true;
}

std::string UI::to_string() const
{
    return sdl_version();
}

sptr_t<Panel> UI::panel()
{
    return _panel;
}

sptr_t<::SDL_Renderer> UI::renderer()
{
    return _renderer;
}

void UI::create_panel()
{
    /*
     * Info panel.
     */
    _panel = std::make_shared<Panel>(_renderer, _conf.video.statusbar);

    /*
     * Fullscreen widget.
     */
    _wid_fullscreen = std::make_shared<widget::Fullscreen>(_renderer, [this]() {
        return _is_fullscreen;
    });
    _wid_fullscreen->action([this]() {
        toggle_fullscreen();
    });

    /*
     * Photo-camera (screenshot) widget.
     */
    _wid_photocamera = std::make_shared<widget::PhotoCamera>(_renderer);
    _wid_photocamera->action([this]() {
        screenshot();
    });

    /*
     * Reset widget.
     */
    const auto paused_cb = [this]() {
        return paused();
    };

    _wid_reset = std::make_shared<widget::Reset>(_renderer, paused_cb);
    _wid_reset->action([this]() {
        if (_reset_cb) {
            _reset_cb();
        }
    });

    /*
     * Pause widget.
     */
    _wid_pause = std::make_shared<widget::Pause>(_renderer, paused_cb);
    _wid_pause->action([this]() {
        pause(paused() ^ true);
    });

    /*
     * Volume control widget.
     */
    if (audio_enabled()) {
        const auto getvol = [this]() {
            return audio_volume();
        };
        const auto setvol = [this](float vol) {
            audio_volume(vol);
        };
        _wid_volume = std::make_shared<widget::Volume>(_renderer, getvol, setvol);
    } else {
        _wid_volume = std::make_shared<widget::Volume>(_renderer);
    }

    /*
     * Enable/Disable Keybaord widget.
     */
    const auto kbd_status = [this]() {
        return widget::Keyboard::Status{.is_enabled = _kbd->is_enabled()};
    };

    const auto kbd_toggle = [this]() {
        _kbd->enable(!_kbd->is_enabled());
        log.debug("Keyboard {}\n", (_kbd->is_enabled() ? "enabled" : "disabled"));
    };

    _wid_keyboard = std::make_shared<widget::Keyboard>(_renderer, kbd_status);
    _wid_keyboard->action(kbd_toggle);

    /*
     * Empty widget used as separator.
     */
    auto empty = std::make_shared<widget::Empty>(_renderer);

    _panel->add(_wid_fullscreen, Panel::Just::Right);
    _panel->add(_wid_photocamera, Panel::Just::Right);
    _panel->add(empty, Panel::Just::Right);
    _panel->add(_wid_reset, Panel::Just::Right);
    _panel->add(_wid_pause, Panel::Just::Right);
    _panel->add(_wid_volume, Panel::Just::Right);
    _panel->add(_wid_keyboard, Panel::Just::Right);
    _panel->add(empty, Panel::Just::Right);
}

void UI::toggle_panel_visibility()
{
    _panel->visible(_panel->visible() ^ true);
}

void UI::run()
{
    _running = true;

    auto old_handler = std::signal(SIGINT, signal_handler);
    if (old_handler == SIG_ERR) {
        throw UIError{"Can't set SIGINT handler: {}", Error::to_string()};
    }

    if (std::signal(SIGSEGV, signal_handler) == SIG_ERR) {
        throw UIError{"Can't set SIGSEGV handler: {}", Error::to_string()};
    }

    if (std::signal(SIGABRT, signal_handler) == SIG_ERR) {
        throw UIError{"Can't set SIGABRT handler: {}", Error::to_string()};
    }

    attach_controllers();

    audio_play();
    event_loop();
    audio_stop();

    std::signal(SIGINT, old_handler);
    std::signal(SIGSEGV, old_handler);
    std::signal(SIGABRT, old_handler);

    _running = false;
}

void UI::event_loop()
{
    ::SDL_Event event{};
    int64_t start{};

    while (!_stop) {
        start = utils::now() - start;

        while (::SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                _stop = true;
                return;

            case SDL_WINDOWEVENT:
                win_event(event);
                break;

            case SDL_KEYUP:
            case SDL_KEYDOWN:
                kbd_event(event);
                break;

            case SDL_CONTROLLERAXISMOTION:
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMOVED:
            case SDL_CONTROLLERDEVICEREMAPPED:
            case SDL_CONTROLLERTOUCHPADDOWN:
            case SDL_CONTROLLERTOUCHPADMOTION:
            case SDL_CONTROLLERTOUCHPADUP:
            case SDL_CONTROLLERSENSORUPDATE:
                joy_event(event);
                break;

            case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEWHEEL:
                mouse_event(event);

            default:;
            }

            _panel->event(event);
        }

        if (_mouse_visible && (utils::now() - _mouse_active_time) > MOUSE_INACTIVE_TIME) {
            ::SDL_ShowCursor(SDL_DISABLE);
            _mouse_visible = false;
        }

        render_screen();

        if (signal_key != keyboard::KEY_NONE) {
            hotkeys(signal_key);
            signal_key = keyboard::KEY_NONE;
        }

        int64_t delay = _fps_time - utils::now() + start;
        start = (delay > 0 ? utils::sleep(delay) - delay : 0);
    }
}

void UI::win_event(const ::SDL_Event& event)
{
    const auto& wevent = event.window;

    switch (wevent.event) {
    case SDL_WINDOWEVENT_CLOSE:
        break;

    case SDL_WINDOWEVENT_FOCUS_GAINED:
        if (_kbd) {
            _kbd->reset();
        }
        break;

    case SDL_WINDOWEVENT_RESIZED:
        resize(wevent.data1, wevent.data2);
        break;

    default:;
    }
}

void UI::kbd_event(const SDL_Event& event)
{
#ifdef __APPLE__
    constexpr static const auto ALT = KMOD_GUI;
#else
    constexpr static const auto ALT = KMOD_ALT;
#endif

    const auto& kevent = event.key;
    const auto& key = kevent.keysym;

    switch (kevent.type) {
    case SDL_KEYDOWN:
        if (kevent.repeat != 0) {
            /* Key repeat disabled */
            break;
        }

        if (key.mod & ALT) {
            /*
             * Handle ALT-xx hotkeys.
             */
            switch (key.sym) {
            case SDLK_f:
                /*
                 * Toggle Fullscreen mode (ALT-F).
                 */
                _wid_fullscreen->action();
                break;

            case SDLK_j:
                /*
                 * Swap Joysticks (ALT-J).
                 */
                hotkeys(keyboard::KEY_ALT_J);
                break;

            case SDLK_k:
                /*
                 * Enable/Disable the emulated keyboard (ALT-K).
                 */
                _wid_keyboard->action();
                break;

            case SDLK_p:
                /*
                 * Toggle Pause mode (ALT-P).
                 */
                _wid_pause->action();
                break;

            case SDLK_s:
                /*
                 * Screenshot (ALT+SHIFT+S).
                 */
                if (key.mod & KMOD_SHIFT) {
                    _wid_photocamera->action();
                }
                break;

            case SDLK_v:
                /*
                 * Toggle Panel visibility (ALT+V).
                 */
                toggle_panel_visibility();
                break;

            default:;
            }

        } else if (key.sym == SDLK_PAUSE) {
            /*
             * Toggle Pause mode (PAUSE key).
             */
            _wid_pause->action();

        } else if (_kbd) {
            /*
             * Handle normal keyboard key press.
             */
            _kbd->key_pressed(to_key(key.scancode));
        }
        break;

    case SDL_KEYUP:
        if (_kbd) {
            _kbd->key_released(to_key(kevent.keysym.scancode));
        }
        break;

    default:;
    }
}

void UI::joy_event(const ::SDL_Event& event)
{
    int jid{};
    joyptr_t ejoy{};

    switch (event.type) {
    case SDL_CONTROLLERDEVICEADDED:
        log.debug("ui: Game controller detected: devid {}\n", event.cdevice.which);
        joy_add(event.cdevice.which);
        break;

    case SDL_CONTROLLERDEVICEREMOVED:
        log.debug("ui: Game controller disconnected: jid {}\n", event.cdevice.which);
        joy_del(event.cdevice.which);
        break;

    case SDL_CONTROLLERBUTTONUP:
    case SDL_CONTROLLERBUTTONDOWN:
        ejoy = find_joystick(event.cbutton.which);
        if (ejoy) {
            uint16_t pos = 0;
            const auto& jport = ejoy->port();
            switch (event.cbutton.button) {
            case SDL_CONTROLLER_BUTTON_A:
                pos = jport.a;
                break;
            case SDL_CONTROLLER_BUTTON_B:
                pos = jport.b;
                break;
            case SDL_CONTROLLER_BUTTON_X:
                pos = jport.x;
                break;
            case SDL_CONTROLLER_BUTTON_Y:
                pos = jport.y;
                break;
            case SDL_CONTROLLER_BUTTON_BACK:
                pos = jport.back;
                break;
            case SDL_CONTROLLER_BUTTON_GUIDE:
                pos = jport.guide;
                break;
            case SDL_CONTROLLER_BUTTON_START:
                pos = jport.start;
                break;
            case SDL_CONTROLLER_BUTTON_LEFTSTICK:
            case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
            case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
                pos = jport.fire;
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_UP:
                pos = jport.up;
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                pos = jport.down;
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                pos = jport.left;
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                pos = jport.right;
                break;
            default:
                log.warn("ui: Unrecognised button: {}. Ignored\n", event.cbutton.button);
            }

            if (pos != 0) {
                pos = (event.type == SDL_CONTROLLERBUTTONUP ? (ejoy->position() & ~pos) : (ejoy->position() | pos));
                ejoy->position(pos);
            }
        }
        break;

    case SDL_CONTROLLERAXISMOTION:
        jid = event.caxis.which;
        ejoy = find_joystick(jid);
        if (ejoy) {
            auto it = _sdl_joys.find(jid);
            auto* gc = it->second.get();

            int16_t ix = ::SDL_GameControllerGetAxis(gc, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX);
            int16_t iy = ::SDL_GameControllerGetAxis(gc, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY);

            if (ix < 2 && ix > -2 && iy < 2 && iy > -2) {
                /* Right joystick has the priority over the left joystick */
                ix = ::SDL_GameControllerGetAxis(gc, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX);
                iy = ::SDL_GameControllerGetAxis(gc, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTY);
            }

            const auto& jport = ejoy->port();
            uint16_t pos = ejoy->position() & (jport.fire | jport.a | jport.b | jport.x | jport.y |
                jport.guide | jport.back | jport.start);

            pos |= (ix < -12452 ? jport.left :
                   (ix >  12452 ? jport.right : 0));

            pos |= (iy < -12452 ? jport.up :
                   (iy >  12452 ? jport.down : 0));

            ejoy->position(pos);
        }
        break;

    case SDL_CONTROLLERDEVICEREMAPPED:
    case SDL_CONTROLLERTOUCHPADDOWN:
    case SDL_CONTROLLERTOUCHPADMOTION:
    case SDL_CONTROLLERTOUCHPADUP:
    case SDL_CONTROLLERSENSORUPDATE:
    default:;
    }
}

void UI::mouse_event(const SDL_Event& event)
{
    switch (event.type) {
    case SDL_MOUSEBUTTONUP:
        if (event.button.button == PANEL_BUTTON) {
            /*
             * Right-button toggles panel visiblity.
             */
            toggle_panel_visibility();
        }
        break;

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEMOTION:
    case SDL_MOUSEWHEEL:
    default:;
    }

    /*
     * Mouse moved or button pressed/release: The cursor must be visible.
     */
    if (!_mouse_visible) {
        ::SDL_ShowCursor(SDL_ENABLE);
        _mouse_active_time = utils::now();
        _mouse_visible = true;
    }
}

void UI::toggle_fullscreen()
{
    if (_is_fullscreen) {
        /*
         * Leave fullscreen.
         */
        if (::SDL_SetWindowFullscreen(_window.get(), 0) < 0) {
            log.error("ui: Can't leave fullscreen mode: {}\n", sdl_error());
            return;
        }

        _is_fullscreen = false;

    } else {
        /*
         * Enter fullscreen.
         */
        if (::SDL_SetWindowFullscreen(_window.get(), SDL_WINDOW_FULLSCREEN_DESKTOP) < 0) {
            log.error("ui: Can't enter fullscreen mode: {}\n", sdl_error());
            return;
        }

        _is_fullscreen = true;
    }
}

void UI::screen_sizes(float scale)
{
    _screen_scale = (scale == 0.0f ? 1.0f : scale);

    switch (_conf.video.sleffect) {
    case SLEffect::Adv_Horizontal:
        _screen_width  = (2 * _conf.video.width - _screen_ratio) * _screen_scale;
        _screen_height = _tex_height * _screen_scale;
        break;
    case SLEffect::Adv_Vertical:
        _screen_height = (2 * _conf.video.height - 1.0f / _screen_ratio) * _screen_scale;
        _screen_width  = _tex_width * _screen_scale;
        break;
    default:
        _screen_width = _tex_width * _screen_scale;
        _screen_height = _screen_width / _screen_ratio;
    }
}

void UI::resize(int width, int height)
{
    int x, y;

    _win_width = width;
    _win_height = height;

    /*
     * Resize keeping the aspect ratio.
     */
    if (_conf.video.sresize) {
        /*
         * Smooth resize: The emulated screen is gradually scaled.
         */
        _screen_width = _win_width;
        _screen_height = _screen_width / _screen_ratio;
        if (_screen_height > _win_height) {
            _screen_height = _win_height;
            _screen_width = _screen_height * _screen_ratio;
        }

        _screen_scale = _screen_width / _screen_height;

    } else {
        /*
         * Step resize: The emulated screen is integer scaled.
         */
        screen_sizes(std::floor(_win_width / _tex_width));
        if (_screen_height > _win_height) {
            screen_sizes(std::floor(_win_height / _tex_height));
            while (_screen_scale > 1.0f && (_screen_width > _win_width || _screen_height > _win_height)) {
                screen_sizes(_screen_scale - 1.0f);
            }
        }
    }

    x = (_win_width - _screen_width) / 2;
    if (x + _screen_width > _win_width) {
        x = 0;
    }

    y = (_win_height - _screen_height) / 2;
    if (y + _screen_height > _win_height) {
        y = 0;
    }

    _screen_rect = {
        .x = x,
        .y = y,
        .w = _screen_width,
        .h = _screen_height
    };
}

void UI::postrender_effects()
{
    auto sleffect = _conf.video.sleffect;
    if (sleffect == ui::SLEffect::None) {
        return;
    }

    int width = _win_width;
    int height = _win_height;
    uint8_t alpha = SCANLINE_COLOR.a;

    switch (sleffect) {
    case ui::SLEffect::Adv_Horizontal:
    case ui::SLEffect::Adv_Vertical:
        alpha = 255 * (1.0f - ADV_SCANLINE_BRIGHT);
        break;
    default:;
    }

    if (::SDL_SetRenderDrawColor(_renderer.get(), SCANLINE_COLOR.r, SCANLINE_COLOR.g, SCANLINE_COLOR.b, alpha) < 0) {
        log.error("ui: Can't set render draw color: {}\n", sdl_error());
        return;
    }

    ::SDL_Rect rect{
        .x = 0,
        .y = 0,
        .w = width,
        .h = height
    };

    int skip = std::ceil(_screen_scale);
    if (skip < 2) {
        skip = 2;
    }

    switch (sleffect) {
    case ui::SLEffect::Horizontal:
    case ui::SLEffect::Adv_Vertical:
        rect.h = skip / 2;
        for (int y = 1; y < height; y += skip) {
            rect.y = y;
            ::SDL_RenderFillRect(_renderer.get(), &rect);
        }
        break;

    case ui::SLEffect::Vertical:
    case ui::SLEffect::Adv_Horizontal:
        rect.w = skip / 2;
        for (int x = 0; x < width; x += skip) {
            rect.x = x;
            ::SDL_RenderFillRect(_renderer.get(), &rect);
        }
        break;

    default:;
    }
}

void UI::render_screen()
{
    uint32_t* dst{nullptr};
    int pitch{};

    if (::SDL_LockTexture(_screen_tex.get(), nullptr, reinterpret_cast<void**>(&dst), &pitch) < 0) {
        throw UIError{"Can't lock texture: {}", sdl_error()};
    }

#if 0
    // Is this necessary?
    if (pitch / _conf.video.width != 4) {
        log.fatal("ui: Invalid pitch: {}\n", pitch);
    }
#endif

    switch (_conf.video.sleffect) {
    case SLEffect::None:
        std::transform(_screen_raw.begin(), _screen_raw.end(), dst, [](Rgba px) {
            return px.u32;
        });
        break;

    case SLEffect::Horizontal:
    case SLEffect::Vertical:
        /*
         * The horizontal and vertical scanlines effect generate fake scanlines
         * by changing the alpha value of valid scanlines (see postrender_effects()).
         * Pixel values are increased to compensate for the loss of luminosity due to
         * these fake scanlines.
         */
        std::transform(_screen_raw.begin(), _screen_raw.end(), dst, [](Rgba px) {
            return (px * SCANLINE_LUMINOSITY).u32;
        });
        break;

    case SLEffect::Adv_Horizontal: {
        /*
         * In the advanced horizontal scanlines effect a new "empty" scanline
         * is intercalated between two valid scanlines.
         * These "empty" lines are filled with "reflections" of the neighbouring scanlines.
         * Pixel values of valid scanlines are increased to compensate for the loss of
         * luminosity caused by the "empty" lines.
         */
        unsigned line = 0;
        for (auto it = _screen_raw.begin(); it != _screen_raw.end(); it += _conf.video.width) {
            std::for_each(it, it + _conf.video.width, [&dst, &line, this](Rgba px) {
                Rgba pixel{px * ADV_SCANLINE_LUMINOSITY};
                Rgba refle{px * ADV_SCANLINE_REFLECTION};
                *dst = pixel.u32;
                if (line + 1 < _conf.video.height) {
                    dst[_tex_width] = refle.u32;
                }
                if (line > 0) {
                    dst[-_tex_width] = (Rgba{dst[-_tex_width]} + refle).u32;
                }
                ++dst;
            });
            dst += _conf.video.width;
            ++line;
        }}
        break;

    case SLEffect::Adv_Vertical: {
        /*
         * In the advanced vertical scanlines effect a new "empty" vertical line
         * is intercalated between two valid vertical lines.
         * These "empty" lines are filled with "reflections" of the neighbouring lines.
         * Pixel values of valid lines are increased to compensate for the loss of
         * luminosity caused by the "empty" lines.
         */
        unsigned x = 0;
        for (auto it = _screen_raw.begin(); it != _screen_raw.end(); it += _conf.video.width) {
            std::for_each(it, it + _conf.video.width, [&dst, &x, this](Rgba px) {
                Rgba pixel{px * ADV_SCANLINE_LUMINOSITY};
                Rgba refle{px * ADV_SCANLINE_REFLECTION};
                *dst = pixel.u32;
                if (x + 1 < _conf.video.width) {
                    dst[1] = refle.u32;
                }
                if (x > 0) {
                    dst[-1] = (Rgba{dst[-1]} + refle).u32;
                }
                dst += 2;
            });
            ++x;
            --dst;
        }}
        break;
    }

    ::SDL_UnlockTexture(_screen_tex.get());

    if (::SDL_SetRenderDrawColor(_renderer.get(), CRT_COLOR.r, CRT_COLOR.g, CRT_COLOR.b, CRT_COLOR.a) < 0 ||
        ::SDL_RenderClear(_renderer.get()) < 0 ||
        ::SDL_RenderCopy(_renderer.get(), _screen_tex.get(), nullptr, &_screen_rect) < 0) {
        throw IOError{"Can't copy texture: {}", sdl_error()};
    }

    postrender_effects();

    _panel->render(_win_width, _win_height);

    ::SDL_RenderPresent(_renderer.get());
}

inline UI::joyptr_t UI::find_joystick(::SDL_JoystickID jid)
{
    auto it = std::find_if(_joys.begin(), _joys.end(), [jid](const joyptr_t& joy) {
        return (static_cast<::SDL_JoystickID>(joy->joyid()) == jid);
    });

    return (it == _joys.end() ? joyptr_t{} : *it);
}

void UI::joy_add(int devid)
{
    const char* name = ::SDL_GameControllerNameForIndex(devid);
    if (name == nullptr) {
        name = "";
    }

    ::SDL_GameController* gc = ::SDL_GameControllerOpen(devid);
    if (gc == nullptr) {
        log.error("ui: Can't open game controller: devid {} \"{}\": {}. Controller ignored\n",
            devid, name, sdl_error());
        return;
    }

    ::SDL_Joystick* js = ::SDL_GameControllerGetJoystick(gc);
    ::SDL_JoystickID jid = ::SDL_JoystickInstanceID(js);
    auto eid = find_joystick(jid);
    if (eid) {
        log.debug("ui: Game controller already handled: devid {} \"{}\", jid {}\n", devid, name, jid);
        ::SDL_GameControllerClose(gc);
        return;
    }

    auto ejoy = find_joystick(Joystick::JOYID_UNASSIGNED);
    if (!ejoy) {
        log.debug("ui: No space for a new game controller: devid {} \"{} \": Controller ignored\n", devid, name);
        ::SDL_GameControllerClose(gc);
        return;
    }

    /*
     * Associate the new game controller to the emulated joystick.
     */
    auto gcptr = uptr_t<::SDL_GameController, void(*)(::SDL_GameController*)>{gc, ::SDL_GameControllerClose};
    _sdl_joys.emplace(jid, std::move(gcptr));

    ejoy->reset(jid, name);

    log.debug("ui: Game controller added: devid {} \"{}\", jid {}, controller {:p}\n",
        devid, name, jid, static_cast<void*>(gc));
}

void UI::joy_del(::SDL_JoystickID jid)
{
    auto ejoy = find_joystick(jid);
    if (ejoy) {
        ejoy->reset();
        auto nh = _sdl_joys.extract(jid);

        log.debug("ui: Game controller deleted: jid {}, controller {:p}\n",
            jid, static_cast<void*>(nh.mapped().get()));
    }
}

void UI::attach_controllers()
{
    size_t count = std::count_if(_joys.begin(), _joys.end(), [](const auto& ejoy) {
        return (ejoy->joyid() == Joystick::JOYID_UNASSIGNED);
    });

    log.debug("ui: Unassigned emulated joysticks: {}\n", count);

    for (int devid = 0; devid < ::SDL_NumJoysticks() && count > 0; ++devid) {
        if (::SDL_IsGameController(devid)) {
            joy_add(devid);
            --count;
        }
    }
}

void UI::screenshot() const
{
    /*
     * Native size and no rendering effects.
     */
    const int w = _conf.video.width;
    const int h = _conf.video.height;

    void* pixels = const_cast<Rgba*>(_screen_raw.data());

    const uptrd_t<::SDL_Surface> image{
        ::SDL_CreateRGBSurfaceWithFormatFrom(pixels, w, h, 32, w * 4, SDL_PIXELFORMAT_RGBA8888),
        ::SDL_FreeSurface
    };

    if (!image) {
        throw UIError{"Can't create screenshot image: {}", sdl_error()};
    }

    struct std::tm tm{};
    const time_t now = std::time(nullptr);
    if (::localtime_r(&now, &tm) == nullptr) {
        throw UIError{"Can't get local time: {}", Error::to_string()};
    }

    char buf[20]{};
    std::strftime(buf, sizeof(buf), "%Y-%m-%d_%H.%M.%S", &tm);

    const auto fname = std::format("{}/{}{}.png", _conf.video.screenshotdir, SCREENSHOT_PREFIX, buf);

    if (::IMG_SavePNG(image.get(), fname.c_str()) < 0) {
        throw UIError{"Can't save screenshot image: {}", sdl_error()};
    }
}

}
}
}
