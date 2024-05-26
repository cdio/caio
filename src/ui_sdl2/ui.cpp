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
#include <iostream>
#include <memory>

#include <SDL_image.h>

#include "endian.hpp"
#include "icon.hpp"
#include "logger.hpp"
#include "utils.hpp"

#include "ui_sdl2/widget_empty.hpp"
#include "ui_sdl2/widget_fullscreen.hpp"
#include "ui_sdl2/widget_pause.hpp"
#include "ui_sdl2/widget_reset.hpp"
#include "ui_sdl2/widget_volume.hpp"

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
        ::IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
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
     * Initialise main window, renderers and textures.
     */
    _window = ::SDL_CreateWindow(_conf.video.title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        _win_width, _win_height, SDL_WINDOW_RESIZABLE);

    if (_window == nullptr) {
        throw UIError{"Can't create main window: {}", sdl_error()};
    }

    const Image& ico = icon();

#ifdef __LITTLE_ENDIAN__
    _icon = ::SDL_CreateRGBSurfaceWithFormatFrom(const_cast<Rgba*>(ico.data.data()), ico.width, ico.height, 32,
        ico.width * 4, SDL_PIXELFORMAT_ABGR8888);
#else
    _icon = ::SDL_CreateRGBSurfaceWithFormatFrom(const_cast<Rgba*>(ico.data.data()), ico.width, ico.height, 32,
        ico.width * 4, SDL_PIXELFORMAT_RGBA8888);
#endif

    if (_icon == nullptr) {
        throw UIError{"Can't create main window icon: {}", sdl_error()};
    }

    ::SDL_SetWindowIcon(_window, _icon);

    _renderer = ::SDL_CreateRenderer(_window, -1, 0);
    if (_renderer == nullptr) {
        throw UIError{"Can't create renderer: {}", sdl_error()};
    }

    if (::SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND) < 0) {
        throw UIError{"Can't set renderer blend mode: {}", sdl_error()};
    }

    _screen_raw = std::vector<Rgba>(vconf.width * vconf.height, CRT_COLOR);

    _screen_tex = ::SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
        _tex_width, _tex_height);

    if (_screen_tex == nullptr) {
        throw UIError{"Can't create screen texture: {}", sdl_error()};
    }

    _is_fullscreen = false;
    if (vconf.fullscreen) {
        toggle_fullscreen();
    }

    audio_reset();
    create_panel();
}

UI::~UI()
{
    stop();

    //XXX FIXME stop does not mean that we can destroy the elements below
    //          we need to make sure the main loop is ended before doing that.

    _panel = {};

    if (_window != nullptr) {
        ::SDL_DestroyWindow(_window);
    }

    if (_renderer != nullptr) {
        /*
         * This also destroys the textures associated to the renderer.
         */
        ::SDL_DestroyRenderer(_renderer);
    }

    ::SDL_FreeSurface(_icon);
    ::IMG_Quit();
    ::SDL_Quit();
}

void UI::title(const std::string& title)
{
    if (_window != nullptr) {
        ::SDL_SetWindowTitle(_window, title.c_str());
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

::SDL_Renderer* UI::renderer()
{
    return _renderer;
}

void UI::create_panel()
{
    /*
     * Default panel widgets (from right to left):
     * Fullscreen, Reset, Pause, Volume.
     */
    _panel = std::make_shared<Panel>(_renderer);

    auto fullscreen = std::make_shared<widget::Fullscreen>(_renderer, [this]() { return _is_fullscreen; });
    fullscreen->action([this]() {
        toggle_fullscreen();
    });

    auto paused_cb = [this]() {
        return paused();
    };

    auto reset = std::make_shared<widget::Reset>(_renderer, paused_cb);
    reset->action([this]() {
        if (_reset_cb) {
            _reset_cb();
        }
    });

    auto pause = std::make_shared<widget::Pause>(_renderer, paused_cb);
    pause->action([this]() {
        this->pause(paused() ^ true);
    });

    std::function<float()> getvol = {};
    std::function<void(float)> setvol = {};
    if (audio_enabled()) {
        getvol = [this]() {
            /* Get volume */
            return audio_volume();
        };

        setvol = [this](float vol) {
            /* Set volume */
            audio_volume(vol);
        };
    }
    auto volume = std::make_shared<widget::Volume>(_renderer, getvol, setvol);

    auto empty = std::make_shared<widget::Empty>(_renderer);

    _panel->add(fullscreen, Panel::Just::RIGHT);
    _panel->add(reset, Panel::Just::RIGHT);
    _panel->add(pause, Panel::Just::RIGHT);
    _panel->add(volume, Panel::Just::RIGHT);
    _panel->add(empty, Panel::Just::RIGHT);
    _panel->add(empty, Panel::Just::RIGHT);
}

void UI::toggle_panel_visibility()
{
    _panel->visible(_panel->visible() ^ true);
}

void UI::run()
{
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
}

void UI::event_loop()
{
    ::SDL_Event event{};
    int64_t start{};
    int64_t delay{};

    while (!_stop) {
        start = caio::now() - start;

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

        if (_mouse_visible && (caio::now() - _mouse_active_time) > MOUSE_INACTIVE_TIME) {
            ::SDL_ShowCursor(SDL_DISABLE);
            _mouse_visible = false;
        }

        render_screen();

        if (signal_key != keyboard::KEY_NONE) {
            hotkeys(signal_key);
            signal_key = keyboard::KEY_NONE;
        }

        delay = _fps_time - caio::now() + start;
        start = (delay > 0 ? caio::sleep(delay) - delay : 0);
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
    const auto& kevent = event.key;
    const auto& key = kevent.keysym;

    switch (kevent.type) {
    case SDL_KEYDOWN:
        if (kevent.repeat != 0) {
            /* Key repeat disabled */
            break;
        }

        if (key.mod & KMOD_ALT) {
            /*
             * Handle ALT-xx hotkeys.
             */
            switch (key.sym) {
            case SDLK_f:
                /*
                 * Toggle Fullscreen mode.
                 */
                toggle_fullscreen();
                break;

            case SDLK_j:
                /*
                 * Swap Joysticks.
                 */
                hotkeys(keyboard::KEY_ALT_J);
                break;

            case SDLK_k:
                /*
                 * Toggle virtual joystick/keyboard.
                 */
                hotkeys(keyboard::KEY_ALT_K);
                break;

            case SDLK_m:
                /*
                 * Enter Monitor.
                 */
                hotkeys(keyboard::KEY_ALT_M);
                break;

            case SDLK_p:
                /*
                 * Toggle Pause mode.
                 */
                pause(paused() ^ true);
                break;

            case SDLK_v:
                /*
                 * Toggle Panel visibility.
                 */
                toggle_panel_visibility();
                break;

            default:;
            }

        } else if (key.sym == SDLK_PAUSE) {
            /*
             * Toggle Pause mode.
             */
            hotkeys(keyboard::KEY_PAUSE);

        } else if (_kbd) {
            /*
             * Handle normal keyboard key press.
             */
            if (!_panel->visible()) {
                _kbd->key_pressed(to_key(key.scancode));
            }
        }
        break;

    case SDL_KEYUP:
        if (key.sym == SDLK_ESCAPE && _panel->visible()) {
            /*
             * If the panel is active, the ESC key deactivates it.
             */
            toggle_panel_visibility();

        } else if (!_panel->visible() && _kbd) {
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
        _mouse_active_time = caio::now();
        _mouse_visible = true;
    }
}

void UI::toggle_fullscreen()
{
    if (_is_fullscreen) {
        /*
         * Leave fullscreen.
         */
        if (::SDL_SetWindowFullscreen(_window, 0) < 0) {
            log.error("ui: Can't leave fullscreen mode: {}\n", sdl_error());
            return;
        }

        _is_fullscreen = false;

    } else {
        /*
         * Enter fullscreen.
         */
        if (::SDL_SetWindowFullscreen(_window, SDL_WINDOW_FULLSCREEN_DESKTOP) < 0) {
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

    if (::SDL_SetRenderDrawColor(_renderer, SCANLINE_COLOR.r, SCANLINE_COLOR.g, SCANLINE_COLOR.b, alpha) < 0) {
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
            ::SDL_RenderFillRect(_renderer, &rect);
        }
        break;

    case ui::SLEffect::Vertical:
    case ui::SLEffect::Adv_Horizontal:
        rect.w = skip / 2;
        for (int x = 0; x < width; x += skip) {
            rect.x = x;
            ::SDL_RenderFillRect(_renderer, &rect);
        }
        break;

    default:;
    }
}

void UI::render_screen()
{
    uint32_t* dst{nullptr};
    int pitch{};

    if (::SDL_LockTexture(_screen_tex, nullptr, reinterpret_cast<void**>(&dst), &pitch) < 0) {
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

    ::SDL_UnlockTexture(_screen_tex);

    if (::SDL_SetRenderDrawColor(_renderer, CRT_COLOR.r, CRT_COLOR.g, CRT_COLOR.b, CRT_COLOR.a) < 0 ||
        ::SDL_RenderClear(_renderer) < 0 ||
        ::SDL_RenderCopy(_renderer, _screen_tex, nullptr, &_screen_rect) < 0) {
        throw IOError{"Can't copy texture: {}", sdl_error()};
    }

    postrender_effects();

    _panel->render(_win_width, _win_height);

    ::SDL_RenderPresent(_renderer);
}

inline UI::joyptr_t UI::find_joystick(::SDL_JoystickID jid)
{
    auto it = std::find_if(_joys.begin(), _joys.end(), [jid](const joyptr_t& joy) {
        return (joy->joyid() == jid);
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

    ejoy->reset(jid);

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
    size_t count = 0;
    for (const auto& ejoy : _joys) {
        if (ejoy->joyid() == Joystick::JOYID_UNASSIGNED) {
            ++count;
        }
    }

    log.debug("ui: Unassigned emulated joysticks: {}\n", count);

    for (int devid = 0; devid < ::SDL_NumJoysticks() && count > 0; ++devid) {
        if (::SDL_IsGameController(devid)) {
            joy_add(devid);
            --count;
        }
    }

}

}
}
}
