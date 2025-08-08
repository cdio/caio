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

#define UI_DEBUG(args...)      log.debug(args)
//#define UI_DEBUG(args...)

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
    auto& vconf = _conf.video;

    switch (vconf.sleffect) {
    case SLEffect::Adv_Horizontal:
    case SLEffect::Adv_Vertical:
        if (vconf.sresize) {
            log.warn("Smooth resize cannot be used with advanced scanlines effects. Smooth resize disabled\n");
            vconf.sresize = false;
        }
    default:;
    }

    init_sdl();
    init_window();
    init_renderer();
    init_texture();
    init_fs_mode();

    /*
     * Initialise the audio system and create the info panel.
     */
    audio_reset();
    create_panel();

    /*
     * Set fullscreen/normal mode.
     */
    _is_fullscreen = false;
    if (vconf.fullscreen) {
        toggle_fullscreen();
    }
}

UI::~UI() noexcept
{
    stop();

    while (_running) {
        std::this_thread::yield();
    }

    _panel = {};    /* Destroy the panel before SDL is terminated */

    ::IMG_Quit();
    ::SDL_Quit();
}

void UI::init_sdl()
{
    if (::SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0 ||
        ::IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG ||
        ::TTF_Init() != 0) {
        throw UIError{"Can't initialise SDL library: {}", sdl_error()};
    }
}

void UI::init_window_size()
{
    const auto& vconf = _conf.video;

    const Size2 default_screen_size{
        .width = static_cast<int>(vconf.width),
        .height = static_cast<int>(vconf.height)
    };

    _screen_ratio = aspect_ratio(default_screen_size);
    _screen_scale = (vconf.scale < 1.0f ? 1.0f : vconf.scale);

    _tex_size = default_screen_size;

    switch (vconf.sleffect) {
    case SLEffect::Adv_Horizontal:
        /*
         * In the advanced horizontal scanlines effect a new "empty"
         * scanline is intercalated between two valid scanlines.
         */
        _tex_size.height = 2 * vconf.height - 1;
        _screen_size.height = static_cast<int>(std::ceil(_tex_size.height * _screen_scale));
        _screen_size.width  = static_cast<int>(std::ceil(_screen_size.height * _screen_ratio));
        break;

    case SLEffect::Adv_Vertical:
        /*
         * In the advanced vertical scanlines effect a new "empty"
         * vertical line is intercalated between two valid vertical lines.
         */
        _tex_size.width = 2 * vconf.width - 1;
        _screen_size.width  = static_cast<int>(std::ceil(_tex_size.width * _screen_scale));
        _screen_size.height = static_cast<int>(std::ceil(_screen_size.width / _screen_ratio));
        break;

    default:
        _screen_size.width = static_cast<int>(std::ceil(_tex_size.width * _screen_scale));
        _screen_size.height = static_cast<int>(std::ceil(_screen_size.width / _screen_ratio));
    }

    /*
     * Main window size (windowed mode).
     */
    _win_size = _screen_size;

    /*
     * Position within the main window where the texture is rendered.
     */
    _screen_rect = {
        .x = 0,
        .y = 0,
        .w = _screen_size.width,
        .h = _screen_size.height
    };

    log.debug("ui: Aspect ratio: {}, scanlines effect: {}, tex size: {} x {}, screen size: {} x {}, window size: {} x {}\n",
        ui::to_string(vconf.aspect),
        ui::to_string(vconf.sleffect),
        _tex_size.width, _tex_size.height,
        _screen_size.width, _screen_size.height,
        _win_size.width, _win_size.height);
}

void UI::init_window()
{
    init_window_size();

    auto* windowp = ::SDL_CreateWindow(_conf.video.title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        _win_size.width, _win_size.height, SDL_WINDOW_RESIZABLE);

    if (windowp == nullptr) {
        throw UIError{"Can't create main window: {}", sdl_error()};
    }

    _window = sptr_t<::SDL_Window>{windowp, ::SDL_DestroyWindow};

    const Image& ico = icon();

    auto* iconp = ::SDL_CreateRGBSurfaceWithFormatFrom(const_cast<Rgba*>(ico.data.data()),
        ico.width, ico.height, 32, ico.width * 4, SDL_PIXELFORMAT_RGBA32);

    if (iconp == nullptr) {
        throw UIError{"Can't create window icon: {}", sdl_error()};
    }

    _icon = uptrd_t<::SDL_Surface>{iconp, ::SDL_FreeSurface};

    ::SDL_SetWindowIcon(_window.get(), _icon.get());

    if (const int rate = current_refresh(); rate < REFRESH_RATE) {
        log.warn("ui: Current refresh rate is too slow: {} Hz. Resolution changes is recommended\n", rate);
    }
}

void UI::init_renderer()
{
    auto* rendp = ::SDL_CreateRenderer(_window.get(), -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (rendp == nullptr) {
        throw UIError{"Can't create renderer: {}", sdl_error()};
    }

    _renderer = sptr_t<::SDL_Renderer>{rendp, ::SDL_DestroyRenderer};

    if (::SDL_SetRenderDrawBlendMode(_renderer.get(), SDL_BLENDMODE_BLEND) < 0) {
        throw UIError{"Can't set renderer blend mode: {}", sdl_error()};
    }
}

void UI::init_texture()
{
    /*
     * Raw (RGBA) scanline buffers.
     */
    const size_t area = _conf.video.width * _conf.video.height;
    _screen_raw[0] = Scanline(area, CRT_COLOR);
    _screen_raw[1] = Scanline(area, CRT_COLOR);

    /*
     * Screen texture.
     */
    auto* screenp = ::SDL_CreateTexture(_renderer.get(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
        _tex_size.width, _tex_size.height);

    if (screenp == nullptr) {
        throw UIError{"Can't create screen texture: {}", sdl_error()};
    }

    _screen_tex = sptr_t<::SDL_Texture>{screenp, ::SDL_DestroyTexture};
}

inline float UI::aspect_ratio(const Size2& wsize)
{
    return to_value(_conf.video.aspect, static_cast<int>(100.0f * wsize.width / wsize.height) / 100.0f);
}

Size2 UI::scale(const Size2& csize, const Size2& wsize)
{
    static const auto do_scale_x = [](const Size2& wsize, float scale_x, float ratio) -> Size2 {
        const auto w = static_cast<int>(wsize.width * scale_x);
        const auto h = static_cast<int>(w / ratio);
        return {w, h};
    };

    static const auto do_scale_y = [](const Size2& wsize, float scale_y, float ratio) -> Size2 {
        const auto h = static_cast<int>(wsize.height * scale_y);
        const auto w = static_cast<int>(h * ratio);
        return {w, h};
    };

    const float ratio = static_cast<float>(wsize.width) / wsize.height;

    float scale_x = static_cast<float>(csize.width) / wsize.width;
    float scale_y = static_cast<float>(csize.height) / wsize.height;

    if (!_conf.video.sresize) {
        scale_x = std::floor(scale_x);
        scale_y = std::floor(scale_y);
    }

    scale_x = std::max(1.0f, scale_x);
    scale_y = std::max(1.0f, scale_y);

    const int wx = scale_x * wsize.width;
    const int hx = wx / ratio;

    return (hx > csize.height ? do_scale_y(wsize, scale_y, ratio) : do_scale_x(wsize, scale_x, ratio));
}

std::tuple<Size2, ::SDL_DisplayMode> UI::closest_display_mode(const Size2& wsize)
{
    const int index = ::SDL_GetWindowDisplayIndex(_window.get());
    if (index < 0) {
        throw UIError{"Can't get current display index: {}", sdl_error()};
    }

    const int modes = ::SDL_GetNumDisplayModes(index);
    if (modes <= 0) {
        throw UIError{"Can't get number of display modes: {}", sdl_error()};
    }

    const ::SDL_DisplayMode desired{
        .format       = SDL_PIXELFORMAT_RGBA8888,
        .w            = wsize.width,
        .h            = wsize.height,
        .refresh_rate = REFRESH_RATE,
        .driverdata   = nullptr
    };

    UI_DEBUG("ui: Desired fullscreen mode for display index {}: {} x {} @ {} Hz\n",
        index,
        desired.w,
        desired.h,
        desired.refresh_rate);

    /*
     * Look for the supported display mode closest to the desired one.
     */
    ::SDL_DisplayMode candidate{};
    ::SDL_DisplayMode closest{};
    int closest_covered_area{};
    Size2 closest_wsize{};

    for (int m = 0; m < modes; ++m) {
        /*
         * Display modes are sorted:
         *     width                -> largest to smallest
         *     height               -> largest to smallest
         *     bits per pixel       -> more colors to fewer colors
         *     packed pixel layout  -> largest to smallest
         *     refresh rate         -> highest to lowest
         *
         * https://wiki.libsdl.org/SDL2/SDL_GetDisplayMode
         */
        if (::SDL_GetDisplayMode(index, m, &candidate) != 0) {
            throw UIError{"Can't get display mode: Display index {}, mode {}: {}", index, m, sdl_error()};
        }

        const int max_w = (desired.w > FULLSCREEN_MIN_WIDTH  ? desired.w : FULLSCREEN_MIN_WIDTH);
        const int max_h = (desired.h > FULLSCREEN_MIN_HEIGHT ? desired.h : FULLSCREEN_MIN_HEIGHT);
        const bool ignore = (candidate.refresh_rate != REFRESH_RATE || candidate.w < max_w || candidate.h < max_h);
        if (ignore) {
            /*
             * Some modern monitors do not work well with lower resolutions.
             */
            continue;
        }

        const auto candidate_wsize = scale({candidate.w, candidate.h}, wsize);

        const int candidate_area = candidate.w * candidate.h;
        const int desired_area = candidate_wsize.width * candidate_wsize.height;

        const int covered_area = static_cast<int>(100.0f * desired_area / candidate_area);
        const bool selected = (closest_covered_area <= covered_area);

        UI_DEBUG("ui: Candidate: {:4d} x {:4d} @ {:2d} Hz, covered area: {:3d}%, selected: {}\n",
            candidate.w,
            candidate.h,
            candidate.refresh_rate,
            covered_area,
            selected);

        if (selected) {
            closest = candidate;
            closest_covered_area = covered_area;
            closest_wsize = candidate_wsize;
        }
    }

    log.debug("ui: Selected fullscreen mode for display index {}: {} x {} @ {} Hz, covered area: {}%\n",
        index,
        closest.w,
        closest.h,
        closest.refresh_rate,
        closest_covered_area);

    return {closest_wsize, closest};
}

void UI::init_fs_mode()
{
    const auto& vconf = _conf.video;
    const int expand_w = (vconf.sleffect == SLEffect::Adv_Horizontal);
    const int expand_h = (vconf.sleffect == SLEffect::Adv_Vertical);

    Size2 wsize{
        .width  = _tex_size.width  << expand_w,
        .height = _tex_size.height << expand_h
    };

    const float aratio = aspect_ratio(wsize);

    if (vconf.aspect != AspectRatio::System) {
        if (expand_h) {
            wsize.height = std::floor(wsize.width / aratio);
        } else {
            wsize.width = std::floor(wsize.height * aratio);
        }
    }

    const auto [closest_wsize, closest] = closest_display_mode(wsize);
    if (closest.w == 0) {
        /*
         * No 60Hz modes: Downgrade to desktop fullscreen.
         */
        _fs_flags = SDL_WINDOW_FULLSCREEN_DESKTOP;

        const auto rate = current_refresh();
        log.warn("ui: Current refresh rate is too slow: {} Hz. Monitor change is recommended.\n", rate);

    } else {
        _fs_flags = SDL_WINDOW_FULLSCREEN;

        if (::SDL_SetWindowDisplayMode(_window.get(), &closest) != 0) {
            throw UIError{"Can't set fullscreen window display mode: {}", sdl_error()};
        }
    }

    const int margin_x = (closest.w - closest_wsize.width) >> 1;
    const int margin_y = (closest.h - closest_wsize.height) >> 1;

    _screen_fs_rect = {
        .x = margin_x,
        .y = margin_y,
        .w = closest_wsize.width,
        .h = closest_wsize.height
    };

    _fs_size = {
        .width  = closest.w,
        .height = closest.h
    };
}

int UI::current_refresh()
{
    ::SDL_DisplayMode mode{};

    if (::SDL_GetCurrentDisplayMode(0, &mode) != 0) {
        throw UIError{"Can't get current display mode: {}", sdl_error()};
    }

    return mode.refresh_rate;
}

void UI::title(const std::string& title)
{
    if (_window) {
        ::SDL_SetWindowTitle(_window.get(), title.c_str());
    }
}

void UI::keyboard(const KeyboardPtr& kbd)
{
    _kbd = kbd;
}

void UI::joystick(const std::initializer_list<JoystickPtr>& il)
{
    _joys = il;
}

void UI::hotkeys(const HotkeysCb& hotkeys_cb)
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

inline size_t UI::raw_index()
{
    return (_raw_index ^ 1);
}

bool UI::render_line(unsigned line, const Scanline& sline)
{
    if (_stop) {
//        log.debug("ui: Can't render line: System is stopped\n");
        return false;
    }

    if (line >= _conf.video.height || sline.size() != _conf.video.width) {
        log.fatal("ui: Can't render line: Invalid raster line {}, size {}.\n", line, sline.size());
        /* NOTREACHED */
    }

    auto& raw = _screen_raw[_raw_index];
    std::copy(sline.begin(), sline.end(), raw.begin() + line * _conf.video.width);

    if (line + 1 == _conf.video.height) {
        /*
         * Tell the main loop the video frame is ready to be rendered.
         */
        _raw_index ^= 1;
        _raw_sem.release();
        return true;
    }

    return false;
}

void UI::clear_screen(Rgba color)
{
    auto& raw = _screen_raw[raw_index()];
    std::fill(raw.begin(), raw.end(), color);
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
        _screenshot = true;
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

    const auto prev_int = std::signal(SIGINT, signal_handler);
    if (prev_int == SIG_ERR) {
        throw UIError{"Can't set SIGINT handler: {}", Error::to_string()};
    }

    const auto prev_segv = std::signal(SIGSEGV, signal_handler);
    if (prev_segv == SIG_ERR) {
        throw UIError{"Can't set SIGSEGV handler: {}", Error::to_string()};
    }

    const auto prev_abrt = std::signal(SIGABRT, signal_handler);
    if (prev_abrt == SIG_ERR) {
        throw UIError{"Can't set SIGABRT handler: {}", Error::to_string()};
    }

    attach_controllers();

    audio_play();
    event_loop();
    audio_stop();

    std::signal(SIGINT, prev_int);
    std::signal(SIGSEGV, prev_segv);
    std::signal(SIGABRT, prev_abrt);

    _running = false;
}

void UI::event_loop()
{
    ::SDL_Event event{};

    while (!_stop) {
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

        if (signal_key != keyboard::KEY_NONE) {
            hotkeys(signal_key);
            signal_key = keyboard::KEY_NONE;
        }

        if (!paused()) {
            _raw_sem.acquire();
        }

        render_screen();

        if (_screenshot) {
            screenshot();
            _screenshot = false;
        }
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
        resize_event(wevent.data1, wevent.data2);
        break;

    case SDL_WINDOWEVENT_MAXIMIZED:
        _is_maximized = true;
        break;

    case SDL_WINDOWEVENT_RESTORED:
    case SDL_WINDOWEVENT_MINIMIZED:
        _is_maximized = false;
        break;

    default:;
    }
}

void UI::resize_event(int width, int height)
{
    if (_is_fullscreen) {
        return;
    }

    _win_size = {
        .width = width,
        .height = height
    };

    if (_conf.video.sresize) {
        /*
         * Smooth resize: The emulated screen is gradually scaled.
         */
        _screen_size = scale(_win_size, _tex_size);
        _screen_scale = static_cast<float>(_screen_size.width) / _tex_size.width;

    } else {
        /*
         * Step resize: The emulated screen is integer scaled.
         */
        const bool expand_h = (_conf.video.sleffect == SLEffect::Adv_Vertical);

        const int width = (expand_h ? _tex_size.width : _tex_size.height * _screen_ratio);
        const int height = (expand_h ? _tex_size.width / _screen_ratio : _tex_size.height);

        const int scale_xy = (expand_h ? _win_size.width / _tex_size.width :
                                         _win_size.height / _tex_size.height);

        int scale = std::max(1, scale_xy);
        for (; scale > 1 && ((width * scale > _win_size.width) || (height * scale > _win_size.height)); --scale);

        _screen_scale = scale;
        _screen_size = { width * scale, height * scale };
    }

    const int margin_x = (_win_size.width - _screen_size.width) / 2;
    const int margin_y = (_win_size.height - _screen_size.height) / 2;

    _screen_rect = {
        .x = (_screen_size.width <= _win_size.width) * margin_x,
        .y = (_screen_size.height <= _win_size.height) * margin_y,
        .w = _screen_size.width,
        .h = _screen_size.height
    };
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
    JoystickPtr ejoy{};

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

            int16_t ix{};
            int16_t iy{};

            switch (event.caxis.axis) {
            case SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX:
            case SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTY:
                ix = ::SDL_GameControllerGetAxis(gc, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX);
                iy = ::SDL_GameControllerGetAxis(gc, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTY);
                break;
            case SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX:
            case SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY:
                ix = ::SDL_GameControllerGetAxis(gc, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX);
                iy = ::SDL_GameControllerGetAxis(gc, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY);
                break;
            default:;
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
    if (_is_maximized) {
        /*
         * FIXME
         * macos chrashes miserably when the fullscreen mode
         * is selected while in maximized windowed mode.
         * Check if this is a SDL2 problem.
         */
        return;
    }

    if (_is_fullscreen) {
        /*
         * Leave fullscreen.
         */
        if (::SDL_SetWindowFullscreen(_window.get(), 0) < 0) {
            log.error("ui: Can't leave fullscreen mode: {}\n", sdl_error());
            return;
        }

        ::SDL_SetWindowPosition(_window.get(), _win_pos.width, _win_pos.height);
        ::SDL_SetWindowSize(_window.get(), _win_size.width, _win_size.height);

        _is_fullscreen = false;

    } else {
        /*
         * Enter fullscreen.
         */
        ::SDL_GetWindowPosition(_window.get(), &_win_pos.width, &_win_pos.height);

        if (::SDL_SetWindowFullscreen(_window.get(), _fs_flags) < 0) {
            log.error("ui: Can't enter fullscreen mode: {}\n", sdl_error());
            return;
        }

        _is_fullscreen = true;
    }
}

void UI::postrender_effects()
{
    //TODO improve speed (shader?)

    const auto sleffect = _conf.video.sleffect;
    if (sleffect == ui::SLEffect::None) {
        return;
    }

    const Size2& wsize = (_is_fullscreen ? _fs_size : _win_size);

    const uint8_t alpha = ((sleffect == ui::SLEffect::Adv_Horizontal || sleffect == ui::SLEffect::Adv_Vertical) ?
        255 * (1.0f - ADV_SCANLINE_BRIGHT) : SCANLINE_COLOR.a);

    if (::SDL_SetRenderDrawColor(_renderer.get(), SCANLINE_COLOR.r, SCANLINE_COLOR.g, SCANLINE_COLOR.b, alpha) < 0) {
        log.error("ui: Can't set render draw color: {}\n", sdl_error());
        return;
    }

    ::SDL_Rect rect{
        .x = 0,
        .y = 0,
        .w = wsize.width,
        .h = wsize.height
    };

    const int scale = (_is_fullscreen ? 2 : std::ceil(_screen_scale));
    const int skip = (scale < 2 ? 2 : scale);

    switch (sleffect) {
    case ui::SLEffect::Horizontal:
    case ui::SLEffect::Adv_Vertical:
        rect.h = skip / 2;
        for (int y = 1; y < wsize.height; y += skip) {
            rect.y = y;
            ::SDL_RenderFillRect(_renderer.get(), &rect);
        }
        break;

    case ui::SLEffect::Vertical:
    case ui::SLEffect::Adv_Horizontal:
        rect.w = skip / 2;
        for (int x = 0; x < wsize.width; x += skip) {
            rect.x = x;
            ::SDL_RenderFillRect(_renderer.get(), &rect);
        }
        break;

    default:;
    }
}

void UI::render_screen()
{
    const auto sleffect_alpha = [](Rgba* dst, const Scanline& raw) {
        /*
         * The horizontal and vertical scanlines effect generate fake scanlines
         * by changing the alpha value of valid scanlines (see postrender_effects()).
         * Pixel values are increased to compensate for the loss of luminosity due to
         * these fake scanlines.
         */
        std::transform(raw.begin(), raw.end(), dst, [](Rgba px) {
            return (px * SCANLINE_LUMINOSITY).u32;
        });
    };

    const auto sleffect_adv_horizontal = [this](Rgba* dst, const Scanline& raw) {
        /*
         * In the advanced horizontal scanlines effect a new "empty" scanline
         * is intercalated between two valid scanlines.
         * These "empty" lines are filled with "reflections" of the neighbouring scanlines.
         * Pixel values of valid scanlines are increased to compensate for the loss of
         * luminosity caused by the "empty" lines.
         */
        unsigned line = 0;
        for (auto it = raw.begin(); it != raw.end(); it += _conf.video.width) {
            std::for_each(it, it + _conf.video.width, [&dst, &line, this](Rgba px) {
                const Rgba pixel{px * ADV_SCANLINE_LUMINOSITY};
                const Rgba refle{px * ADV_SCANLINE_REFLECTION};
                *dst = pixel;
                if (line + 1 < _conf.video.height) {
                    dst[_tex_size.width] = refle;
                }
                if (line > 0) {
                    dst[-_tex_size.width] = dst[-_tex_size.width] + refle;
                }
                ++dst;
            });
            dst += _conf.video.width;
            ++line;
        }
    };

    const auto sleffect_adv_vertical = [this](Rgba* dst, const Scanline& raw) {
        /*
         * In the advanced vertical scanlines effect a new "empty" vertical line
         * is intercalated between two valid vertical lines.
         * These "empty" lines are filled with "reflections" of the neighbouring lines.
         * Pixel values of valid lines are increased to compensate for the loss of
         * luminosity caused by the "empty" lines.
         */
        unsigned x = 0;
        for (auto it = raw.begin(); it != raw.end(); it += _conf.video.width) {
            std::for_each(it, it + _conf.video.width, [&dst, &x, this](Rgba px) {
                const Rgba pixel{px * ADV_SCANLINE_LUMINOSITY};
                const Rgba refle{px * ADV_SCANLINE_REFLECTION};
                *dst = pixel;
                if (x + 1 < _conf.video.width) {
                    dst[1] = refle;
                }
                if (x > 0) {
                    dst[-1] = dst[-1] + refle;
                }
                dst += 2;
            });
            ++x;
            --dst;
        }
    };

    Rgba* dst{nullptr};
    int pitch{};

    if (::SDL_LockTexture(_screen_tex.get(), nullptr, reinterpret_cast<void**>(&dst), &pitch) < 0) {
        throw UIError{"Can't lock texture: {}", sdl_error()};
    }

    const auto& raw = _screen_raw[raw_index()];

    switch (_conf.video.sleffect) {
    case SLEffect::None:
        std::copy(raw.begin(), raw.end(), dst);
        break;

    case SLEffect::Horizontal:
    case SLEffect::Vertical:
        sleffect_alpha(dst, raw);
        break;

    case SLEffect::Adv_Horizontal:
        sleffect_adv_horizontal(dst, raw);
        break;

    case SLEffect::Adv_Vertical:
        sleffect_adv_vertical(dst, raw);
        break;
    }

    ::SDL_UnlockTexture(_screen_tex.get());

    const auto& rect = (_is_fullscreen ? _screen_fs_rect : _screen_rect);

    if (::SDL_SetRenderDrawColor(_renderer.get(), CRT_COLOR.r, CRT_COLOR.g, CRT_COLOR.b, CRT_COLOR.a) < 0 ||
        ::SDL_RenderClear(_renderer.get()) < 0 ||
        ::SDL_RenderCopy(_renderer.get(), _screen_tex.get(), nullptr, &rect) < 0) {
        throw IOError{"Can't copy texture: {}", sdl_error()};
    }

    postrender_effects();

    const auto& wsize = (_is_fullscreen ? _fs_size : _win_size);
    _panel->render(wsize.width, wsize.height);

    ::SDL_RenderPresent(_renderer.get());
}

inline UI::JoystickPtr UI::find_joystick(::SDL_JoystickID jid)
{
    auto it = std::find_if(_joys.begin(), _joys.end(), [jid](const JoystickPtr& joy) {
        return (static_cast<::SDL_JoystickID>(joy->joyid()) == jid);
    });

    return (it == _joys.end() ? JoystickPtr{} : *it);
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

    log.debug("ui: Non attached emulated joysticks: {}\n", count);

    for (int devid = 0; devid < ::SDL_NumJoysticks() && count > 0; ++devid) {
        if (::SDL_IsGameController(devid)) {
            joy_add(devid);
            --count;
        }
    }
}

void UI::screenshot()
{
    int w{}, h{};
    ::SDL_GetWindowSize(_window.get(), &w, &h);

    uptrd_t<::SDL_Surface> image{
        ::SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA32),
        ::SDL_FreeSurface
    };

    if (!image) {
        throw UIError{"Can't create screenshot image: {}", sdl_error()};
    }

    if (::SDL_RenderReadPixels(_renderer.get(), nullptr, image->format->format, image->pixels, image->pitch) != 0) {
        throw UIError{"Can't read screenshot pixels: {}", sdl_error()};
    }

#if 1 // FIXME std::format for date/time not yet available
    struct std::tm tm{};
    const time_t now = std::time(nullptr);
    if (::localtime_r(&now, &tm) == nullptr) {
        throw UIError{"Can't get local time: {}", Error::to_string()};
    }

    char buf[20]{};
    std::strftime(buf, sizeof(buf), "%Y-%m-%d_%H.%M.%S", &tm);
    const auto fname = std::format("{}/{}{}.png", _conf.video.screenshotdir, SCREENSHOT_PREFIX, buf);
#else
    const auto utc = std::chrono::system_clock::now();
    const auto now = std::chrono::zoned_time{utc};
    const auto fname = std::format("{}/{}{:%F_%H.%M.%S}.png", _conf.video.screenshotdir, SCREENSHOT_PREFIX, now);
#endif

    if (::IMG_SavePNG(image.get(), fname.c_str()) < 0) {
        throw UIError{"Can't save screenshot image: {}", sdl_error()};
    }
}

}
}
}
