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
#include "ui_sdl2/widget.hpp"

#include "logger.hpp"
#include "utils.hpp"
#include "ui_sdl2/sdl2.hpp"

namespace caio {
namespace ui {
namespace sdl2 {
namespace widget {

::SDL_Rect to_rect(const ::SDL_Rect& rect, const ::SDL_Rect& dstrect)
{
    const float width_factor = static_cast<float>(dstrect.w) / WIDTH;
    const float height_factor = static_cast<float>(dstrect.h) / HEIGHT;
    const int in_widget_x = rect.x * width_factor;
    const int in_widget_y = rect.y * height_factor;
    const int width = rect.w * width_factor;
    const int height = rect.h * height_factor;
    return {
        .x = dstrect.x + in_widget_x,
        .y = dstrect.y + in_widget_y,
        .w = width,
        .h = height
    };
}

Widget* Widget::pressed_widget{nullptr};

Widget::Widget(const sptr_t<::SDL_Renderer>& renderer)
    : _renderer{renderer}
{
}

Widget::~Widget()
{
}

void Widget::load(const fs::Path& fname)
{
    if (_texture) {
        _texture.reset();
    }

    if (_renderer) {
        _texture = {
            ::IMG_LoadTexture(_renderer.get(), fname.c_str()),
            ::SDL_DestroyTexture
        };

        if (!_texture) {
            throw UIError{"Can't load image: {}: {}", fname.c_str(), sdl_error()};
        }
    }
}

void Widget::load(const std::span<const uint8_t> data)
{
    if (_texture) {
        _texture.reset();
    }

    if (_renderer) {
        ::SDL_RWops* ops = ::SDL_RWFromConstMem(data.data(), data.size());
        if (ops == nullptr) {
            throw UIError{"Can't load image from memory: {}", sdl_error()};
        }

        _texture = {
            ::IMG_LoadTexture_RW(_renderer.get(), ops, 1),
            ::SDL_DestroyTexture
        };

        if (!_texture) {
            throw UIError{"Can't create texture from memory: {}", sdl_error()};
        }
    }
}

void Widget::action(const std::function<void()>& act)
{
    _action = act;
}

void Widget::action()
{
    if (_action) {
        _action();
        start_action_period();
    }
}

bool Widget::enabled() const
{
    return true;
}

bool Widget::is_idle()
{
    return (!is_action_period());
}

void Widget::event(const ::SDL_Event& event, const ::SDL_Rect& rect)
{
    switch (event.type) {
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
    case SDL_MOUSEWHEEL:
    case SDL_MOUSEMOTION:
        break;

    case SDL_MOUSEBUTTONDOWN:
        pressed_widget = (event.button.button == ACTION_BUTTON ? this : nullptr);
        break;

    case SDL_MOUSEBUTTONUP:
        if (pressed_widget == this && in_rect(event.button.x, event.button.y, rect)) {
            action();
        }
        pressed_widget = nullptr;
        break;

    default:;
    }
}

void Widget::render(const ::SDL_Rect& srcrect, const ::SDL_Rect& dstrect)
{
    update_action_period();
    if (::SDL_RenderCopy(_renderer.get(), _texture.get(), &srcrect, &dstrect) < 0) {
        throw UIError{"Can't SDL_RenderCopy: {}", sdl_error()};
    }
}

void Widget::render(const ::SDL_Rect& srcrect, const ::SDL_Rect& dstrect, Rgba color)
{
    auto prev_color = color_modulator();
    color_modulator(color);
    render(srcrect, dstrect);
    color_modulator(prev_color);
}

void Widget::render(const ::SDL_Rect& srcrect, const ::SDL_Rect& dstrect, const ::SDL_Point& centre, float angle,
    ::SDL_RendererFlip flip)
{
    update_action_period();
    if (::SDL_RenderCopyEx(_renderer.get(), _texture.get(), &srcrect, &dstrect, angle, &centre, flip) < 0) {
        throw UIError{"Can't SDL_RenderCopyEx: {}", sdl_error()};
    }
}

void Widget::render(const ::SDL_Rect& srcrect, const ::SDL_Rect& dstrect, const ::SDL_Point& centre, float angle,
    ::SDL_RendererFlip flip, Rgba color)
{
    auto prev_color = color_modulator();
    color_modulator(color);
    render(srcrect, dstrect, centre, angle, flip);
    color_modulator(prev_color);
}

void Widget::render_progress(float progress, const ::SDL_Rect& dstrect)
{
    static const int border_px = 3;

    static const ::SDL_Rect bar_rect_ext{
        .x = INFO_X,
        .y = INFO_Y,
        .w = INFO_WIDTH,
        .h = INFO_HEIGHT
    };

    static const ::SDL_Rect bar_rect_int{
        .x = bar_rect_ext.x + border_px,
        .y = bar_rect_ext.y + border_px,
        .w = bar_rect_ext.w - 2 * border_px,
        .h = bar_rect_ext.h - 2 * border_px
    };

    const ::SDL_Rect progress_rect{
        .x = bar_rect_int.x,
        .y = bar_rect_int.y,
        .w = static_cast<int>(bar_rect_int.w * std::max(0.0f, std::min(progress, 1.0f))),
        .h = bar_rect_int.h
    };

    const auto rect_ext = to_rect(bar_rect_ext, dstrect);
    const auto rect_int = to_rect(bar_rect_int, dstrect);
    const auto rect_prg = to_rect(progress_rect, dstrect);
    render_rect(rect_ext, PROGRESS_BORDER_COLOR);
    render_rect(rect_int, PROGRESS_BG_COLOR);
    render_rect(rect_prg, PROGRESS_COLOR);
}

void Widget::render_rect(const ::SDL_Rect& rect, Rgba color)
{
    draw_color(color);
    ::SDL_RenderFillRect(_renderer.get(), &rect);
}

Rgba Widget::draw_color() const
{
    Rgba color{};

    if (::SDL_GetRenderDrawColor(_renderer.get(), &color.r, &color.g, &color.b, &color.a) < 0) {
        throw UIError{"Can't SDL_GetRenderDrawColor: {}", sdl_error()};
    }

    return color;
}

void Widget::draw_color(Rgba color)
{
    if (::SDL_SetRenderDrawColor(_renderer.get(), color.r, color.g, color.b, color.a) < 0) {
        throw UIError{"Can't ::SDL_SetRenderDrawColor: {}", sdl_error()};
    }
}

Rgba Widget::color_modulator() const
{
    Rgba color{};

    if (::SDL_GetTextureColorMod(_texture.get(), &color.r, &color.g, &color.b) < 0 ||
        ::SDL_GetTextureAlphaMod(_texture.get(), &color.a) < 0) {
        throw UIError{"Can't SDL_GetTexture{{Color,Alpha}}Mod: {}", sdl_error()};
    }

    return color;
}

void Widget::color_modulator(Rgba color)
{
    if (::SDL_SetTextureColorMod(_texture.get(), color.r, color.g, color.b) < 0 ||
        ::SDL_SetTextureAlphaMod(_texture.get(), color.a) < 0) {
        throw UIError{"Can't SDL_SetTexture{{Color,Alpha}}Mod: {}", sdl_error()};
    }
}

void Widget::start_action_period()
{
    _is_action_period = true;
    _action_time = utils::now();
}

void Widget::update_action_period()
{
    if (_is_action_period && (utils::now() > (_action_time + ACTION_TIME))) {
        _is_action_period = false;
    }
}

bool Widget::is_action_period() const
{
    return _is_action_period;
}

Label::Label(const sptr_t<::SDL_Renderer>& renderer)
    : _renderer{renderer}
{
}

void Label::reset(const std::string& label, Rgba color)
{
    if (label != _label) {
        auto [tex, w, h] = print(label, color);
        _texture = std::move(tex);
        if (_texture) {
            _label = label;
            _ratio = static_cast<float>(w) / h;
        } else {
            _label = "";
        }
    }
}

void Label::render(const ::SDL_Rect& dstrect)
{
    if (_texture) {
        auto rect = to_rect(LABEL_RECT, dstrect);
        const auto new_font_size = rect.w / LABEL_MAX_COLS;
        if (new_font_size > LABEL_MIN_FONT_SIZE) {
            /*
             * Show label only if the rendered font is readable.
             */
            rect.x += static_cast<int>(new_font_size * (LABEL_MAX_COLS - _label.size()) / 2);
            rect.w = static_cast<int>(_label.size() * rect.w / 8);
            rect.h = static_cast<int>(rect.w / _ratio);
            ::SDL_RenderCopy(_renderer.get(), _texture.get(), nullptr, &rect);
        }
    }
}

Label::Texture Label::print(const std::string& msg, Rgba color, unsigned width)
{
#define NULL_TEXTURE    Texture{uptrd_t<::SDL_Texture>{nullptr, nullptr}, 0, 0}

    if (!font) {
        font.init();
        if (!font) {
            return NULL_TEXTURE;
        }
    }

    if (msg.empty()) {
        return NULL_TEXTURE;
    }

    const ::SDL_Color sdlcolor{color.r, color.g, color.b, color.a};

    /* Ensure the utf8 message ends with a null value */
    const auto zmsg = msg + "\U00000000";

    auto* surf = ::TTF_RenderUTF8_Blended_Wrapped(font.get(), zmsg.data(), sdlcolor, static_cast<uint32_t>(width));
    if (surf == nullptr) {
        log.error("Label: Can't TTF_RenderUTF8_Blended_Wrapped: {}\n", sdl_error());
        return NULL_TEXTURE;
    }

    const uptrd_t<::SDL_Surface> surface{surf, ::SDL_FreeSurface};

    auto* tex = ::SDL_CreateTextureFromSurface(_renderer.get(), surf);
    if (tex == nullptr) {
        log.error("Label: Can't SDL_CreateTextureFromSurface: {}\n", sdl_error());
        return NULL_TEXTURE;
    }

    uptrd_t<::SDL_Texture> texture{tex, ::SDL_DestroyTexture};

    int w{}, h{};
    if (::SDL_QueryTexture(tex, nullptr, nullptr, &w, &h) != 0) {
        log.error("Label: Can't SDL_QueryTexture: {}\n", sdl_error());
        return NULL_TEXTURE;
    }

    return {std::move(texture), w, h};
}

Label::Font::Font()
    : uptrd_t<::TTF_Font>{nullptr, nullptr}
{
}

Label::Font& Label::Font::init()
{
    #include "icons/DejaVuSansMNerdFontMono_Regular_stripped.hpp"
#define FONT_NAME       DejaVuSansMNerdFontMono_Regular_stripped_ttf

    _ttf = utils::base64_decode(std::span<const uint8_t>{FONT_NAME, std::size(FONT_NAME) - 1});
    auto* ops = ::SDL_RWFromConstMem(_ttf.data(), _ttf.size());
    uptrd_t<::TTF_Font> font{
        ::TTF_OpenFontRW(ops, 1, WIDTH),
        ::TTF_CloseFont
    };

    if (!font) {
        log.error("Can't load font: {}\n", sdl_error());
    } else {
        ::TTF_SetFontStyle(font.get(), TTF_STYLE_NORMAL);
        static_cast<uptrd_t<::TTF_Font>&>(*this) = std::move(font);
    }

    return *this;
}

Label::Font Label::font{};

}
}
}
}
