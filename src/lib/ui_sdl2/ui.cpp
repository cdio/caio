/*
 * Copyright (C) 2020-2022 Claudio Castiglia
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
#include <memory>

#include <SDL_image.h>

#include "icon.hpp"
#include "logger.hpp"
#include "utils.hpp"

#include "ui_sdl2/sdl2.hpp"
#include "ui_sdl2/widget_empty.hpp"
#include "ui_sdl2/widget_fullscreen.hpp"
#include "ui_sdl2/widget_pause.hpp"
#include "ui_sdl2/widget_reset.hpp"
#include "ui_sdl2/widget_volume.hpp"


namespace caio {
namespace ui {
namespace sdl2 {

static std::atomic<Keyboard::Key> signal_key{Keyboard::KEY_NONE};

static void signal_handler(int signo)
{
    switch (signo) {
    case SIGINT:
        signal_key = Keyboard::KEY_CTRL_C;
        break;

    default:;
    }
}

UI::UI(const Config &conf)
    : _conf{conf}
{
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0 ||
        IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        throw_sdl_uierror("Can't initialise SDL library");
    }

    const auto &vconf = conf.video;

    _fps_time = 1000000 / vconf.fps;

    _screen_width = static_cast<int>(vconf.width * vconf.scale);
    _screen_height = static_cast<int>(vconf.height * vconf.scale);
    _screen_ratio = static_cast<float>(vconf.width) / static_cast<float>(vconf.height);

    _screen_rect = {
        .x = 0,
        .y = 0,
        .w = _screen_width,
        .h = _screen_height
    };

    _win_width = _screen_width;
    _win_height = _screen_height;

    _window = SDL_CreateWindow(vconf.title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        _win_width, _win_height, SDL_WINDOW_RESIZABLE);

    if (_window == nullptr) {
        throw_sdl_uierror("Can't create main window");
    }

    const Image &ico = icon();

    _icon = SDL_CreateRGBSurfaceWithFormatFrom(const_cast<Rgba *>(ico.data.data()), ico.width, ico.height, 32,
        ico.width * 4, SDL_PIXELFORMAT_RGBA8888);

    if (_icon == nullptr) {
        throw_sdl_uierror("Can't create main window icon");
    }

    SDL_SetWindowIcon(_window, _icon);

    _renderer = SDL_CreateRenderer(_window, -1, 0);
    if (_renderer == nullptr) {
        throw_sdl_uierror("Can't create renderer");
    }

    if (SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND) < 0) {
        throw_sdl_uierror("Can't set renderer blend mode");
    }

    _screen_raw = std::vector<Rgba>(vconf.width * vconf.height);

    _screen_tex = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
        vconf.width, vconf.height);

    if (_screen_tex == nullptr) {
        throw_sdl_uierror("Can't create screen texture");
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

    for (auto &pair : _sdl_joys) {
        SDL_Joystick *sjoy = pair.second;
        SDL_JoystickClose(sjoy);
    }

    _panel = {};

    if (_window != nullptr) {
        SDL_DestroyWindow(_window);
    }

    if (_renderer != nullptr) {
        /*
         * This also destroys the textures associated to the renderer.
         */
        SDL_DestroyRenderer(_renderer);
    }

    SDL_FreeSurface(_icon);
    IMG_Quit();
    SDL_Quit();
}

void UI::title(const std::string &title)
{
    if (_window != nullptr) {
        SDL_SetWindowTitle(_window, title.c_str());
    }
}

void UI::keyboard(const keyboard_ptr_t &kbd)
{
    _kbd = kbd;
}

void UI::joystick(const std::initializer_list<joystick_ptr_t> &il)
{
    _joys = il;
}

void UI::hotkeys(const hotkeys_cb_t &hotkeys_cb)
{
    _hotkeys_cb = hotkeys_cb;
}

void UI::hotkeys(Keyboard::Key key)
{
    if (_hotkeys_cb) {
        _hotkeys_cb(key);
    }
}

void UI::pause(const std::function<void(bool)> &pause_cb, const std::function<bool()> &ispause_cb)
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

void UI::reset(const std::function<void()> &reset_cb)
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

void UI::render_line(unsigned line, const Scanline &sline)
{
    if (_stop) {
//        log.debug("ui: Can't render line: System is stopped.\n");
        return;
    }

    if (line >= _conf.video.height || sline.size() != _conf.video.width) {
        log.warn("ui: Can't render line: Invalid raster line %d, size %d. Ignored\n", line, sline.size());
        return;
    }

    std::copy(sline.begin(), sline.end(), _screen_raw.begin() + line * _conf.video.width);
}

void UI::stop()
{
    _stop = true;
}

std::string UI::to_string() const
{
    return sdl_version();
}

std::shared_ptr<Panel> UI::panel()
{
    return _panel;
}

SDL_Renderer *UI::renderer()
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

    auto volume = std::make_shared<widget::Volume>(_renderer,
        [this]() {
            /* Get volume */
            return audio_volume();
        },
        [this](float vol) {
            /* Set volume */
            audio_volume(vol);
        });

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
        throw UIError{"ui: Can't set signal handler: " + Error::to_string()};
    }

    audio_play();

    event_loop();

    audio_stop();

    std::signal(SIGINT, old_handler);
}

void UI::event_loop()
{
    SDL_Event event{};
    int64_t start{};
    int64_t delay{};

    while (!_stop) {
        start = utils::now() - start;

        while (SDL_PollEvent(&event)) {
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

            case SDL_JOYAXISMOTION:
            case SDL_JOYBALLMOTION:
            case SDL_JOYHATMOTION:
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
            case SDL_JOYDEVICEADDED:
            case SDL_JOYDEVICEREMOVED:
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
            SDL_ShowCursor(SDL_DISABLE);
            _mouse_visible = false;
        }

        render_screen();

        if (signal_key != Keyboard::KEY_NONE) {
            hotkeys(signal_key);
            signal_key = Keyboard::KEY_NONE;
        }

        delay = _fps_time - utils::now() + start;
        start = (delay > 0 ? utils::sleep(delay) - delay: 0);
    }
}

void UI::win_event(const SDL_Event &event)
{
    const auto &wevent = event.window;

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

void UI::kbd_event(const SDL_Event &event)
{
    const auto &kevent = event.key;
    const auto &key = kevent.keysym;

    switch (kevent.type) {
    case SDL_KEYDOWN:
        if (kevent.repeat != 0) {
            /* Key repeat disabled */
            break;
        }

        if (key.mod & (KMOD_LALT | KMOD_RALT)) {
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
                hotkeys(Keyboard::KEY_ALT_J);
                break;

            case SDLK_m:
                /*
                 * Enter Monitor.
                 */
                hotkeys(Keyboard::KEY_ALT_M);
                break;

            case SDLK_p:
                /*
                 * Toggle Pause mode.
                 */
                hotkeys(Keyboard::KEY_PAUSE);
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
            hotkeys(Keyboard::KEY_PAUSE);

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

void UI::joy_event(const SDL_Event &event)
{
    int32_t jid{};
    joystick_ptr_t ejoy{};

    switch (event.type) {
    case SDL_JOYDEVICEADDED:
        /*
         * New joystick detected.
         */
        jid = event.jdevice.which;
        log.debug("ui: New game controller detected, id: %d\n", jid);
        joy_add(jid);
        break;

    case SDL_JOYDEVICEREMOVED:
        /*
         * Joystick removed.
         */
        jid = event.jdevice.which;
        log.debug("ui: Game controller disconnected, id: %d\n", jid);
        joy_del(jid);
        break;

    case SDL_JOYBUTTONDOWN:
        jid = event.jbutton.which;
        ejoy = joystick(jid);
        if (ejoy) {
            auto pos = ejoy->position() | Joystick::JOY_FIRE;
            ejoy->position(pos);
        }
        break;

    case SDL_JOYBUTTONUP:
        jid = event.jbutton.which;
        ejoy = joystick(jid);
        if (ejoy) {
            auto pos = ejoy->position() & ~Joystick::JOY_FIRE;
            ejoy->position(pos);
        }
        break;

    case SDL_JOYHATMOTION:
        jid = event.jhat.which;
        ejoy = joystick(jid);
//        log.debug("ui: joy: %d, hat: %d, value: %d\n", jid, event.jhat.hat, event.jhat.value);
        if (ejoy) {
            uint8_t pos{Joystick::JOY_NONE};
            switch (event.jhat.value) {
            case SDL_HAT_UP:
                pos = Joystick::JOY_UP;
                break;
            case SDL_HAT_RIGHT:
                pos = Joystick::JOY_RIGHT;
                break;
            case SDL_HAT_DOWN:
                pos = Joystick::JOY_DOWN;
                break;
            case SDL_HAT_LEFT:
                pos = Joystick::JOY_LEFT;
                break;
            case SDL_HAT_RIGHTUP:
                pos = Joystick::JOY_RIGHT | Joystick::JOY_UP;
                break;
            case SDL_HAT_RIGHTDOWN:
                pos = Joystick::JOY_RIGHT | Joystick::JOY_DOWN;
                break;
            case SDL_HAT_LEFTUP:
                pos = Joystick::JOY_LEFT | Joystick::JOY_UP;
                break;
            case SDL_HAT_LEFTDOWN:
                pos = Joystick::JOY_LEFT | Joystick::JOY_DOWN;
                break;
            case SDL_HAT_CENTERED:
            default:
                break;
            }
            ejoy->position(pos);
        }
        break;

    case SDL_JOYAXISMOTION:
        jid = event.jaxis.which;
        ejoy = joystick(jid);
        if (ejoy) {
            /*
             * Axis 0: Left joystick X direction
             * Axis 2: Right joystick X direction
             *
             * Axis 1: Left joystick Y direction
             * Axis 3: Right joystick Y direction
             *
             * Positive value: Right or Down
             * Negative value: Left or Up
             */
            auto *sjoy = _sdl_joys[jid];
            uint8_t axis = event.jaxis.axis;

            int16_t ix, iy;
            if ((axis & 1) == 0) {
                ix = event.jaxis.value;
                iy = SDL_JoystickGetAxis(sjoy, axis | 0x01);
            } else {
                ix = SDL_JoystickGetAxis(sjoy, axis & 0xFE);
                iy = event.jaxis.value;
            }

            uint8_t pos = ejoy->position() & Joystick::JOY_FIRE;

            pos |= (ix < -12452 ? Joystick::JOY_LEFT :
                   (ix >  12452 ? Joystick::JOY_RIGHT : 0));

            pos |= (iy < -12452 ? Joystick::JOY_UP :
                   (iy >  12452 ? Joystick::JOY_DOWN : 0));

            ejoy->position(pos);
        }
        break;

    case SDL_JOYBALLMOTION:
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
    default:;
    }
}

void UI::mouse_event(const SDL_Event &event)
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
        SDL_ShowCursor(SDL_ENABLE);
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
        if (SDL_SetWindowFullscreen(_window, 0) < 0) {
            log.error("ui: Can't leave fullscreen mode: %s\n", sdl_error().c_str());
            return;
        }

        _is_fullscreen = false;

    } else {
        /*
         * Enter fullscreen.
         */
        if (SDL_SetWindowFullscreen(_window, SDL_WINDOW_FULLSCREEN_DESKTOP) < 0) {
            log.error("ui: Can't enter fullscreen mode: %s\n", sdl_error().c_str());
            return;
        }

        _is_fullscreen = true;
    }
}

void UI::resize(int width, int height)
{
    int x, y, w, h;

    _win_width = width;
    _win_height = height;

    /*
     * Resize keeping the aspect ratio.
     */
    if (_conf.video.smooth_resize) {
        /*
         * Smooth resize: The emulated screen is gradually scaled.
         */
        w = _win_width;
        h = w / _screen_ratio;
        if (h > _win_height) {
            h = _win_height;
            w = _screen_ratio * h;
        }
    } else {
        /*
         * Step resize: The emulated screen is integer scaled.
         */
        w = _win_width / _conf.video.width;
        h = _win_height / _conf.video.height;
        w *= _conf.video.width;
        h *= _conf.video.height;
        if (w == 0) {
            w = _screen_ratio * h;
        } else {
            h = w / _screen_ratio;
        }
    }

    if (w <= 0 || h <= 0) {
        w = _conf.video.width;
        h = _conf.video.height;
    }

    x = (_win_width - w) >> 1;
    y = (_win_height - h) >> 1;

    if (x + w > _win_width) {
        x = 0;
    }

    if (y + h > _win_height) {
        y = 0;
    }

    /* Update the emulated screen size */
    _screen_rect = {
        .x = x,
        .y = y,
        .w = w,
        .h = h
    };
}

void UI::render_screen()
{
    uint32_t *dst{nullptr};
    int pitch{};

    /*
     * Draw the scaled emulated screen.
     */
    if (SDL_LockTexture(_screen_tex, nullptr, reinterpret_cast<void **>(&dst), &pitch) < 0) {
        throw_sdl_uierror("Can't lock texture");
    }

#if 0
    // Is this necessary?
    if (pitch / _conf.video.width != 4) {
        log.fatal("ui: Invalid pitch: %d\n", pitch);
    }
#endif

    for (auto &pixel : _screen_raw) {
        *dst++ = pixel.to_host_u32();
    }

    SDL_UnlockTexture(_screen_tex);

    if (SDL_SetRenderDrawColor(_renderer, CRT_COLOR.r, CRT_COLOR.g, CRT_COLOR.b, CRT_COLOR.a) < 0 ||
        SDL_RenderClear(_renderer) < 0 ||
        SDL_RenderCopy(_renderer, _screen_tex, nullptr, &_screen_rect) < 0) {
        throw_sdl_uierror("Can't copy texture");
    }

    /*
     * Scanlines effect.
     */
    auto sleffect = _conf.video.sleffect;
    if (sleffect != ui::SLEffect::NONE) {
        int width = _win_width;
        int height = _win_height;

        if (SDL_SetRenderDrawColor(_renderer, SCANLINE_COLOR.r, SCANLINE_COLOR.g, SCANLINE_COLOR.b,
            SCANLINE_COLOR.a) < 0) {
            log.error("ui: Can't set render draw color: %s\n", sdl_error().c_str());

        } else {
            int scale = static_cast<int>(_conf.video.scale);
            if (scale == 1) {
                ++scale;
            }

            switch (sleffect) {
            case ui::SLEffect::HORIZONTAL:
                for (int y = 0; y < height; y += scale) {
                    SDL_RenderDrawLine(_renderer, 0, y, width, y);
                }
                break;

            case ui::SLEffect::VERTICAL:
                for (int x = 0; x < width; x += scale) {
                    SDL_RenderDrawLine(_renderer, x, 0, x, height);
                }
                break;

            default:;
            }
        }
    }

    _panel->render(_win_width, _win_height);

    SDL_RenderPresent(_renderer);
}

std::shared_ptr<Joystick> UI::joystick(unsigned jid)
{
    if (jid < _joys.size()) {
        return _joys[jid];
    }

    return {};
}

void UI::joy_add(int32_t jid)
{
    joystick_ptr_t ejoy = joystick(jid);
    if (!ejoy) {
        log.debug("ui: No room for a new game controller, id: %d. New controller ignored.\n",
            jid);
        return;
    }

    SDL_Joystick *sjoy = SDL_JoystickOpen(jid);
    if (sjoy == nullptr) {
        log.error("ui: Can't open new game controller, id: %d: %s. New game controller ignored.\n",
            jid, sdl_error().c_str());
        return;
    }

    /*
     * Associate the new game controller to an emulated joystick.
     */
    _sdl_joys[jid] = sjoy;
    ejoy->reset(jid);
    log.debug("ui: New game controller added, id: %d, 0x%p\n", jid, sjoy);
}

void UI::joy_del(int32_t jid)
{
    joystick_ptr_t ejoy = joystick(jid);
    if (!ejoy) {
        /*
         * The game controller was never associated to an emulated joystick.
         */
        return;
    }

    SDL_Joystick *sjoy = _sdl_joys[jid];
    if (sjoy != nullptr) {
        /*
         * Dis-associate the game controller from
         * the emulated joystick and release resources.
         */
        ejoy->reset();
        SDL_JoystickClose(sjoy);
        _sdl_joys.erase(jid);
        log.debug("ui: Game controller deleted, id: %d, 0x%p\n", jid, sjoy);
    }
}

}
}
}
