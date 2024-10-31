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
#include "dearimgui.hpp"

#include <cstdio>
#include <vector>

#include "3rdparty/imgui/libimgui/imgui_internal.h"

#include "fs.hpp"
#include "utils.hpp"

using namespace std::string_literals;

namespace caio {
namespace ui {
namespace sdl2 {
namespace dearimgui {

#include "icons/DejaVuSansMNerdFontMono_Regular_stripped.hpp"
#define FONT_NAME       DejaVuSansMNerdFontMono_Regular_stripped_ttf

static buffer_t                         ttf{};
static std::vector<::ImFont*>           loaded_fonts{};
static std::vector<::ImFont*>::iterator current_loaded_font{};

void Gui::init(const std::string& inifile, ::SDL_Window* sdlwin, ::SDL_Renderer* sdlrend, const FontParams& fontp)
{
    if (ttf.size() == 0) {
        ttf = utils::base64_decode({FONT_NAME, std::size(FONT_NAME)});
    }

    _inifile = inifile;
    _sdlwin  = sdlwin;
    _sdlrend = sdlrend;

    /*
     * Initialise Dear ImGui.
     */
    ::ImGui::CreateContext();
    ::ImGui_ImplSDL2_InitForSDLRenderer(_sdlwin, _sdlrend);
    ::ImGui_ImplSDLRenderer2_Init(_sdlrend);

    auto& io = ::ImGui::GetIO();
    io.ConfigFlags |= ::ImGuiConfigFlags_NavEnableKeyboard | ::ImGuiConfigFlags_NavEnableGamepad;
    io.FontGlobalScale = fontp.scale;
    io.IniFilename = (_inifile.empty() ? nullptr : _inifile.c_str());
    io.LogFilename = nullptr;

    style(Style::Light);

    /*
     * Load fonts and set the first font size as default.
     */
    const auto font_sizes = (fontp.sizes ? fontp.sizes() : std::span<float>{});
    const auto font_ranges = (fontp.ranges ? fontp.ranges().data() : nullptr);

    ::ImFontConfig font_config{};
    font_config.FontDataOwnedByAtlas = false;

    std::for_each(font_sizes.begin(), font_sizes.end(), [&io, &font_ranges, &font_config](float fsize) {
        auto* font = io.Fonts->AddFontFromMemoryTTF(ttf.data(), ttf.size(), fsize, &font_config, font_ranges);
        loaded_fonts.push_back(font);
    });

    current_loaded_font = loaded_fonts.begin();

    /*
     * Custom inifile handler for section INI_SNAME.
     */
    const auto clear_all = [](::ImGuiContext*, ::ImGuiSettingsHandler*) {
        current_loaded_font = loaded_fonts.begin();
    };

    const auto write_all = [](::ImGuiContext*, ::ImGuiSettingsHandler* handler, ::ImGuiTextBuffer* buf) {
        const auto* self = reinterpret_cast<Gui*>(handler->UserData);
        const int font_index = current_loaded_font - loaded_fonts.begin();
        int x{}, y{}, w{}, h{};
        ::SDL_GetWindowPosition(self->_sdlwin, &x, &y);
        ::SDL_GetWindowSize(self->_sdlwin, &w, &h);
        buf->appendf(
            "[%s][gui]\n"
            "win=%d,%d,%d,%d\n"
            "font-index=%d\n"
            "style=%d\n\n",
            INI_SNAME,
            x, y, w, h,
            font_index,
            static_cast<int>(self->_style));
    };

    const auto read_open = [](::ImGuiContext*, ::ImGuiSettingsHandler*, const char* name) -> void* {
        /*
         * We must return a non-zero ptr for our read_line to be called.
         * The returned value is the ptr paremeter for read_line.
         */
        return reinterpret_cast<void*>(name == "gui"s);
    };

    const auto read_line = [](::ImGuiContext*, ::ImGuiSettingsHandler* handler, void* ptr, const char* line) {
        auto* self = reinterpret_cast<Gui*>(handler->UserData);
        int x{}, y{}, w{}, h{};
        if (::sscanf(line, "win=%d,%d,%d,%d\n", &x, &y, &w, &h) == 4) {
            ::SDL_SetWindowPosition(self->_sdlwin, x, y);
            ::SDL_SetWindowSize(self->_sdlwin, w, h);

            /* Force a SDL window event so imgui gets updated */
            ::SDL_Event event{
                .window = {
                    .type       = ::SDL_WINDOWEVENT,
                    .timestamp  = ::SDL_GetTicks(),
                    .windowID   = ::SDL_GetWindowID(self->_sdlwin),
                    .event      = ::SDL_WINDOWEVENT_RESIZED,
                    .data1      = static_cast<::Sint32>(w),
                    .data2      = static_cast<::Sint32>(h)
                }
            };
            ::SDL_PushEvent(&event);

        } else if (::sscanf(line, "font-index=%d\n", &x) == 1) {
            current_loaded_font = loaded_fonts.begin() + std::min(x, static_cast<int>(loaded_fonts.size()));
        } else if (::sscanf(line, "style=%d\n", &x) == 1) {
            self->style(static_cast<Style>(x));
        }
    };

    ::ImGuiSettingsHandler ini_handler{};
    ini_handler.TypeName   = INI_SNAME;
    ini_handler.TypeHash   = ::ImHashStr(INI_SNAME);
    ini_handler.ClearAllFn = clear_all;
    ini_handler.ReadOpenFn = read_open;
    ini_handler.ReadLineFn = read_line;
    ini_handler.WriteAllFn = write_all;
    ini_handler.UserData   = this;
    ::ImGui::AddSettingsHandler(&ini_handler);
}

void Gui::release()
{
    ::ImGui_ImplSDLRenderer2_Shutdown();
    ::ImGui_ImplSDL2_Shutdown();
    ::ImGui::DestroyContext();
}

void Gui::process_event(const ::SDL_Event& event)
{
    ::ImGui_ImplSDL2_ProcessEvent(&event);
}

void Gui::pre_render()
{
    ::ImGui_ImplSDLRenderer2_NewFrame();
    ::ImGui_ImplSDL2_NewFrame();
    ::ImGui::NewFrame();
    ::ImGui::PushFont(current_font());
}

void Gui::post_render()
{
    ::ImGui::PopFont();
    ::ImGui::Render();
    const uint8_t color = 255 * (_style == Style::Dark);
    ::SDL_SetRenderDrawColor(_sdlrend, color, color, color, 255);
    ::SDL_RenderClear(_sdlrend);
    ::ImGui_ImplSDLRenderer2_RenderDrawData(::ImGui::GetDrawData(), _sdlrend);
    ::SDL_RenderPresent(_sdlrend);
}

::ImFont* Gui::current_font()
{
    return (*current_loaded_font);
}

void Gui::process_font_incdec(bool inc)
{
    if (inc) {
        next_font_size();
    } else {
        prev_font_size();
    }
}

void Gui::next_font_size()
{
    if (current_loaded_font + 1 != loaded_fonts.end()) {
        ++current_loaded_font;
    }
}

void Gui::prev_font_size()
{
    if (current_loaded_font != loaded_fonts.begin()) {
        --current_loaded_font;
    }
}

float Gui::font_width()
{
    const ::ImGuiContext& g = *::ImGui::GetCurrentContext();
    const float fwidth = g.FontSize + g.Style.ItemSpacing.x;
    return fwidth;
}

void Gui::cursor_to_column(unsigned col)
{
    const float pos = font_width() * col;
    ::ImGui::SameLine(1.0f, pos);
}

void Gui::cursor_to_valuecol()
{
    cursor_to_column(VALUE_COLUMN);
}

void Gui::begin_window(const std::string& id, const Size& container_size)
{
    const auto [width, height] = container_size;
    ::ImGui::SetNextWindowSize(::ImVec2{width, height}, 0);
    ::ImGui::SetNextWindowPos(::ImVec2{}, ::ImGuiCond_Always, ::ImVec2{});
    ::ImGui::Begin(id.c_str(), nullptr,
        ::ImGuiWindowFlags_NoTitleBar |
        ::ImGuiWindowFlags_NoCollapse |
        ::ImGuiWindowFlags_NoDecoration |
        ::ImGuiWindowFlags_NoSavedSettings |
        ::ImGuiWindowFlags_NoMove |
        ::ImGuiWindowFlags_NoResize |
        ::ImGuiWindowFlags_AlwaysAutoResize);
}

void Gui::end_window()
{
    ::ImGui::End();
}

void Gui::begin_section(const std::string& id, const Size& size, int flags)
{
    ::ImGui::BeginChild(id.c_str(), size, flags);
}

void Gui::end_section()
{
    ::ImGui::EndChild();
}

Gui::Size Gui::section_size()
{
    return {::ImGui::GetWindowWidth(), ::ImGui::GetWindowHeight()};
}

void Gui::begin_subsection(const std::string& msg)
{
    ::ImGui::Text("%s", msg.c_str());
    ::ImGui::Indent();
}

void Gui::end_subsection()
{
    ::ImGui::Unindent();
}

void Gui::begin_disabled(bool disabled)
{
    ::ImGui::BeginDisabled(disabled);
}

void Gui::end_disabled()
{
    ::ImGui::EndDisabled();
}

void Gui::define_popup(const std::string& id, const ActionCb& action)
{
    if (::ImGui::BeginPopupModal(id.c_str(), nullptr,
        ::ImGuiWindowFlags_AlwaysAutoResize | ::ImGuiWindowFlags_NoSavedSettings)) {
        if (action) {
            action();
        }
        ::ImGui::EndPopup();
        ::ImGui::SetItemDefaultFocus();
    }
}

void Gui::open_popup(const std::string& id)
{
    ::ImGui::OpenPopup(id.c_str());
}

void Gui::close_popup()
{
    ::ImGui::CloseCurrentPopup();
}

void Gui::to_column(float col)
{
    if (col < 0.0f) {
        float cols = ::ImGui::GetWindowWidth() / font_width();
        col = cols + col;
        if (col < 0.0f) {
             col = cols;
        }
    }
    ::ImGui::SameLine(col * font_width(), 0.0f);
}

void Gui::sameline()
{
    ::ImGui::SameLine();
}

void Gui::newline()
{
    print("");
}

void Gui::separator(const std::string& msg)
{
    if (msg.empty()) {
        ::ImGui::Separator();
    } else {
        ::ImGui::SeparatorText(msg.c_str());
    }
}

void Gui::print(const std::string& msg)
{
    ::ImGui::Text("%s", msg.c_str());
}

void Gui::select_table(const std::string& label, bool selected, const ActionCb& action)
{
    if (::ImGui::Selectable(label.c_str(), selected, ::ImGuiSelectableFlags_DontClosePopups) && action) {
        action();
    }
}

bool Gui::button(const std::string& label, const ActionCb& action, float width)
{
    if (::ImGui::Button(label.c_str(), ::ImVec2{width, 0})) {
        if (action) {
            action();
        }
        return true;
    }
    return false;
}

bool Gui::button_ok(const ActionCb& action)
{
    return button("OK", action, BUTTON_WIDTH_OK);
}

bool Gui::button_cancel(const ActionCb& action)
{
    return button("Cancel", action, BUTTON_WIDTH_CANCEL);
}

std::optional<bool> Gui::buttons_ok_cancel(const ActionCb& ok, const ActionCb& cancel)
{
    if (button_ok(ok)) {
        return true;
    }

    sameline();

    if (button_cancel(cancel)) {
        return false;
    }

    return {};
}

void Gui::input_text(const std::string& msg, const std::string& id, std::string& dst)
{
    const float fwidth = font_width();
    print(msg);
    cursor_to_valuecol();
    char dir[80];
    ::snprintf(dir, sizeof(dir), "%s", dst.c_str());
    ::ImGui::SetNextItemWidth(20 * fwidth);
    if (::ImGui::InputText(id.c_str(), dir, sizeof(dir), 0)) {
        dst = dir;
    }
}

void Gui::input_int(const std::string& msg, const std::string& id, int& dst, size_t width, bool(*cond)(int))
{
    const float fwidth = font_width();
    print(msg);
    cursor_to_valuecol();
    ::ImGui::SetNextItemWidth(width * fwidth);
    if (int d = dst; ::ImGui::InputInt(id.c_str(), &d, 1, 1, ::ImGuiInputTextFlags_CharsDecimal) && cond(d)) {
        dst = d;
    }
}

int Gui::input(const std::string& id, std::string& dst, int flags)
{
    char buf[256];
    ::snprintf(buf, sizeof(buf), "%s", dst.c_str());
    if (::ImGui::InputText(id.c_str(), buf, sizeof(buf), flags)) {
        dst = buf;
        return true;
    }
    return false;
}

void Gui::checkbox(const std::string& msg, const std::string& id, bool& dst)
{
    print(msg);
    cursor_to_valuecol();
    ::ImGui::Checkbox(id.c_str(), &dst);
}

void Gui::combo_select(const std::string& label, const std::string& id, const char** list, size_t size,
    std::string& dst)
{
    const float fwidth = font_width();

    print(label);
    cursor_to_valuecol();
    ::ImGui::SetNextItemWidth(10 * fwidth);

    const auto it = std::find(list, list + size, dst);
    int index = (it == list + size ? -1 : it - list);

    ::ImGui::Combo(id.c_str(), &index, list, size, size);
    dst = list[index];
}

void Gui::combo_key(const std::string& msg, const std::string& id, keyboard::Key& key)
{
    constexpr static const int SHOW_ITEMS = 10;
    static const auto knames = keyboard::key_names();
    static const auto getter = [](void*, int idx) -> const char* { return knames[idx].c_str(); };
    static const auto kname_index = [](keyboard::Key key) -> int {
        const auto& kname = keyboard::to_string(key);
        if (const auto it = std::find(knames.begin(), knames.end(), kname); it != knames.end()) {
            return (it - knames.begin());
        }
        return knames.size() - 1;
    };
    const float fwidth = font_width();

    int dst = kname_index(key);

    print(msg);
    cursor_to_valuecol();
    ::ImGui::SetNextItemWidth(10 * fwidth);
    ::ImGui::Combo(id.c_str(), &dst, getter, nullptr, knames.size(), SHOW_ITEMS);

    key = keyboard::to_key(knames[dst]);
}

void Gui::combo_scanlines(std::string& dst)
{
    static const std::string label = "Scanlines effect";
    static const std::string id = "##scanlines";
    static const char* scanline_names[]{
        "None",
        "Horizontal",
        "Advanced horizontal",
        "Vertical",
        "Advanced vertical"
    };
    static const std::string scanline_codes[]{ "n", "h", "H", "v", "V" };
    static const auto code_to_index = [](std::string_view slcode) -> int {
        if (const auto it = std::find(std::begin(scanline_codes), std::end(scanline_codes), slcode);
            it != std::end(scanline_codes)) {
            return (it - std::begin(scanline_codes));
        }
        return 0;
    };
    const float fwidth = font_width();

    int sl_index = code_to_index(dst);
    int prev_sl_index = sl_index;

    print(label);
    cursor_to_valuecol();
    ::ImGui::SetNextItemWidth(10 * fwidth);
    ::ImGui::Combo(id.c_str(), &sl_index, scanline_names, std::size(scanline_names), std::size(scanline_names));

    if (sl_index != prev_sl_index) {
        dst = scanline_codes[sl_index];
    }
}

void Gui::combo_statusbar(std::string& dst)
{
    static const std::string label = "Status bar position";
    static const std::string id = "##statusbar";
    static const char* position_names[]{
        "None",
        "Center",
        "North",
        "South",
        "Easth",
        "West",
        "North-East",
        "North-West",
        "South-East",
        "South-West"
    };
    static const char* position_codes[]{
        "none",
        "center",
        "north",
        "south",
        "easth",
        "west",
        "north-east",
        "north-west",
        "south-east",
        "south-west"
    };
    static const auto code_to_index = [](std::string_view code) -> int {
        if (const auto it = std::find(std::begin(position_codes), std::end(position_codes), code);
            it != std::end(position_codes)) {
            return (it - std::begin(position_codes));
        }
        return 0;
    };
    const float fwidth = font_width();
    int index = code_to_index(utils::tolow(dst));
    int prev_index = index;

    print(label);
    cursor_to_valuecol();
    ::ImGui::SetNextItemWidth(10 * fwidth);
    ::ImGui::Combo(id.c_str(), &index, position_names, std::size(position_names), std::size(position_names));

    if (index != prev_index) {
        dst = position_codes[index];
    }
}

void Gui::combo_path(const std::string& msg, const std::string& id, std::string& dst, fs::IDir& idir,
    const SetterCb& setter, unsigned width)
{
    const float fwidth = font_width();

    print(msg);
    cursor_to_valuecol();
    ::ImGui::SetNextItemWidth(width * fwidth);

    if (idir.empty()) {
        idir.reset(dst);
    }

    if (::ImGui::BeginCombo(id.c_str(), dst.c_str())) {
        std::error_code ec{};
#ifdef GUI_COMBO_PATH_RELATIVE
        if (!dst.empty()) {
            dst = std::filesystem::proximate(dst, ec);
        }
#endif
        idir.refresh();
        for (size_t i = 0; i < idir.size(); ++i) {
            const auto& entry = idir[i];
            const auto dstc = std::filesystem::weakly_canonical(dst, ec);
            const bool is_selected = ((i == 0) ? dst.empty() : std::filesystem::equivalent(dstc, entry, ec));
            if (::ImGui::Selectable(entry.c_str(), is_selected, ::ImGuiSelectableFlags_DontClosePopups)) {
                setter(idir, entry, dst);
            }
        }
        ::ImGui::EndCombo();
    }
}

void Gui::combo_directory(const std::string& msg, const std::string& id, std::string& dst, fs::IDir& idir)
{
    static const auto setter = [](fs::IDir& idir, const fs::Path& entry, std::string& dst) {
        if (entry == fs::IDir::ENTRY_BACK) {
            dst = idir.path().parent_path();
        } else if (entry == ENTRY_EMPTY) {
            dst = {};
        } else {
            dst = entry;
        }
        idir.reset(dst);
    };

    combo_path(msg, id, dst, idir, setter, COMBO_FILE_WIDTH);
}

void Gui::combo_file(const std::string& msg, const std::string& id, std::string& dst, fs::IDir& idir, unsigned width)
{
    static const auto setter = [](fs::IDir& idir, const fs::Path& entry, std::string& dst) {
        if (entry == fs::IDir::ENTRY_BACK) {
            idir.reset(idir.path().parent_path());
        } else if (entry == ENTRY_EMPTY) {
            dst = {};
        } else if (std::filesystem::status(entry).type() == std::filesystem::file_type::directory) {
            idir.reset(entry);
        } else {
            dst = entry;
            idir.reset(entry.parent_path());
        }
    };

    combo_path(msg, id, dst, idir, setter, width);
}

void Gui::combo_file(const std::string& msg, const std::string& id, std::string& dst, fs::IDir& idir,
    const std::string& mprefix, const std::string& ext, unsigned width)
{
    if (!idir.filter()) {
        const auto filter = [mprefix, ext](const fs::Path& entry) -> fs::Path {
            const auto eext = entry.extension();
            auto fname = entry.stem().string();
            if (eext == ext && fname.starts_with(mprefix)) {
                return fname.erase(0, mprefix.size());
            }
            return {};
        };
        idir.filter(filter);
        dst = filter(dst);
    }

    combo_file(msg, id, dst, idir, width);
}

void Gui::combo_palette(std::string& dst, fs::IDir& idir, const std::string& mprefix)
{
    combo_file("Colour palette", "##palette", dst, idir, mprefix, ".plt", COMBO_WIDTH);
}

void Gui::combo_keymaps(std::string& dst, fs::IDir& idir, const std::string& mprefix)
{
    combo_file("Keyboard mappings", "##keymaps", dst, idir, mprefix, ".kbd", COMBO_WIDTH);
}

void Gui::keyboard_focus()
{
    ::ImGui::SetKeyboardFocusHere();
}

void Gui::style(Style style)
{
    auto* colors = ::ImGui::GetStyle().Colors;

    switch (style) {
    case Style::Light:
        ::ImGui::StyleColorsLight();
        break;
    case Style::Dark:
        ::ImGui::StyleColorsDark();
        colors[::ImGuiCol_WindowBg] = {0.15f, 0.15f, 0.15f, 1.00f};
        break;
    }

    _style = style;

#if 0
    colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border]                 = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator]              = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_Tab]                    = ImLerp(colors[ImGuiCol_Header],       colors[ImGuiCol_TitleBgActive], 0.80f);
    colors[ImGuiCol_TabSelected]            = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabSelectedOverline]    = colors[ImGuiCol_HeaderActive];
    colors[ImGuiCol_TabDimmed]              = ImLerp(colors[ImGuiCol_Tab],          colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabDimmedSelected]      = ImLerp(colors[ImGuiCol_TabSelected],  colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextLink]               = colors[ImGuiCol_HeaderActive];
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
#endif
}

}
}
}
}
