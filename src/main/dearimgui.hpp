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

#include "endian.hpp"
#include "fs.hpp"
#include "keyboard.hpp"
#include "types.hpp"
#include "utils.hpp"

#define IMGUI_USE_WCHAR32
#include "3rdparty/imgui/libimgui/imgui.h"
#include "3rdparty/imgui/libimgui/imgui_impl_sdl2.h"
#include "3rdparty/imgui/libimgui/imgui_impl_sdlrenderer2.h"

#include <optional>

namespace caio {
namespace ui {
namespace sdl2 {
namespace dearimgui {

class Gui {
public:
    constexpr static const char* INI_SNAME              = "caio";
    constexpr static const float FONT_SCALE             = 1.0f;
    constexpr static const float FONT_SIZE              = 10.0f;
    constexpr static const float BUTTON_WIDTH_OK        = 100.0f;
    constexpr static const float BUTTON_WIDTH_CANCEL    = 100.0f;
    constexpr static const unsigned VALUE_COLUMN        = 11;
    constexpr static const char* ENTRY_EMPTY            = "## ";
    constexpr static const unsigned COMBO_WIDTH         = 10;
    constexpr static const unsigned COMBO_FILE_WIDTH    = 20;

    enum class Style : int {
        Light,
        Dark
    };

    enum SectionFlags : int {
        Border              = ::ImGuiChildFlags_Border,
        ResizeX             = ::ImGuiChildFlags_ResizeX
    };

    enum InputFlags : int {
        AutoSelectAll       = ::ImGuiInputTextFlags_AutoSelectAll,
        EscapeClearsAll     = ::ImGuiInputTextFlags_EscapeClearsAll,
        EnterReturnsTrue    = ::ImGuiInputTextFlags_EnterReturnsTrue
    };

    struct FontParams {
        float scale;                                            /* Font scale                   */
        std::function<std::span<const float>()> sizes{};        /* Font sizes to load           */
        std::function<std::span<const uint32_t>()> ranges{};    /* Font glyph ranges to load    */
    };

    using Size = ::ImVec2;
    using ActionCb = std::function<void()>;
    using FilterCb = fs::IDir::FilterCb;

    void init(const std::string& inifile, ::SDL_Window* sdlwin, ::SDL_Renderer* sdlrend, const FontParams& fontp);

    void release();

    void process_event(const ::SDL_Event& event);

    void pre_render();
    void post_render();

    static ::ImFont* current_font();

    static void process_font_incdec(bool inc);

    static void cursor_to_column(unsigned col);
    static void cursor_to_valuecol();

    static void begin_window(const std::string& id, const Size& container_size);
    static void end_window();

    static void begin_section(const std::string& id, const Size& size = {}, int flags = 0);
    static void end_section();
    static Size section_size();

    static void begin_subsection(const std::string& msg);
    static void end_subsection();

    static void begin_disabled(bool disabled = true);
    static void end_disabled();

    static void define_popup(const std::string& id, const ActionCb& action);
    static void open_popup(const std::string& id);
    static void close_popup();

    static void to_column(float col);
    static void sameline();
    static void newline();
    static void separator(const std::string& msg = {});

    static void print(const std::string& msg);

    template<typename... Args>
    void print(std::format_string<Args...> fmt, Args&&... args)
    {
        print(std::vformat(fmt.get(), std::make_format_args(args...)));
    }

    static void select_table(const std::string& label, bool selected, const ActionCb& action);

    static bool button(const std::string& label, const ActionCb& action = {}, float width = 0.0f);
    static bool button_ok(const ActionCb& action = {});
    static bool button_cancel(const ActionCb& action = {});
    static std::optional<bool> buttons_ok_cancel(const ActionCb& ok = {}, const ActionCb& cancel = {});

    static int input(const std::string& id, std::string& dst, int flags = 0);
    static void input_text(const std::string& msg, const std::string& id, std::string& dst);
    static void input_int(const std::string& msg, const std::string& id, int& dst, size_t width, bool(*cond)(int));

    static void checkbox(const std::string& msg, const std::string& id, bool& dst);

    static void combo_select(const std::string& label, const std::string& id, const char** list, size_t size,
        std::string& dst);

    static void combo_aspect(std::string& dst);
    static void combo_scanlines(std::string& dst);
    static void combo_statusbar(std::string& dst);
    static void combo_key(const std::string& msg, const std::string& id, keyboard::Key& key);

    static void combo_directory(const std::string& msg, const std::string& id, std::string& dst, fs::IDir& dir);

    static void combo_file(const std::string& msg, const std::string& id, std::string& dst, fs::IDir& dir,
        unsigned width = COMBO_FILE_WIDTH);

    static void combo_file(const std::string& msg, const std::string& id, std::string& dst, fs::IDir& dir,
        const std::string& mprefix, const std::string& ext, unsigned width = COMBO_FILE_WIDTH);

    static void combo_palette(std::string& dst, fs::IDir& idir, const std::string& mprefix);
    static void combo_keymaps(std::string& dst, fs::IDir& idir, const std::string& mprefix);

    static void keyboard_focus();

    void style(Style style);

    Style style()
    {
        return _style;
    }

private:
    using SetterCb = std::function<void(fs::IDir&, const fs::Path&, std::string&)>;

    static void combo_path(const std::string& msg, const std::string& id, std::string& dst, fs::IDir& dir,
        const SetterCb& setter, unsigned width = COMBO_FILE_WIDTH);

    static void  next_font_size();
    static void  prev_font_size();
    static float font_width();

    std::string     _inifile{""};
    ::SDL_Window*   _sdlwin{};
    ::SDL_Renderer* _sdlrend{};
    Style           _style{};
};

}
}
}
}
